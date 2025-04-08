// Copyright(C) 2021 by Steven Adler
//
// This file is part of NOAA Weather Plugin for OpenCPN.
//
// NOAA Weather Plugin for OpenCPN is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// NOAA Weather Plugin for OpenCPN is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with the NOAA Weather Plugin for OpenCPN. If not, see <https://www.gnu.org/licenses/>.
//
// This plugin has been developed independent of NOAA and 
// is not authorised for use by, nor affilited with NOAA

//
// Project: NOAA Weather Plugin
// Description: Retrieve NOAA Weather Forecasts and Alerts
// Owner: twocanplugin@hotmail.com
// Date: 6/12/2021
// Version History: 
// 1.0 Initial Release
// 1.1 02/04/2025 - Add National Data Buoy Center (NDBC) reports
//

// Reference Information
// https://www.ndbc.noaa.gov/faq/rt_data_access.shtml
// https://www.ndbc.noaa.gov/docs/ndbc_web_data_guide.pdf

#include "noaa_weather_plugin.h"

// The class factories, used to create and destroy instances of the PlugIn
extern "C" DECL_EXP opencpn_plugin* create_pi(void *ppimgr) {

	return new NOAA_Plugin(ppimgr);
}

extern "C" DECL_EXP void destroy_pi(opencpn_plugin* p) {

	delete p;
}

NOAA_Plugin::NOAA_Plugin(void *ppimgr) : opencpn_plugin_118(ppimgr) {
	
	// Load the plugin bitmaps/icons 
	wxString pluginFolder = GetPluginDataDir(PLUGIN_PACKAGE_NAME) + wxFileName::GetPathSeparator() + "data" + wxFileName::GetPathSeparator();
	pluginBitmap = GetBitmapFromSVGFile(pluginFolder + "plugin_logo.svg", 32, 32);
	
	// BUG BUG This should be scaled dynamically based on the chart scale
	buoyBitmap = GetBitmapFromSVGFile(pluginFolder + "buoy_icon.svg", 32, 32);
}

NOAA_Plugin::~NOAA_Plugin(void) {

	// Nothing to do in the destructor
}

int NOAA_Plugin::Init(void) {

	// Maintain a reference to the OpenCPN window to use as the parent for the dialog
	parentWindow = GetOCPNCanvasWindow();

	// Maintain a reference to the OpenCPN configuration object 
	// BUG BUG No preferences dialog to set flag for either scheduled reports or realtime observations 
	configSettings = GetOCPNConfigObject();
	if (configSettings) {
		configSettings->SetPath(_T("/PlugIns/NOAA"));
		configSettings->Read(_T("Mode"), &useScheduled, true);
	}

	// Add our context menu items, Requires INSTALLS_CONTEXTMENU_ITEMS
	// BUG BUG Move wxID's to the header, so there is a central place to maintain the values
	wxMenuItem *menuItem = new wxMenuItem(NULL, wxID_SEPARATOR, wxEmptyString, wxEmptyString, wxITEM_SEPARATOR, NULL);
	AddCanvasContextMenuItem(menuItem, this);

	menuItem = new wxMenuItem(NULL, wxID_HIGHEST + 1, _T("NOAA Forecast"), wxEmptyString, wxITEM_NORMAL, NULL);
	noaaForecastMenu = AddCanvasContextMenuItem(menuItem, this);
	
	menuItem = new wxMenuItem(NULL, wxID_HIGHEST + 2, _T("NOAA Alerts"), wxEmptyString, wxITEM_NORMAL, NULL);
	noaaAlertMenu = AddCanvasContextMenuItem(menuItem, this);

	menuItem = new wxMenuItem(NULL, wxID_HIGHEST + 3, _T("NOAA Reports"), wxEmptyString, wxITEM_NORMAL, NULL);
	noaaBuoyMenu = AddCanvasContextMenuItem(menuItem, this);

	// Only enable the Reports menu item when the cursor is actually positioned on a buoy
	SetCanvasContextMenuItemGrey(noaaBuoyMenu, true);

	// Download either the NOAA NDBC Station List or Scheduled Reports
	// BUG BUG Should this be done everytime OpenCPN loads, at scheduled times or manually?
	if (OCPN_isOnline()) {

		if (useScheduled) {
			// Download scheduled reports for all reporting stations
			DownloadScheduledReports();
		}
		else {
			// The station list is used to retrieve realtime observations for an individual station
			DownloadStationList();
		}
	}

	// Notify OpenCPN what events we want to receive callbacks for
	return (WANTS_CONFIG | INSTALLS_CONTEXTMENU_ITEMS | WANTS_NMEA_EVENTS |
		WANTS_MOUSE_EVENTS | WANTS_CURSOR_LATLON | WANTS_OVERLAY_CALLBACK | 
		WANTS_OPENGL_OVERLAY_CALLBACK | WANTS_ONPAINT_VIEWPORT);
}

// OpenCPN is either closing down, or we have been disabled from the Preferences Dialog
bool NOAA_Plugin::DeInit(void) {

	return true;
}

// Indicate what version of the OpenCPN Plugin API we support
int NOAA_Plugin::GetAPIVersionMajor() {

	return OCPN_API_VERSION_MAJOR;
}

int NOAA_Plugin::GetAPIVersionMinor() {

	return OCPN_API_VERSION_MINOR;
}

// The plugin version numbers. 
int NOAA_Plugin::GetPlugInVersionMajor() {

	return PLUGIN_VERSION_MAJOR;
}

int NOAA_Plugin::GetPlugInVersionMinor() {

	return PLUGIN_VERSION_MINOR;
}

// Return descriptions for the Plugin
wxString NOAA_Plugin::GetCommonName() {

	return _T(PLUGIN_COMMON_NAME);
}

wxString NOAA_Plugin::GetShortDescription() {

	return _T(PLUGIN_SHORT_DESCRIPTION);
}

wxString NOAA_Plugin::GetLongDescription() {

	return _T(PLUGIN_LONG_DESCRIPTION);
}

// The plugin bitmap is generated from a SVG file
wxBitmap* NOAA_Plugin::GetPlugInBitmap() {

	return &pluginBitmap;
}

// Requires WANTS_NMEA_EVENTS
// The current position is used as the basis for retrieving weather forecasts and alerts
void NOAA_Plugin::SetPositionFix(PlugIn_Position_Fix &pfix) {

	currentLatitude = pfix.Lat; 
	currentLongitude = pfix.Lon; 
}

// Requires WANTS_CURSOR_LATLON 
// Iterate over the list of visible stations and if hovering over a buoy, enable the context menu item
void NOAA_Plugin::SetCursorLatLon(double lat, double lon) {

	if (IsUnderCursor(lat, lon, &id, &name)) {
		SetCanvasContextMenuItemGrey(noaaBuoyMenu, false);
	}
	else {
		SetCanvasContextMenuItemGrey(noaaBuoyMenu, true);
	}
}

// Requires WANTS_ONPAINT_VIEWPORT
// The view port (chart extent) is used to determine what buoys can be overlayed on the chart
void NOAA_Plugin::SetCurrentViewPort(PlugIn_ViewPort& vp) {

	viewPort = vp;
	visibleBuoys = FilterVisibleBuoys(vp);

	// BUG BUG Should scale the buoy icon depending on vp.chart_scale
	// By observation, scales ranges included: 
	// 101415
	// 812551
	// 5050832
}

// Requires WANTS_MOUSE_EVENTS
// Handles double click events on the weather buoy
bool NOAA_Plugin::MouseEventHook(wxMouseEvent& event) {

	// Only perform these actions if we have an Internet connection
	if (OCPN_isOnline()) {

		// BUG BUG Multi-canvas support??
		if (GetCanvasIndexUnderMouse() == 0) {
			double lat, lon;

			// Handle a double click event to retrieve the weather observations from the buoy
			if (event.LeftDClick()) {

				// Convert Pixel Co-ordinates to latitude and longitude
				GetCanvasLLPix(&viewPort, event.GetPosition(), &lat, &lon);

				// Iterate over the visible station list and see if any buoys were the double click target
				if (IsUnderCursor(lat, lon, &id, &name)) {

					// Display the weather observation
					if (useScheduled) {
						std::string sid = id.ToStdString();
						const auto p = std::find_if(visibleBuoys.begin(), visibleBuoys.end(),
							[sid](const BuoyData& a) { return a.id == sid; });

						if (p != visibleBuoys.end()) {
							wxMessageBox(wxString::Format("Wind Direction: %d\nWind Speed %0.2f\nPressure: %0.2f\nTemperature: %0.2f",
								p->windDirection, p->windSpeed, p->barometricPressure, p->airTemperature), p->id);
						}
					}
					else {
						DownloadRealtimeObservation(id, name);
					}
					return true;
				}
			}
		}
	}
	// I think returning false allows the event to be forwarded to OpenCPN (eg. chart object properties)
	return false;
}

// Handle context menu events
void NOAA_Plugin::OnContextMenuItemCallback(int menuId) {

	// Only perform these actions if we have an Internet connection
	if (OCPN_isOnline()) {

		// BUG BUG Should these menu items be greyed out if no Internet connection ?
		if (menuId == noaaForecastMenu) {

			// Obtain the correct station id & grids given our current position
			wxString forecastUrl = GetForecastUrl(currentLatitude, currentLongitude);

			if (forecastUrl.Length() > 0) {
				// Once we have the url, can now retrieve the actual forecast
				wxString jsonResponse = ExecuteQuery(forecastUrl);
				wxJSONReader reader;
				wxJSONValue root;

				// Check for any JSON parsing errors
				if (reader.Parse(jsonResponse, &root) > 0) {
					wxLogMessage("NOAA Weather Plugin, Json parser error(s) in the following response:\n%s", jsonResponse);
					for (auto it : reader.GetErrors()) {
						wxLogMessage("NOAA Weather Plugin, Json parser error: %s", it);
					}
				}
				else {
					// Display the forecast values in a simple data grid
					// BUG BUG Should this be a modal dialog?
					NOAA_Plugin_Dialog* dialog = new NOAA_Plugin_Dialog(parentWindow, jsonResponse);
					dialog->Show();
				}
			}
			else {
				wxMessageBox("Error retrieving forecast\nPlease check OpenCPN log",
					_T(PLUGIN_COMMON_NAME), wxICON_ERROR);
				wxLogMessage("NOAA Weather Plugin, Error retrieving forecast URL: %s", forecastUrl);
			}
		}

		// Marine Weather Alerts can be retrieved directly given the vessel's current position.
		// No need to retrieve the root object to determine the grid or station id.
		if (menuId == noaaAlertMenu) {
			// Example URL https://api.weather.gov/alerts/active?point=47.606210,-122.33207
			wxString url = wxString::Format("https://api.weather.gov/alerts/active?point=%07.4f,%08.4f", currentLatitude, currentLongitude);

			wxString jsonResponse = ExecuteQuery(url);
			wxJSONReader reader;
			wxJSONValue root;

			// Check for any JSON parsing errors
			if (reader.Parse(jsonResponse, &root) > 0) {
				wxLogMessage("NOAA Weather Plugin, Json parser error(s) in the following response:\n%s", jsonResponse);
				for (auto it : reader.GetErrors()) {
					wxLogMessage("NOAA Weather Plugin, Json parser error: %s", it);
				}
			}
			else {
				// If there are no warnings, the "features" array is empty
				if (root["features"].IsArray() && (root["features"].Size() > 0)) {
					wxMessageBox(root["features"][0]["properties"]["headline"].AsString() +
						+"\n" + root["features"][0]["properties"]["description"].AsString(),
						root["features"][0]["properties"]["event"].AsString(),
						wxICON_WARNING);
				}
				else {
					wxMessageBox("No alerts issued by NWS, but the prudent mariner will check other information sources",
						_T(PLUGIN_COMMON_NAME), wxICON_INFORMATION);
				}
			}
		}

		// Weather Reports
		if (menuId == noaaBuoyMenu) {

			// Iterate over the list of visible buoys and find the matching report
			if (useScheduled) {
				std::string sid = id.ToStdString();
				const auto p = std::find_if(visibleBuoys.begin(), visibleBuoys.end(),
					[sid](const BuoyData& a) { return a.id == sid; });

				if (p != visibleBuoys.end()) {
					wxMessageBox(wxString::Format("Wind Direction: %d\nWind Speed %0.2f\nPressure: %0.2f\nTemperature: %0.2f",
						p->windDirection, p->windSpeed, p->barometricPressure, p->airTemperature), p->id);
				}
			}
			else {
				DownloadRealtimeObservation(id, name);
			}
		}
	}
}

// Drawing on the canvas when in multi canvas mode and not using OpenGL
bool NOAA_Plugin::RenderOverlayMultiCanvas(wxDC& dc, PlugIn_ViewPort* vp,
	int canvasIndex, int priority) {

	// Only draw in legacy mode
	if (priority == OVERLAY_LEGACY) {

		if (dc.IsOk()) {

			// Render the NDBC Buoys
			if (canvasIndex == 0) {
				for (auto it : visibleBuoys) {
					wxPoint wxP;
					GetCanvasPixLL(vp, &wxP, it.latitude, it.longitude);
					dc.DrawBitmap(buoyBitmap, wxP.x, wxP.y, true);
				}
			}
			return true;
		}
		else {
			wxLogDebug("NOAA Weather Plugin, Canvas DC not OK");
			return false;
		}
	}
	else {
		wxLogDebug("NOAA Weather Plugin, OpenCPN not using legacy priority. Current priority: %d", priority);
		return false;
	}
}

// Drawing on the canvas when using OpenGL and in multi canvas mode
bool NOAA_Plugin::RenderGLOverlayMultiCanvas(wxGLContext* pcontext, PlugIn_ViewPort* vp,
	int canvasIndex, int priority) {

	// BUG BUG No idea what the other priorities do?? OVERLAY_OVER_SHIPS, OVERLAY_OVER_UI,
	if (priority == OVERLAY_OVER_EMBOSS) {

		if (pcontext->IsOK()) {

			NOAA_Graphics* glRenderer = new NOAA_Graphics();

			if (canvasIndex == 0) {
				// Render the NDBC Buoys
				for (auto it : visibleBuoys) {
					wxPoint wxP;
					GetCanvasPixLL(vp, &wxP, it.latitude, it.longitude);
					glRenderer->DrawBitmap(buoyBitmap, wxP.x, wxP.y, true);
				}
			}

			delete glRenderer;

			return true;
		}
		else {
			wxLogDebug("NOAA Weather Plugin, Canvas GLContext not OK");
			return false;
		}
	}
	else {
		wxLogDebug("NOAA Weather Plugin, OpenCPN not in Emboss Mode, Current Mode: %d", priority);
		return false;
	}
}

// Generated a filtered list of stations that are bounded within the View Port
std::vector<BuoyData> NOAA_Plugin::FilterVisibleBuoys(const PlugIn_ViewPort &vp) {

	std::vector<BuoyData> results;
	for (auto it : allBuoys) { 
		if ((it.latitude >= vp.lat_min) && (it.latitude <= vp.lat_max)) {
			if ((it.longitude >= vp.lon_min) && (it.longitude <= vp.lon_max)) {
				results.push_back(it);
			}
		}
	}
	return results;
}

// Determine if any buoy is under the cursor
// BUG BUG Is the 'wiggle' factor sufficient ? 
// Could calculate based on the chart scale and the pixel size of the icon
bool NOAA_Plugin::IsUnderCursor(double lat, double lon, wxString *id, wxString *name) {

	for (auto it : visibleBuoys) { 
		if ((it.latitude >= lat - 0.15) && (it.latitude <= lat + 0.15)) {
			if ((it.longitude >= lon - 0.15) && (it.longitude <= lon + 0.15)) {
				*id = it.id;
				*name = it.name;
				return true;
			}
		}
	}
	return false;
}

// Extract Latitude and Longitude as doubles from the station list file (see below). The string has the Unicode degree character
// 12.000 N 23.000 W (12&#176;0'0" N 23&#176;0'0" W)
void NOAA_Plugin::ParsePosition(wxString location, double* latitude, double* longitude) {
	
	wxRegEx re("([0-9]{1,2}.[0-9]{3})\\s(N|S)\\s([0-9]{1,3}.[0-9]{3})\\s(E|W)*");

	if (re.Matches(location)) {

		re.GetMatch(location, 1).ToDouble(latitude);
		if (re.GetMatch(location, 2) == 'S') {
			*latitude *= -1;
		}

		re.GetMatch(location, 3).ToDouble(longitude);
		if (re.GetMatch(location, 4) == 'W') {
			*longitude *= -1;
		}
	}
	else {
		// Register an error, perhaps in case NOAA changes the format
		wxLogMessage("NOAA Weather Plugin, Error parsing station list position: %s", location);
	}
}


// Download the National Data Buoy Centre Station List
// This is a superset of all weather observations and the station id serves
// as a reference to locate each station's realtime observations
bool NOAA_Plugin::DownloadStationList(void) {

	// Download the file
	wxString fileName = wxStandardPaths::Get().GetDocumentsDir() + wxFileName::GetPathSeparator() + "station_table.txt";
	if (DownloadFile("https://www.ndbc.noaa.gov/data/stations/station_table.txt", fileName)) {

		// Parse the file and populate the list of stations
		wxTextFile textFile;
		textFile.Open(fileName);

		wxString line;
		BuoyData buoy;

		// Read past the first two lines which are headers
		textFile.GetFirstLine();
		textFile.GetNextLine();

		// Read remaining lines one by one, until the end of the file
		while (!textFile.Eof()) {
			line = textFile.GetNextLine();

			// Process each line of station data, extracting the Id, Name and Location
			// Each element is separated by the '|' character. For example:
			// 13002|PR|Atlas Buoy||NE Extension||21.000 N 23.000 W (21&#176;0'0" N 23&#176;0'0" W)|| |

			wxStringTokenizer tokenizer(line, "|");

			buoy.id = tokenizer.GetNextToken();
			// Skip the next few tokens
			tokenizer.GetNextToken();
			tokenizer.GetNextToken();
			tokenizer.GetNextToken();
			buoy.name = tokenizer.GetNextToken();
			tokenizer.GetNextToken();
			// Parse the location data	
			ParsePosition(tokenizer.GetNextToken(), &buoy.latitude, &buoy.longitude);
			//ParsePosition(tokenizer.GetNextToken().ToStdString(), &buoy.latitude, &buoy.longitude);
			allBuoys.push_back(buoy);
		}
		textFile.Close();
		return true;
	}
	return false;
}

// Given a station id from the station list retrieve the realtime observations
// Not all stations have realtime observations that are of interest to us.
// Some are waveriders, atmospheric stations etc. so the download may fail as
// the different types of reports have file extensions different to .txt
void NOAA_Plugin::DownloadRealtimeObservation(wxString id, wxString name) {

	// Construct the URL
	wxString url = "https://www.ndbc.noaa.gov/data/realtime2/";
	url.append(id);
	url.append(".txt");

	wxLogMessage("NOAA Weather Plugin, Downloading Station: %s, url: %s", id, url);

	// Regular Expression to parse realtime observations
	wxRegEx regex("(\\b(MM)|([A-Z0-9]{4,6})|((-?\\d+\\.)?\\d+)\\b)");

	// BUG BUG Could download as a file and process similarly to scheduled reports
	// Fetch the station's realtime  weather observation
	wxString data = ExecuteQuery(url);

	if (data.Length() > 0) {
		BuoyData buoy;
		wxStringTokenizer tokenizer(data, "\n\r");

		// Read past the first two lines which are headers
		tokenizer.GetNextToken();
		tokenizer.GetNextToken();

		// Read the next line which contains the last reported realtime observation
		// BUG BUG Could read subsequent lines to display a history
		wxString line = tokenizer.GetNextToken();
		
		// A sample looks like the following. Annoyingly, spaces are used to align it on a text page
		// #YY  MM DD hh mm WDIR WSPD GST  WVHT   DPD   APD MWD   PRES  ATMP  WTMP  DEWP  VIS PTDY  TIDE
		// #yr  mo dy hr mn degT m/s   m/s   m     sec   sec degT  hPa  degC  degC  degC   nmi  hPa    ft
		// 2025 04 04 05 00  27  3.7   MM    MM    MM    MM  MM     MM  30.2    MM    MM   MM   MM    MM

		wxString remainder;
		remainder = line;
		// Loop through each matching group
		int j = 0;
		
		while (regex.Matches(remainder)) {

			size_t start, len;
			regex.GetMatch(&start, &len, 0);

			// Populate the buoy data
			if (j == 5) {
				regex.GetMatch(remainder, 1).ToInt(&buoy.windDirection);
			}
			if (j == 6) {
				regex.GetMatch(remainder, 1).ToDouble(&buoy.windSpeed);
			}
			if (j == 12) {
				regex.GetMatch(remainder, 1).ToDouble(&buoy.barometricPressure);
			}
			if (j == 13) {
				regex.GetMatch(remainder, 1).ToDouble(&buoy.airTemperature);
			}
			remainder = remainder.Mid(start + len);
			j++;
		}
		wxMessageBox(wxString::Format("Wind Direction: %d\nWind Speed %0.2f\nPressure: %0.2f\nTemperature: %0.2f",
			buoy.windDirection, buoy.windSpeed, buoy.barometricPressure, buoy.airTemperature), id);
	}
	else {
		wxMessageBox("This buoy does not support weather observations",_T(PLUGIN_COMMON_NAME), wxICON_INFORMATION);
	}
}

// BUG BUG Add Date Time fields
// BUG BUG Format using user's speed and temperature units
// Download scheduled reports
// The format of this data is slightly different to a realtime obsservation as it includes the station id
bool NOAA_Plugin::DownloadScheduledReports(void) {

	// Download the file, Should probably name the file using some data time information
	wxString fileName = wxStandardPaths::Get().GetDocumentsDir() + wxFileName::GetPathSeparator() + "observations.txt";
	if (DownloadFile("https://www.ndbc.noaa.gov/data/latest_obs/latest_obs.txt", fileName)) {

		// The weather observations are in the following format. Annoyingly, spaces are used to align it on a text page
		// #STN       LAT      LON  YYYY MM DD hh mm WDIR WSPD   GST WVHT  DPD APD MWD   PRES  PTDY  ATMP  WTMP  DEWP  VIS   TIDE
		// #text      deg      deg   yr mo day hr mn degT  m/s   m/s   m   sec sec degT   hPa   hPa  degC  degC  degC  nmi     ft
		// 13001    12.000  -23.000 2025 04 07 15 00 356   6.8   8.0   MM  MM   MM  MM 1011.1    MM  23.3  24.0    MM   MM     MM

		// Parse the file and extract the weather observations
		wxTextFile textFile;
		textFile.Open(fileName);

		wxString line;
		wxString remainder;
		BuoyData buoy;
		allBuoys.clear();

		// Regular Expression to parse weather observations
		wxRegEx regex("(\\b(MM)|([A-Z0-9]{4,6})|((-?\\d+\\.)?\\d+)\\b)");

		// Read past the first two lines which are headers
		textFile.GetFirstLine();
		textFile.GetNextLine();

		// Read remaining lines one by one, until the end of the file
		while (!textFile.Eof()) {
			line = textFile.GetNextLine();

			remainder = line;
			// Loop through each matching group
			int j = 0;

			while (regex.Matches(remainder)) {

				size_t start, len;
				regex.GetMatch(&start, &len, 0);

				// Populate the buoy data
				if (j == 0) {
					buoy.id = regex.GetMatch(remainder, 1);
				}
				if (j == 1) {
					regex.GetMatch(remainder, 1).ToDouble(&buoy.latitude);
				}
				if (j == 2) {
					regex.GetMatch(remainder, 1).ToDouble(&buoy.longitude);
				}
				if (j == 8) {
					regex.GetMatch(remainder, 1).ToInt(&buoy.windDirection);
				}
				if (j == 9) {
					regex.GetMatch(remainder, 1).ToDouble(&buoy.windSpeed);
				}
				if (j == 15) {
					regex.GetMatch(remainder, 1).ToDouble(&buoy.barometricPressure);
				}
				if (j == 17) {
					regex.GetMatch(remainder, 1).ToDouble(&buoy.airTemperature);
				}
				allBuoys.push_back(buoy);
				remainder = remainder.Mid(start + len);
				j++;
			}
		}
		textFile.Close();
		return true;
	}
	return false;
}

// Retrieve a file such as the station list or scheduled reports from the NOAA NDBC Web Site
// BUG BUG Could refactor ExecuteQuery  rather than duplicate code....
bool NOAA_Plugin::DownloadFile(wxString url, wxString filename) {

	// Download and save the file to a specified location
	wxURI uri(url);

	_OCPN_DLStatus returnCode = OCPN_downloadFile(uri.BuildURI(), filename,
		_T(PLUGIN_COMMON_NAME), wxEmptyString, pluginBitmap, NULL,
		OCPN_DLDS_ELAPSED_TIME | OCPN_DLDS_AUTO_CLOSE, 10);


	if (returnCode == OCPN_DL_NO_ERROR) {
		wxLogMessage("NOAA Weather Plugin, Successfully downloaded %s to %s", url, filename);
		return true;
	}
	else {
		wxLogMessage("NOAA Weather Plugin, Error %d downloading URL: %s", returnCode, url);
		wxMessageBox(wxString::Format("Download error, Please review log for further details"),
			_T(PLUGIN_COMMON_NAME), wxICON_ERROR);
	}
	return false;
}

// Retrieve the url from NOAA from which to find a forecast given a vessel's position
wxString NOAA_Plugin::GetForecastUrl(const double &latitude, const double &longitude) {

	wxString url = wxString::Format("https://api.weather.gov/points/%f,%f", latitude, longitude);
	wxString response = wxEmptyString;

	wxString jsonResponse = ExecuteQuery(url);
	
	wxJSONReader reader;
	wxJSONValue root;

	// Check for any JSON parsing errors
	if (reader.Parse(jsonResponse, &root) > 0) {
			wxLogMessage("NOAA Weather Plugin, Json parser error(s) in the following response:\n%s", jsonResponse);
			for (auto it : reader.GetErrors()) {
				wxLogMessage("NOAA Weather Plugin, Json parser error: %s", it);
			}
	}
	else {
		if (root["properties"].HasMember("forecastGridData")) {
			response = root["properties"]["forecastGridData"].AsString();
		}
	}
	return response;
}

// Just a wrapper around the OpenCPN API which in turn wraps the wxCurl API
wxString NOAA_Plugin::ExecuteQuery(const wxString endpoint) {

	wxString response = wxEmptyString;
	wxURI uri(endpoint);
	wxString temporaryFileName = wxFileName::CreateTempFileName("tmp");

	_OCPN_DLStatus returnCode = OCPN_downloadFile(uri.BuildURI(), temporaryFileName,
		_T(PLUGIN_COMMON_NAME), wxEmptyString, pluginBitmap, NULL,
		OCPN_DLDS_ELAPSED_TIME | OCPN_DLDS_AUTO_CLOSE, 10);

	if (returnCode == OCPN_DL_NO_ERROR) {
		wxFile dataFile;
		dataFile.Open(temporaryFileName, wxFile::read);
		dataFile.ReadAll(&response);
	}
	else {
		wxLogMessage("NOAA Weather Plugin, Error %d dowloading URL: %d", returnCode, endpoint);
		wxMessageBox(wxString::Format("Download error, Please review log for further details"), 
			_T(PLUGIN_COMMON_NAME), wxICON_ERROR);
	}

	// Try to clean up...
	wxRemoveFile(temporaryFileName);

	return response;
}
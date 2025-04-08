// Copyright(C) 2018-2020 by Steven Adler
//
// This file is part of NOAA Weather plugin for OpenCPN.
//
// NOAA Weather plugin for OpenCPN is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// NOAA Weather plugin for OpenCPN is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with the NOAA Weather plugin for OpenCPN. If not, see <https://www.gnu.org/licenses/>.
//


#ifndef NOAA_WEATHER_PLUGIN_H
#define NOAA_WEATHER_PLUGIN_H

// Pre compiled headers 
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
      #include <wx/wx.h>
#endif

// Pre compiled headers 
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

// Defines version numbers, names etc. for this plugin
// This is automagically constructed via version.h.in from CMakeLists.txt, personally I think this is convoluted
#include "version.h"

// OpenCPN include file
#include "ocpn_plugin.h"

// wxJSON (used for parsing OCPN Messaging)
#include "wx/json_defs.h"
#include "wx/jsonreader.h"
#include "wx/jsonval.h"
#include "wx/jsonwriter.h"

// OpenGL 
#include "pidc.h"

// OpenCPN Device Context Abstraction Layer
#include "noaa_weather_graphics.h"

// Dialog to display weather forecast data
#include "noaa_weather_dialog.h"

// wxWidgets include files

// Configuration
#include <wx/fileconf.h>

// Strings
#include <wx/string.h>
#include <wx/tokenzr.h>
#include <wx/regex.h>

// File handling
#include <wx/stdpaths.h>
#include <wx/filename.h>

// Web Access
#include <wx/uri.h>

// STL
#include <string>
#include <vector>
#include <regex>

// NDBC Station data
typedef struct _buoydata {
	std::string id;
	std::string name;
	double latitude;
	double longitude;
	double windSpeed;
	int windDirection;
	double barometricPressure;
	double airTemperature;
} BuoyData;

// Used to determine what query to send (not used anywhere ?)
typedef enum _nooa {
	FORECAST = 0,	// A forecast for an area 
	ALERT = 1,		// Weather alerts for an area
	BOUY = 2,		// Scheduled observations
	REALTIME = 3	// Realtime observations
} NOAA_REQUEST;

// The NOAA Weather plugin
class NOAA_Plugin : public opencpn_plugin_118 {

public:
	// The constructor
	NOAA_Plugin(void *ppimgr);
	
	// and destructor
	~NOAA_Plugin(void);

	// Overridden OpenCPN plugin methods
	int Init(void);
	bool DeInit(void);
	int GetAPIVersionMajor();
	int GetAPIVersionMinor();
	int GetPlugInVersionMajor();
	int GetPlugInVersionMinor();
	wxString GetCommonName();
	wxString GetShortDescription();
	wxString GetLongDescription();
	void SetPositionFix(PlugIn_Position_Fix &pfix);
	void OnContextMenuItemCallback(int id);
	wxBitmap *GetPlugInBitmap();
	bool RenderGLOverlayMultiCanvas(wxGLContext* pcontext, PlugIn_ViewPort* vp,
		int canvasIndex, int priority);
	bool RenderOverlayMultiCanvas(wxDC& dc, PlugIn_ViewPort* vp,
		int canvasIndex, int priority);
	void SetCursorLatLon(double lat, double lon);
	void SetCurrentViewPort(PlugIn_ViewPort& vp);
	bool MouseEventHook(wxMouseEvent& event);
	//int GetToolbarToolCount(void);
	//int GetToolbarItemId(void);
	//void OnToolbarToolCallback(int id);
	
private: 
	
	// The viewing area of the chart
	PlugIn_ViewPort viewPort;

	// Bitmap used to draw a weather buoy on the chart
	wxBitmap buoyBitmap;

	// Reference to the OpenCPN window handle
	wxWindow *parentWindow;

	// OpenCPN Configuration Setings (unused)
	wxFileConfig* configSettings;

	// OpenCPN Vessel Position - used to fetch area forecasts and warnings
	double currentLatitude;
	double currentLongitude;

	// Toolbar id
	//int toolbarId;

	// Context Menus
	int noaaAlertMenu;
	int noaaForecastMenu;
	int noaaBuoyMenu;

	wxString GetForecastUrl(const double &latitude, const double &longitude);
	wxString ExecuteQuery(const wxString endpoint);
	void ParsePosition(wxString location, double* latitude, double* longitude);
	std::vector<BuoyData> FilterVisibleBuoys(const PlugIn_ViewPort& vp);
	bool IsUnderCursor(double lat, double lon, wxString *id, wxString *name);
	void DownloadRealtimeObservation(wxString id, wxString name);
	bool DownloadScheduledReports(void);
	bool DownloadStationList(void);
	bool DownloadFile(wxString url, wxString filename);

	// NOAA NDBC Station List
	std::vector<BuoyData> allBuoys;
	std::vector<BuoyData> visibleBuoys;

	// Station Id & Name
	wxString id;
	wxString name;

	// Whether to use scheduled reports or realtime observations
	bool useScheduled = false;
};

#endif
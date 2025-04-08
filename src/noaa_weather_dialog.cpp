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
// along with the NOAA Weather plugin for OpenCPN. If not, see <https://www.``gnu.org/licenses/>.
//


// Project: NOAA Weather plugin
// Description: NOOA Weather plugin for OpenCPN
// Owner: twocanplugin@hotmail.com
// Date: 6/1/2020
// Version History: 
// 1.0 Initial Release
//

#include "noaa_weather_dialog.h"

// Plugin bitmap
wxBitmap pluginBitmap;

// Constructor and destructor implementation
NOAA_Plugin_Dialog::NOAA_Plugin_Dialog( wxWindow* parent, const wxString jsonResponse) : NOAA_Plugin_DialogBase(parent) {
	// Set the dialog's icon
	wxIcon icon;
	icon.CopyFromBitmap(pluginBitmap);
	NOAA_Plugin_Dialog::SetIcon(icon);

	// Populate the grid
	wxJSONReader reader;
	wxJSONValue root;
	wxJSONValue values;
	reader.Parse(jsonResponse, &root);

	values = root["properties"]["temperature"]["values"];
	// resize the grid with the correct number of rows.
	dataGrid->AppendRows(values.Size() - dataGrid->GetNumberRows());

	// Set row labels for date/time, insert cell values for temperature
	for (int i = 0; i < values.Size(); i++) {
		dataGrid->SetRowLabelValue(i, values[i]["validTime"].AsString());
		// Don't need more than 2 decimal places !!
		dataGrid->SetCellValue(i, 0, wxString::Format("%0.2f", values[i]["value"].AsDouble()));
	}

	// insert cell values for wind direction
	values = root["properties"]["windDirection"]["values"];
	for (int i = 0; i < values.Size(); i++) {
		for (int j = 0; j < dataGrid->GetNumberRows(); j++) {
			if (dataGrid->GetRowLabelValue(j) == values[i]["validTime"].AsString()) {
				dataGrid->SetCellValue(j, 1, std::to_string(values[i]["value"].AsInt()));
			}
		}
	}

	// and wind speed
	values = root["properties"]["windSpeed"]["values"];
	for (int i = 0; i < values.Size(); i++) {
		for (int j = 0; j < dataGrid->GetNumberRows(); j++) {
			if (dataGrid->GetRowLabelValue(j) == values[i]["validTime"].AsString()) {
				// Don't need more than 2 decimal places !!
				dataGrid->SetCellValue(j, 2, wxString::Format("%0.2f", values[i]["value"].AsDouble()));
			}
		}

		// Trim the date/time display
		// Existing format is "2021-12-05T17:00:00+00:00/PT1H"
		// /PT1H I think means Period Time 1 hour, ie the next valid period
		for (int j = 0; j < dataGrid->GetNumberRows(); j++) {
			wxString label = wxString::Format("%s %s", dataGrid->GetRowLabelValue(j).Mid(0, 10),
			dataGrid->GetRowLabelValue(j).Mid(11, 5));
			dataGrid->SetRowLabelValue(j, label);
		}
	}
}

NOAA_Plugin_Dialog::~NOAA_Plugin_Dialog() {
}

void NOAA_Plugin_Dialog::OnInit(wxInitDialogEvent& event) {

	wxSize newSize = this->GetSize();
	dataGrid->SetMinSize(wxSize(512, 20 * dataGrid->GetDefaultRowSize()));
	dataGrid->SetMaxSize(wxSize(-1, 20 * dataGrid->GetDefaultRowSize()));

	Fit();

	// After we've fitted in everything adjust the dataGrid column widths
	int colWidth = (int)((dataGrid->GetSize().GetWidth() - dataGrid->GetRowLabelSize() - wxSystemSettings::GetMetric(wxSYS_VSCROLL_X, NULL)) / 3);
	dataGrid->SetColSize(0, colWidth);
	dataGrid->SetColSize(1, colWidth);
	dataGrid->SetColSize(2, colWidth);
}

void NOAA_Plugin_Dialog::OnClose(wxCommandEvent &event) {
	this->Close(true);
}
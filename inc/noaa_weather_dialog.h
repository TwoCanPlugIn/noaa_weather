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
// along with the Racing plugin for OpenCPN. If not, see <https://www.gnu.org/licenses/>.
//

#ifndef NOAA_WEATHER_DIALOG_H
#define NOAA_WEATHER_DIALOG_H

// wxWidgets Precompiled Headers
#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif

// The base class from which we are derived
// Note wxFormBuilder used to generate UI
#include "noaa_weather_dialogbase.h"

// wxJSON (used for parsing SignalK data)
#include "wx/json_defs.h"
#include "wx/jsonreader.h"
#include "wx/jsonval.h"
#include "wx/jsonwriter.h"
#include <wx/stdpaths.h>

// image for dialog icon
extern wxBitmap pluginBitmap;

class NOAA_Plugin_Dialog : public NOAA_Plugin_DialogBase {
	
public:
	NOAA_Plugin_Dialog(wxWindow* parent, const wxString jasonResponse);
	~NOAA_Plugin_Dialog();
		
protected:
	//overridden methods from the base class
	void OnInit(wxInitDialogEvent& event);
	void OnClose(wxCommandEvent &event);
	
private:
	
};

#endif

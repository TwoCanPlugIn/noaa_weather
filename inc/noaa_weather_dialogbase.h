///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.2.1-0-g80c4cb6)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/font.h>
#include <wx/grid.h>
#include <wx/gdicmn.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/frame.h>

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class NOAA_Plugin_DialogBase
///////////////////////////////////////////////////////////////////////////////
class NOAA_Plugin_DialogBase : public wxFrame
{
	private:

	protected:
		wxGrid* dataGrid;
		wxButton* buttonClose;

		// Virtual event handlers, override them in your derived class
		virtual void OnClose( wxCommandEvent& event ) { event.Skip(); }


	public:

		NOAA_Plugin_DialogBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("NOAA Weather Forecast"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 521,364 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );

		~NOAA_Plugin_DialogBase();

};


///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.2.1-0-g80c4cb6)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "noaa_weather_dialogbase.h"

///////////////////////////////////////////////////////////////////////////

NOAA_Plugin_DialogBase::NOAA_Plugin_DialogBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* sizerDialog;
	sizerDialog = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* sizerList;
	sizerList = new wxBoxSizer( wxVERTICAL );

	dataGrid = new wxGrid( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );

	// Grid
	dataGrid->CreateGrid( 1, 3 );
	dataGrid->EnableEditing( false );
	dataGrid->EnableGridLines( true );
	dataGrid->EnableDragGridSize( false );
	dataGrid->SetMargins( 0, 0 );

	// Columns
	dataGrid->EnableDragColMove( false );
	dataGrid->EnableDragColSize( true );
	dataGrid->SetColLabelValue( 0, wxT("Temp") );
	dataGrid->SetColLabelValue( 1, wxT("Wind Dir") );
	dataGrid->SetColLabelValue( 2, wxT("Wind Speed") );
	dataGrid->SetColLabelSize( wxGRID_AUTOSIZE );
	dataGrid->SetColLabelAlignment( wxALIGN_CENTER, wxALIGN_CENTER );

	// Rows
	dataGrid->EnableDragRowSize( false );
	dataGrid->SetRowLabelSize( 150 );
	dataGrid->SetRowLabelAlignment( wxALIGN_LEFT, wxALIGN_CENTER );

	// Label Appearance

	// Cell Defaults
	dataGrid->SetDefaultCellAlignment( wxALIGN_LEFT, wxALIGN_TOP );
	sizerList->Add( dataGrid, 0, wxALL|wxEXPAND, 5 );


	sizerDialog->Add( sizerList, 1, wxEXPAND, 5 );

	wxBoxSizer* sizerButtons;
	sizerButtons = new wxBoxSizer( wxHORIZONTAL );

	buttonClose = new wxButton( this, wxID_ANY, wxT("Close"), wxDefaultPosition, wxDefaultSize, 0 );
	sizerButtons->Add( buttonClose, 0, wxALL, 5 );


	sizerDialog->Add( sizerButtons, 0, wxEXPAND|wxFIXED_MINSIZE, 5 );


	this->SetSizer( sizerDialog );
	this->Layout();

	this->Centre( wxBOTH );

	// Connect Events
	buttonClose->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( NOAA_Plugin_DialogBase::OnClose ), NULL, this );
}

NOAA_Plugin_DialogBase::~NOAA_Plugin_DialogBase()
{
	// Disconnect Events
	buttonClose->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( NOAA_Plugin_DialogBase::OnClose ), NULL, this );

}

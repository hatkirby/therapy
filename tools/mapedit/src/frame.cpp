#include "frame.h"
#include "widget.h"
#include "tile_widget.h"
#include <wx/splitter.h>
#include "panel.h"

enum {
  MENU_VIEW_ZOOM_IN,
  MENU_VIEW_ZOOM_OUT
};

wxBEGIN_EVENT_TABLE(MapeditFrame, wxFrame)
  EVT_MENU(wxID_EXIT, MapeditFrame::OnExit)
  EVT_MENU(MENU_VIEW_ZOOM_IN, MapeditFrame::ZoomIn)
  EVT_MENU(MENU_VIEW_ZOOM_OUT, MapeditFrame::ZoomOut)
wxEND_EVENT_TABLE()

MapeditFrame::MapeditFrame(Map map) : wxFrame(NULL, wxID_ANY, "Map Editor", wxPoint(50, 50), wxSize(GAME_WIDTH*3, GAME_HEIGHT*2)), map(map)
{
  wxMenu* menuFile = new wxMenu;
  menuFile->Append(wxID_EXIT);
  
  wxMenu* menuView = new wxMenu;
  menuView->Append(MENU_VIEW_ZOOM_IN, "Zoom In\tCtrl-+");
  menuView->Append(MENU_VIEW_ZOOM_OUT, "Zoom Out\tCtrl--");

  wxMenuBar* menuBar = new wxMenuBar;
  menuBar->Append(menuFile, "&File");
  menuBar->Append(menuView, "&View");
  
  SetMenuBar(menuBar);
  
  wxBoxSizer* sizermain = new wxBoxSizer(wxVERTICAL);
  wxSplitterWindow* splitter = new wxSplitterWindow(this, wxID_ANY);
  splitter->SetSashGravity(0.0);
  splitter->SetMinimumPaneSize(50);
  sizermain->Add(splitter, 1, wxEXPAND, 0);
  
  wxPanel* tileEditorPanel = new wxPanel(splitter, wxID_ANY);
  tileEditor = new TileWidget(tileEditorPanel, wxID_ANY, 6, 6, wxPoint(0,0));
  wxBoxSizer* tileSizer = new wxBoxSizer(wxVERTICAL);
  tileSizer->Add(tileEditor, 1, wxEXPAND, 0);
  tileEditorPanel->SetSizer(tileSizer);
  
  wxPanel* mapEditorPanel = new wxPanel(splitter, wxID_ANY);
  mapEditor = new MapeditWidget(mapEditorPanel, wxID_ANY, &this->map, tileEditor, wxPoint(0,0));
  wxBoxSizer* mapSizer = new wxBoxSizer(wxVERTICAL);
  mapSizer->Add(mapEditor, 1, wxEXPAND, 0);
  mapEditorPanel->SetSizer(mapSizer);
  
  splitter->SplitVertically(tileEditorPanel, mapEditorPanel);
  
  this->SetSizer(sizermain);
  sizermain->SetSizeHints(this);
}

void MapeditFrame::OnExit(wxCommandEvent& event)
{
  Close(true);
}

MapeditWidget* MapeditFrame::GetMapEditor()
{
  return mapEditor;
}

void MapeditFrame::ZoomIn(wxCommandEvent& event)
{
  mapEditor->ZoomIn();
}

void MapeditFrame::ZoomOut(wxCommandEvent& event)
{
  mapEditor->ZoomOut();
}

#include "frame.h"
#include "widget.h"
#include "tile_widget.h"

wxBEGIN_EVENT_TABLE(MapeditFrame, wxFrame)
  EVT_MENU(wxID_EXIT, MapeditFrame::OnExit)
wxEND_EVENT_TABLE()

MapeditFrame::MapeditFrame(Map map) : wxFrame(NULL, wxID_ANY, "Map Editor", wxPoint(50, 50), wxSize(GAME_WIDTH*3, GAME_HEIGHT*2)), map(map)
{
  wxMenu* menuFile = new wxMenu;
  menuFile->Append(wxID_EXIT);

  wxMenuBar* menuBar = new wxMenuBar;
  menuBar->Append(menuFile, "&File");
  
  SetMenuBar(menuBar);
  
  wxPanel* panel = new wxPanel(this, wxID_ANY);
  int clientWidth, clientHeight;
  GetClientSize(&clientWidth, &clientHeight);
  
  TileWidget* tileEdit = new TileWidget(panel, wxID_ANY, 6, wxPoint(0,0), wxSize(TILE_WIDTH*3*6, clientHeight));
  MapeditWidget* wid = new MapeditWidget(panel, wxID_ANY, &this->map, tileEdit, wxPoint(TILE_WIDTH*3*6+8,0), wxSize(GAME_WIDTH*1.5, GAME_HEIGHT*1.5));
}

void MapeditFrame::OnExit(wxCommandEvent& event)
{
  Close(true);
}

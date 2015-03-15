#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "map.h"

class MapeditApp : public wxApp {
  public:
    virtual bool OnInit();
};

class MapeditFrame : public wxFrame {
  public:
    MapeditFrame() : MapeditFrame(Map()) {}
    MapeditFrame(Map map);
    
  private:
    void OnExit(wxCommandEvent& event);
    
    Map map;
    
    wxDECLARE_EVENT_TABLE();
};

wxBEGIN_EVENT_TABLE(MapeditFrame, wxFrame)
  EVT_MENU(wxID_EXIT, MapeditFrame::OnExit)
wxEND_EVENT_TABLE()

wxIMPLEMENT_APP(MapeditApp);

bool MapeditApp::OnInit()
{
  MapeditFrame* frame = new MapeditFrame();
  frame->Show(true);
  return true;
}

MapeditFrame::MapeditFrame(Map map) : wxFrame(NULL, wxID_ANY, "Map Editor", wxPoint(50, 50), wxSize(450, 340)), map(map)
{
  wxMenu* menuFile = new wxMenu;
  menuFile->Append(wxID_EXIT);

  wxMenuBar* menuBar = new wxMenuBar;
  menuBar->Append(menuFile, "&File");
  
  SetMenuBar(menuBar);
}

void MapeditFrame::OnExit(wxCommandEvent& event)
{
  Close(true);
}

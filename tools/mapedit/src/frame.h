#ifndef FRAME_H
#define FRAME_H

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "map.h"
#include "widget.h"
#include "tile_widget.h"

class MapeditFrame : public wxFrame {
  public:
    MapeditFrame() : MapeditFrame(Map(), "") {}
    MapeditFrame(Map map, std::string filename);
    
    MapeditWidget* GetMapEditor();

  private:
    void ZoomIn(wxCommandEvent& event);
    void ZoomOut(wxCommandEvent& event);
    void OnNew(wxCommandEvent& event);
    void OnOpen(wxCommandEvent& event);
    void OnSave(wxCommandEvent& event);
    void OnClose(wxCommandEvent& event);
    void OnExit(wxCloseEvent& event);
    void OnQuit(wxCommandEvent& event);
    
    Map map;
    MapeditWidget* mapEditor;
    TileWidget* tileEditor;
    std::string filename;
    
    wxDECLARE_EVENT_TABLE();
};

#endif

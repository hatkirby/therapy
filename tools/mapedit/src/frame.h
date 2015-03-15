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
    MapeditFrame() : MapeditFrame(Map()) {}
    MapeditFrame(Map map);
    
    MapeditWidget* GetMapEditor();

  protected:
    void ZoomIn(wxCommandEvent& event);
    void ZoomOut(wxCommandEvent& event);
    
  private:
    void OnExit(wxCommandEvent& event);
    
    Map map;
    MapeditWidget* mapEditor;
    TileWidget* tileEditor;
    
    wxDECLARE_EVENT_TABLE();
};

#endif

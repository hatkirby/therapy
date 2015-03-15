#ifndef FRAME_H
#define FRAME_H

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "map.h"

class MapeditFrame : public wxFrame {
  public:
    MapeditFrame() : MapeditFrame(Map()) {}
    MapeditFrame(Map map);
    
  private:
    void OnExit(wxCommandEvent& event);
    
    Map map;
    
    wxDECLARE_EVENT_TABLE();
};

#endif

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "map.h"
#include "frame.h"

class MapeditApp : public wxApp {
  public:
    virtual bool OnInit();
};

wxIMPLEMENT_APP(MapeditApp);

bool MapeditApp::OnInit()
{
  wxInitAllImageHandlers();
  
  MapeditFrame::NewMap();
  
  return true;
}

#ifndef TILE_WIDGET_H
#define TILE_WIDGET_H

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

class TileWidget : public wxControl {
  public:
    TileWidget();
    TileWidget(wxWindow* parent, wxWindowID winid, int numTilesWidth = 8, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
    
    int getSelected();
    
  protected:
    void Init();
    void OnPaint(wxPaintEvent& event);
    void OnClick(wxMouseEvent& event);
    
  private:
    int numTilesWidth;
    wxBitmap tiles;
    int numTiles;
    int selected = 0;
    
    DECLARE_DYNAMIC_CLASS(MapeditWidget);
    DECLARE_EVENT_TABLE();
};
    
#endif

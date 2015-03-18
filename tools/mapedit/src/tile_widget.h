#ifndef TILE_WIDGET_H
#define TILE_WIDGET_H

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

class TileWidget : public wxScrolledCanvas {
  public:
    TileWidget();
    TileWidget(wxWindow* parent, wxWindowID winid, int width, int scale, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
    
    int getSelected();
    
  protected:
    void Init();
    void OnPaint(wxPaintEvent& event);
    void OnClick(wxMouseEvent& event);
    
  private:
    int numTilesWidth;
    wxBitmap tiles;
    int numTiles = 50;
    int selected = 0;
    int scale;
    
    DECLARE_DYNAMIC_CLASS(MapeditWidget)
    DECLARE_EVENT_TABLE()
};
    
#endif

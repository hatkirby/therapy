#ifndef WIDGET_H
#define WIDGET_H

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "map.h"
#include "tile_widget.h"

class MapeditWidget : public wxScrolledWindow {
  public:
    MapeditWidget();
    MapeditWidget(wxWindow* parent, wxWindowID winid, Map* map, TileWidget* tileWidget, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
    
    void ZoomIn();
    void ZoomOut();
    
  protected:
    void Init();
    virtual wxSize DoGetBestSize() const;
    void OnPaint(wxPaintEvent& event);
    void OnClick(wxMouseEvent& event);
    void OnMouseMove(wxMouseEvent& event);
    void OnMouseUp(wxMouseEvent& event);
    void OnMouseOut(wxMouseEvent& event);
    
  private:
    void SetTile(wxPoint pos);
    void SetZoomSize(int zoom);
      
    Map* const map = nullptr;
    wxBitmap tiles;
    TileWidget* tileWidget;
    bool mouseIsDown = false;
    int scale;
    wxPoint mousePos;
    bool mouseIsIn = false;
    
    DECLARE_DYNAMIC_CLASS(MapeditWidget);
    DECLARE_EVENT_TABLE();
};
    
#endif
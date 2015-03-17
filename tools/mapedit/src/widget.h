#ifndef WIDGET_H
#define WIDGET_H

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "map.h"
#include "tile_widget.h"
#include <list>
#include <memory>
#include <utility>
#include <set>

class MapeditFrame;

enum EditMode {
  EditTiles,
  EditEntities
};

class MapeditWidget : public wxScrolledWindow {
  public:
    MapeditWidget();
    MapeditWidget(wxWindow* parent, wxWindowID winid, Map* map, TileWidget* tileWidget, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
    
    void ZoomIn();
    void ZoomOut();
    void SetEditMode(EditMode editMode);
    void StartAddingEntity(MapObject* object);
    void CancelAddingEntity();
    void SetMap(Map* map);
    void SetIsSettingStart(bool isSetting);
    
    MapeditFrame* frame;
    
  protected:
    void Init();
    virtual wxSize DoGetBestSize() const;
    void OnPaint(wxPaintEvent& event);
    void OnClick(wxMouseEvent& event);
    void OnRightClick(wxMouseEvent& event);
    void OnMouseMove(wxMouseEvent& event);
    void OnMouseUp(wxMouseEvent& event);
    void OnMouseOut(wxMouseEvent& event);
    
  private:
    void SetTile(wxPoint pos);
    void SetZoomSize(int zoom);
      
    Map* map = nullptr;
    wxBitmap tiles;
    TileWidget* tileWidget;
    bool mouseIsDown = false;
    int scale;
    wxPoint mousePos;
    bool mouseIsIn = false;
    EditMode editMode = EditTiles;
    int currentPlayer = 0;
    bool isSettingPos = false;
    std::set<std::pair<int,int>> changeBuffer;
    
    MapObject* addingEntity = nullptr;
    MapObjectEntry* movingEntity = nullptr;
    std::shared_ptr<MapObjectEntry> selectedEntity;
    
    DECLARE_DYNAMIC_CLASS(MapeditWidget)
    DECLARE_EVENT_TABLE()
};
    
#endif

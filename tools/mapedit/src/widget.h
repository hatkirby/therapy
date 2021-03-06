#ifndef WIDGET_H
#define WIDGET_H

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <list>
#include <memory>
#include <utility>
#include <set>

class MapeditFrame;
class TileWidget;
class Map;
class MapObject;
class MapObjectEntry;

#include "consts.h"

enum EditMode {
  EditTiles,
  EditEntities
};

class MapeditWidget : public wxScrolledCanvas {
  public:
    MapeditWidget();
    MapeditWidget(wxWindow* parent, wxWindowID winid, Map* map, TileWidget* tileWidget, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
    
    void SetUpZoom(int zoom);
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
    void OnScroll(wxScrollWinEvent& event);
    
  private:
    void SetTile(wxPoint pos);
    void SetZoomSize(int zoom);
    void RenderMap(Map* toRender, wxPaintDC& dc, wxMemoryDC& tiles_dc, int offset_x = EDITOR_SPACING_X, int offset_y = EDITOR_SPACING_Y, bool main = true);
      
    Map* map = nullptr;
    wxBitmap tiles;
    TileWidget* tileWidget;
    bool mouseIsDown = false;
    int scale = 1;
    wxPoint mousePos {GAME_WIDTH/2, GAME_HEIGHT/2};
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

// sends when an entity is selected OR deselected.
// client data will be a pointer to the MapObjectEntry if selection.
// client data will be nullptr if deselection.
wxDECLARE_EVENT(EVT_MAP_SELECTED_ENTITY, wxCommandEvent);
    
#endif

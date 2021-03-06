#include "widget.h"
#include "frame.h"
#include "map.h"
#include "tile_widget.h"
#include "object.h"
#include "undo.h"

IMPLEMENT_DYNAMIC_CLASS(MapeditWidget,wxScrolledCanvas)

BEGIN_EVENT_TABLE(MapeditWidget, wxScrolledCanvas)
	EVT_PAINT(MapeditWidget::OnPaint)
  EVT_LEFT_DOWN(MapeditWidget::OnClick)
  EVT_RIGHT_DOWN(MapeditWidget::OnRightClick)
  EVT_LEFT_UP(MapeditWidget::OnMouseUp)
  EVT_MOTION(MapeditWidget::OnMouseMove)
  EVT_LEAVE_WINDOW(MapeditWidget::OnMouseOut)
  EVT_SCROLLWIN(MapeditWidget::OnScroll)
END_EVENT_TABLE()
    
wxDEFINE_EVENT(EVT_MAP_SELECTED_ENTITY, wxCommandEvent);

MapeditWidget::MapeditWidget()
{
  Init();
}

MapeditWidget::MapeditWidget(wxWindow* parent, wxWindowID winid, Map* map, TileWidget* tileWidget, const wxPoint& pos, const wxSize& size)
  : wxScrolledCanvas(parent, winid, pos, size), map(map), tileWidget(tileWidget)
{
  Init();
}

void MapeditWidget::Init()
{
  tiles = wxBitmap(wxImage("res/tiles.png"));
  
  this->FitInside();
  this->SetScrollRate(5, 5);
}

void MapeditWidget::SetUpZoom(int zoom)
{
  int cW, cH, vXX, vXY;
  GetClientSize(&cW, &cH);
  GetScrollPixelsPerUnit(&vXX, &vXY);
  
  mousePos.x = cW / 2;
  mousePos.y = cH / 2;
  
  scale = zoom;
  SetVirtualSize(EDITOR_WIDTH * scale, EDITOR_HEIGHT * scale);
  Scroll((EDITOR_WIDTH*scale-cW)/2/vXX, (EDITOR_HEIGHT*scale-cH)/2/vXY);
}

wxSize MapeditWidget::DoGetBestSize() const
{
  return {EDITOR_WIDTH*scale, EDITOR_HEIGHT*scale};
}

void MapeditWidget::OnPaint(wxPaintEvent&)
{
  wxPaintDC dc(this);
  wxMemoryDC tiles_dc;
  tiles_dc.SelectObject(tiles);
  
  int vX, vY, vXX, vXY, vW, vH, cW, cH;
  GetViewStart(&vX, &vY);
  GetVirtualSize(&vW, &vH);
  GetClientSize(&cW, &cH);
  GetScrollPixelsPerUnit(&vXX, &vXY);
  vX *= vXX;
  vY *= vXY;
  
  dc.SetPen(*wxGREY_PEN);
  dc.SetBrush(*wxGREY_BRUSH);
  dc.DrawRectangle(0, 0, cW, cH);

  RenderMap(map, dc, tiles_dc);
  
  if (map->getAdjacent(Map::MoveDir::Left).type == Map::MoveType::Warp)
  {
    auto tomap = map->getWorld()->getMap(map->getAdjacent(Map::MoveDir::Left).map);
    
    RenderMap(tomap.get(), dc, tiles_dc, -EDITOR_SPACING_X, EDITOR_SPACING_Y, false);
  }
  
  if (map->getAdjacent(Map::MoveDir::Right).type == Map::MoveType::Warp)
  {
    auto tomap = map->getWorld()->getMap(map->getAdjacent(Map::MoveDir::Right).map);
    
    RenderMap(tomap.get(), dc, tiles_dc, EDITOR_WIDTH-EDITOR_SPACING_X, EDITOR_SPACING_Y, false);
  }
  
  if (map->getAdjacent(Map::MoveDir::Up).type == Map::MoveType::Warp)
  {
    auto tomap = map->getWorld()->getMap(map->getAdjacent(Map::MoveDir::Up).map);
    
    RenderMap(tomap.get(), dc, tiles_dc, EDITOR_SPACING_X, -EDITOR_SPACING_Y, false);
  }
  
  if (map->getAdjacent(Map::MoveDir::Down).type == Map::MoveType::Warp)
  {
    auto tomap = map->getWorld()->getMap(map->getAdjacent(Map::MoveDir::Down).map);
    
    RenderMap(tomap.get(), dc, tiles_dc, EDITOR_SPACING_X, EDITOR_HEIGHT-EDITOR_SPACING_Y, false);
  }
  
  for (auto object : map->getObjects())
  {
    tiles_dc.SelectObject(wxNullBitmap);
    
    wxBitmap sprite = object->getObject().getSprite();
    tiles_dc.SelectObject(sprite);
    
    wxPoint pos {(object->getPosition().first + EDITOR_SPACING_X)*scale-vX, (object->getPosition().second + EDITOR_SPACING_Y)*scale-vY};
    wxSize size {object->getObject().getWidth()*scale, object->getObject().getHeight()*scale};
    dc.StretchBlit(pos.x, pos.y, size.GetWidth(), size.GetHeight(), &tiles_dc, 0, 0, object->getObject().getWidth(), object->getObject().getHeight());
    
    if (editMode == EditEntities)
    {
      if (!selectedEntity)
      {
        wxPen pen(*wxYELLOW, 2);
        dc.SetPen(pen);
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        dc.DrawRectangle(pos.x, pos.y, size.GetWidth(), size.GetHeight());
      } else if (object == selectedEntity)
      {
        wxPen pen(*wxGREEN, 2);
        dc.SetPen(pen);
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        dc.DrawRectangle(pos.x, pos.y, size.GetWidth(), size.GetHeight());
      }
    }
  }
  
  if (map->getWorld()->getStartingMap()->getID() == map->getID())
  {
    wxBitmap sprite = wxImage("res/Starla.png");
    tiles_dc.SelectObject(wxNullBitmap);
    tiles_dc.SelectObject(sprite);
    
    std::pair<int, int> startPos = map->getWorld()->getStartingPosition();
    
    wxPoint pos {(startPos.first + EDITOR_SPACING_X)*scale-vX, (startPos.second + EDITOR_SPACING_Y)*scale-vY};
    wxSize size {PLAYER_WIDTH[currentPlayer]*scale, PLAYER_HEIGHT[currentPlayer]*scale};
    
    dc.StretchBlit(pos.x, pos.y, size.GetWidth(), size.GetHeight(), &tiles_dc, 0, 0, PLAYER_WIDTH[currentPlayer], PLAYER_HEIGHT[currentPlayer]);
  }
  
  if (mouseIsIn)
  {
    if (isSettingPos)
    {
      wxBitmap sprite = wxImage("res/Starla.png");
      tiles_dc.SelectObject(wxNullBitmap);
      tiles_dc.SelectObject(sprite);

      wxPoint pos {mousePos.x - PLAYER_WIDTH[currentPlayer]/2*scale, mousePos.y - PLAYER_HEIGHT[currentPlayer]/2*scale};
      wxSize size {PLAYER_WIDTH[currentPlayer]*scale, PLAYER_HEIGHT[currentPlayer]*scale};

      dc.StretchBlit(pos.x, pos.y, size.GetWidth(), size.GetHeight(), &tiles_dc, 0, 0, PLAYER_WIDTH[currentPlayer], PLAYER_HEIGHT[currentPlayer]);

      wxPen pen(*wxGREEN, 2);
      dc.SetPen(pen);
      dc.SetBrush(*wxTRANSPARENT_BRUSH);
      dc.DrawRectangle(pos.x, pos.y, size.GetWidth(), size.GetHeight());
    } else if (editMode == EditTiles)
    {
      int tile = tileWidget->getSelected();
      int x = (mousePos.x + vX - EDITOR_SPACING_X*scale) / (TILE_WIDTH * scale);
      int y = (mousePos.y + vY - EDITOR_SPACING_Y*scale) / (TILE_HEIGHT * scale);
    
      wxPoint pos {(x*TILE_WIDTH + EDITOR_SPACING_X)*scale-vX, (y*TILE_HEIGHT + EDITOR_SPACING_Y)*scale-vY};
      wxSize size {TILE_WIDTH*scale, TILE_HEIGHT*scale};
  
      tiles_dc.SelectObject(wxNullBitmap);
      tiles_dc.SelectObject(tiles);
      dc.StretchBlit(pos.x, pos.y, size.GetWidth(), size.GetHeight(), &tiles_dc, tile%8*TILE_WIDTH, tile/8*TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT);
  
      wxPen pen(*wxGREEN, 2);
      dc.SetPen(pen);
      dc.SetBrush(*wxTRANSPARENT_BRUSH);
      dc.DrawRectangle(pos.x, pos.y, size.GetWidth(), size.GetHeight());
    } else if ((editMode == EditEntities) && (addingEntity != nullptr))
    {
      wxBitmap sprite = addingEntity->getSprite();
      tiles_dc.SelectObject(wxNullBitmap);
      tiles_dc.SelectObject(sprite);
    
      wxPoint pos {mousePos.x - addingEntity->getWidth()/2*scale, mousePos.y - addingEntity->getHeight()/2*scale};
      wxSize size {addingEntity->getWidth()*scale, addingEntity->getHeight()*scale};
    
      dc.StretchBlit(pos.x, pos.y, size.GetWidth(), size.GetHeight(), &tiles_dc, 0, 0, addingEntity->getWidth(), addingEntity->getHeight());
    
      wxPen pen(*wxGREEN, 2);
      dc.SetPen(pen);
      dc.SetBrush(*wxTRANSPARENT_BRUSH);
      dc.DrawRectangle(pos.x, pos.y, size.GetWidth(), size.GetHeight());
    } else if ((editMode == EditEntities) && (movingEntity != nullptr))
    {
      wxBitmap sprite = movingEntity->getObject().getSprite();
      tiles_dc.SelectObject(wxNullBitmap);
      tiles_dc.SelectObject(sprite);
    
      wxPoint pos {mousePos.x - movingEntity->getObject().getWidth()/2*scale, mousePos.y - movingEntity->getObject().getHeight()/2*scale};
      wxSize size {movingEntity->getObject().getWidth()*scale, movingEntity->getObject().getHeight()*scale};
    
      dc.StretchBlit(pos.x, pos.y, size.GetWidth(), size.GetHeight(), &tiles_dc, 0, 0, movingEntity->getObject().getWidth(), movingEntity->getObject().getHeight());
    
      wxPen pen(*wxGREEN, 2);
      dc.SetPen(pen);
      dc.SetBrush(*wxTRANSPARENT_BRUSH);
      dc.DrawRectangle(pos.x, pos.y, size.GetWidth(), size.GetHeight());
    }
  }
}

void MapeditWidget::SetTile(wxPoint pos)
{
  int vX, vY;
  GetViewStart(&vX, &vY);
  int vXX, vYX;
  GetScrollPixelsPerUnit(&vXX, &vYX);
  vX *= vXX;
  vY *= vYX;

  int x = (pos.x + vX - EDITOR_SPACING_X*scale) / (TILE_WIDTH * scale);
  int y = (pos.y + vY - EDITOR_SPACING_Y*scale) / (TILE_HEIGHT * scale);
  
  changeBuffer.insert({x,y});
  
  Refresh();
}

void MapeditWidget::OnClick(wxMouseEvent& event)
{
  if (!mouseIsIn) return;
  
  mouseIsDown = true;
  
  int vX, vY;
  GetViewStart(&vX, &vY);
  int vXX, vYX;
  GetScrollPixelsPerUnit(&vXX, &vYX);
  vX *= vXX;
  vY *= vYX;
  
  if (editMode == EditTiles)
  {
    SetTile(event.GetPosition());
  } else if (editMode == EditEntities)
  {
    if (addingEntity != nullptr)
    {
      int x = (event.GetPosition().x + vX - EDITOR_SPACING_X*scale) / scale - (addingEntity->getWidth() / 2);
      int y = (event.GetPosition().y + vY - EDITOR_SPACING_Y*scale) / scale - (addingEntity->getHeight() / 2);
      
      auto data = std::make_shared<MapObjectEntry>(
        *addingEntity,
        x,
        y,
        map->getAndIncrementNextObjectIndex());
        
      frame->commitAction(std::make_shared<Undoable>("Add " + addingEntity->getName(), [=] () {
        map->addObject(data);
        
        Refresh();
      }, [=] () {
        map->removeObject(data);
        
        if (data == selectedEntity)
        {
          selectedEntity.reset();
          
          wxCommandEvent sev {EVT_MAP_SELECTED_ENTITY};
          sev.SetClientData(0);
          ProcessEvent(sev);
        }
        
        Refresh();
      }));
      
      frame->SetIsAddingEntity(false);
      addingEntity = nullptr;
    } else if (movingEntity != nullptr)
    {
      int x = (event.GetPosition().x + vX - EDITOR_SPACING_X*scale) / scale - (movingEntity->getObject().getWidth() / 2);
      int y = (event.GetPosition().y + vY - EDITOR_SPACING_Y*scale) / scale - (movingEntity->getObject().getHeight() / 2);
      auto oldPos = movingEntity->getPosition();
      MapObjectEntry* me = movingEntity;
      
      frame->commitAction(std::make_shared<Undoable>("Move " + movingEntity->getObject().getName(), [=] () {
        me->setPosition(x, y);
        
        Refresh();
      }, [=] () {
        me->setPosition(oldPos.first, oldPos.second);
        
        Refresh();
      }));
      
      frame->SetIsAddingEntity(false);
      movingEntity = nullptr;
    } else {
      int x = (event.GetPosition().x + vX - EDITOR_SPACING_X*scale) / scale;
      int y = (event.GetPosition().y + vY - EDITOR_SPACING_Y*scale) / scale;
      
      if (selectedEntity)
      {
        if ((x > selectedEntity->getPosition().first)
          && (x < selectedEntity->getPosition().first + selectedEntity->getObject().getWidth())
          && (y > selectedEntity->getPosition().second)
          && (y < selectedEntity->getPosition().second + selectedEntity->getObject().getHeight()))
        {
          movingEntity = selectedEntity.get();
          frame->SetIsAddingEntity(true);
        } else {
          selectedEntity.reset();
          
          wxCommandEvent sev {EVT_MAP_SELECTED_ENTITY};
          sev.SetClientData(0);
          ProcessEvent(sev);
        }
        
        Refresh();
      } else {
        for (auto object : map->getObjects())
        {
          if ((x >= object->getPosition().first) && (x <= object->getPosition().first + object->getObject().getWidth())
            && (y >= object->getPosition().second) && (y <= object->getPosition().second + object->getObject().getHeight()))
          {
            selectedEntity = object;
            
            wxCommandEvent sev {EVT_MAP_SELECTED_ENTITY};
            sev.SetClientData(object.get());
            ProcessEvent(sev);
            
            Refresh();
            
            break;
          }
        }
      }
    }
  }
  
  if (isSettingPos)
  {
    int x = (event.GetPosition().x + vX - EDITOR_SPACING_X*scale) / scale - (PLAYER_WIDTH[currentPlayer] / 2);
    int y = (event.GetPosition().y + vY - EDITOR_SPACING_Y*scale) / scale - (PLAYER_HEIGHT[currentPlayer] / 2);
    auto oldPos = map->getWorld()->getStartingPosition();
    auto oldSMap = map->getWorld()->getStartingMap();
    
    frame->commitAction(std::make_shared<Undoable>("Set Starting Position", [=] () {
      map->getWorld()->setStart(map, {x, y});
      frame->SetStartposLabel();
      
      Refresh();
    }, [=] () {
      map->getWorld()->setStart(oldSMap, oldPos);
      frame->SetStartposLabel();
      
      Refresh();
    }));
    
    isSettingPos = false;
    frame->SetIsSettingStart(false);
  }
  
  event.Skip();
}

void MapeditWidget::OnRightClick(wxMouseEvent& event)
{
  if (editMode == EditEntities)
  {
    if (selectedEntity)
    {
      int vX, vY;
      GetViewStart(&vX, &vY);
      int vXX, vYX;
      GetScrollPixelsPerUnit(&vXX, &vYX);
      vX *= vXX;
      vY *= vYX;
      
      int x = (event.GetPosition().x + vX - EDITOR_SPACING_X*scale) / scale;
      int y = (event.GetPosition().y + vY - EDITOR_SPACING_Y*scale) / scale;
      
      if ((x > selectedEntity->getPosition().first)
        && (x < selectedEntity->getPosition().first + selectedEntity->getObject().getWidth())
        && (y > selectedEntity->getPosition().second)
        && (y < selectedEntity->getPosition().second + selectedEntity->getObject().getHeight()))
      {
        map->removeObject(selectedEntity);
        selectedEntity.reset();
        
        wxCommandEvent sev {EVT_MAP_SELECTED_ENTITY};
        sev.SetClientData(0);
        ProcessEvent(sev);
        
        Refresh();
      }
    }
  }
}

void MapeditWidget::OnMouseMove(wxMouseEvent& event)
{
  mousePos = event.GetPosition();
  
  int vX, vY, vW, vH;
  GetViewStart(&vX, &vY);
  GetVirtualSize(&vW, &vH);
  int vXX, vYX;
  GetScrollPixelsPerUnit(&vXX, &vYX);
  vX *= vXX;
  vY *= vYX;
  
  if ((mousePos.x+vX >= EDITOR_SPACING_X*scale) && (mousePos.x+vX < (EDITOR_WIDTH-EDITOR_SPACING_X)*scale)
    && (mousePos.y+vY >= EDITOR_SPACING_Y*scale) && (mousePos.y+vY < (EDITOR_HEIGHT-EDITOR_SPACING_Y)*scale))
  {
    mouseIsIn = true;
  
    if (editMode == EditTiles)
    {
      if (mouseIsDown)
      {
        SetTile(event.GetPosition());
      }
    }
  } else {
    mouseIsIn = false;
  }
  
  Refresh();
}

void MapeditWidget::OnMouseUp(wxMouseEvent&)
{
  mouseIsDown = false;
  
  if (editMode == EditTiles)
  {
    std::map<std::pair<int, int>, int> localChangeBuffer;
    for (auto assign : changeBuffer)
    {
      localChangeBuffer[assign] = map->getTileAt(assign.first, assign.second);
    }
    
    int localSelection = tileWidget->getSelected();
    frame->commitAction(std::make_shared<Undoable>("Paint Map", [=] () {
      for (auto assign : localChangeBuffer)
      {
        map->setTileAt(assign.first.first, assign.first.second, localSelection);
      }
      
      Refresh();
    }, [=] () {
      for (auto assign : localChangeBuffer)
      {
        map->setTileAt(assign.first.first, assign.first.second, assign.second);
      }
      
      Refresh();
    }));
    
    changeBuffer.clear();
  }
}

void MapeditWidget::OnMouseOut(wxMouseEvent&)
{
  mouseIsIn = false;

  Refresh();
}

void MapeditWidget::ZoomIn()
{
  SetZoomSize(scale+1);
}

void MapeditWidget::ZoomOut()
{
  if (scale > 1)
  {
    SetZoomSize(scale-1);
  }
}

void MapeditWidget::SetZoomSize(int zoom)
{
  int vX, vY, vXX, vXY;
  GetViewStart(&vX, &vY);
  GetScrollPixelsPerUnit(&vXX, &vXY);
  vX *= vXX;
  vY *= vXY;
  
  int newViewStartX = (vX + mousePos.x) / scale * zoom - mousePos.x;
  int newViewStartY = (vY + mousePos.y) / scale * zoom - mousePos.y;

  SetVirtualSize(EDITOR_WIDTH * zoom, EDITOR_HEIGHT * zoom);
  Scroll(newViewStartX / vXX, newViewStartY / vXY);
  
  scale = zoom;
  
  Refresh();
}

void MapeditWidget::SetEditMode(EditMode editMode)
{
  this->editMode = editMode;
  
  Refresh();
}

void MapeditWidget::StartAddingEntity(MapObject* object)
{
  addingEntity = object;
  
  // Turn everything else off
  selectedEntity = nullptr;
  isSettingPos = false;
  frame->SetIsSettingStart(false);
  
  wxCommandEvent sev {EVT_MAP_SELECTED_ENTITY};
  sev.SetClientData(0);
  ProcessEvent(sev);
}

void MapeditWidget::CancelAddingEntity()
{
  addingEntity = nullptr;
  movingEntity = nullptr;
}

void MapeditWidget::SetIsSettingStart(bool isSetting)
{
  if (isSetting)
  {
    isSettingPos = true;
    
    frame->SetIsAddingEntity(false);
    addingEntity = nullptr;
    selectedEntity = nullptr;
    
    wxCommandEvent sev {EVT_MAP_SELECTED_ENTITY};
    sev.SetClientData(0);
    ProcessEvent(sev);
  } else {
    isSettingPos = false;
  }
}

void MapeditWidget::SetMap(Map* map)
{
  this->map = map;
  selectedEntity = nullptr;
  addingEntity = nullptr;
  movingEntity = nullptr;
  isSettingPos = false;
  
  wxCommandEvent sev {EVT_MAP_SELECTED_ENTITY};
  sev.SetClientData(0);
  ProcessEvent(sev);
  
  Refresh();
}

void MapeditWidget::RenderMap(Map* toRender, wxPaintDC& dc, wxMemoryDC& tiles_dc, int offset_x, int offset_y, bool main)
{
  int vX, vY, vXX, vXY, vW, vH, cW, cH;
  GetViewStart(&vX, &vY);
  GetVirtualSize(&vW, &vH);
  GetClientSize(&cW, &cH);
  GetScrollPixelsPerUnit(&vXX, &vXY);
  vX *= vXX;
  vY *= vXY;
  
  if (main)
  {
    dc.SetPen(*wxGREY_PEN);
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.DrawRectangle(offset_x, offset_y, MAP_WIDTH*TILE_WIDTH*scale, MAP_HEIGHT*TILE_HEIGHT*scale);
  }
  
  for (int y=0; y<MAP_HEIGHT; y++)
  {
    for (int x=0; x<MAP_WIDTH; x++)
    {
      int tile = toRender->getTileAt(x, y);
      
      if (main)
      {
        if (changeBuffer.find({x,y}) != end(changeBuffer))
        {
          tile = tileWidget->getSelected();
        }
      }
      
      wxPoint pos {(x*TILE_WIDTH + offset_x)*scale - vX, (y*TILE_HEIGHT + offset_y) * scale - vY};
      
      wxRasterOperationMode mod = wxCOPY;
      if (!main) mod = wxOR;
      dc.StretchBlit(pos.x, pos.y, TILE_WIDTH*scale, TILE_HEIGHT*scale, &tiles_dc, tile%8*TILE_WIDTH, tile/8*TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT, mod);
    }
  }
}

void MapeditWidget::OnScroll(wxScrollWinEvent&)
{
  int vX, vY, vW, vH;
  GetViewStart(&vX, &vY);
  GetVirtualSize(&vW, &vH);
  int vXX, vYX;
  GetScrollPixelsPerUnit(&vXX, &vYX);
  vX *= vXX;
  vY *= vYX;
  
  if ((mousePos.x+vX >= EDITOR_SPACING_X*scale) && (mousePos.x+vX < (EDITOR_WIDTH-EDITOR_SPACING_X)*scale)
    && (mousePos.y+vY >= EDITOR_SPACING_Y*scale) && (mousePos.y+vY < (EDITOR_HEIGHT-EDITOR_SPACING_Y)*scale))
  {
    mouseIsIn = true;
  } else {
    mouseIsIn = false;
  }
  
  Refresh();
}

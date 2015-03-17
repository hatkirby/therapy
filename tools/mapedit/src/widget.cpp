#include "widget.h"
#include "frame.h"

IMPLEMENT_DYNAMIC_CLASS(MapeditWidget,wxScrolledWindow)

BEGIN_EVENT_TABLE(MapeditWidget, wxScrolledWindow)
	EVT_PAINT(MapeditWidget::OnPaint)
  EVT_LEFT_DOWN(MapeditWidget::OnClick)
  EVT_RIGHT_DOWN(MapeditWidget::OnRightClick)
  EVT_LEFT_UP(MapeditWidget::OnMouseUp)
  EVT_MOTION(MapeditWidget::OnMouseMove)
  EVT_LEAVE_WINDOW(MapeditWidget::OnMouseOut)
END_EVENT_TABLE()

MapeditWidget::MapeditWidget()
{
  Init();
}

MapeditWidget::MapeditWidget(wxWindow* parent, wxWindowID winid, Map* map, TileWidget* tileWidget, const wxPoint& pos, const wxSize& size)
  : wxScrolledWindow(parent, winid, pos, size), map(map), tileWidget(tileWidget)
{
  Init();
}

void MapeditWidget::Init()
{
  tiles = wxBitmap(wxImage("res/tiles.png"));
  
  this->FitInside();
  this->SetScrollRate(5, 5);
  
  SetZoomSize(2);
}

wxSize MapeditWidget::DoGetBestSize() const
{
  return {GAME_WIDTH*2, GAME_HEIGHT*2};
}

void MapeditWidget::OnPaint(wxPaintEvent&)
{
  wxPaintDC dc(this);
  wxMemoryDC tiles_dc;
  tiles_dc.SelectObject(tiles);
  int vX, vY;
  GetViewStart(&vX, &vY);
  int vXX, vYX;
  GetScrollPixelsPerUnit(&vXX, &vYX);
  vX *= vXX;
  vY *= vYX;

  for (int y=0; y<MAP_HEIGHT; y++)
  {
    for (int x=0; x<MAP_WIDTH; x++)
    {
      int tile = map->getTileAt(x, y);
      if (changeBuffer.find({x,y}) != end(changeBuffer))
      {
        tile = tileWidget->getSelected();
      }
      
      dc.StretchBlit(x*TILE_WIDTH*scale-vX, y*TILE_HEIGHT*scale-vY, TILE_WIDTH*scale, TILE_HEIGHT*scale, &tiles_dc, tile%8*TILE_WIDTH, tile/8*TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT);
    }
  }
  
  for (auto object : map->getObjects())
  {
    tiles_dc.SelectObject(wxNullBitmap);
    
    wxBitmap sprite = object->object->getSprite();
    tiles_dc.SelectObject(sprite);
    
    wxPoint pos {(int) object->position.first*scale-vX, (int) object->position.second*scale-vY};
    wxSize size {object->object->getWidth()*scale, object->object->getHeight()*scale};
    dc.StretchBlit(pos.x, pos.y, size.GetWidth(), size.GetHeight(), &tiles_dc, 0, 0, object->object->getWidth(), object->object->getHeight());
    
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
    
    std::pair<double, double> startPos = map->getWorld()->getStartingPosition();
    
    wxPoint pos {(int) startPos.first*scale-vX, (int) startPos.second*scale-vY};
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
      int x = (mousePos.x + vX) / (TILE_WIDTH * scale);
      int y = (mousePos.y + vY) / (TILE_HEIGHT * scale);
    
      wxPoint pos {x*TILE_WIDTH*scale-vX, y*TILE_HEIGHT*scale-vY};
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
      wxBitmap sprite = movingEntity->object->getSprite();
      tiles_dc.SelectObject(wxNullBitmap);
      tiles_dc.SelectObject(sprite);
    
      wxPoint pos {mousePos.x - movingEntity->object->getWidth()/2*scale, mousePos.y - movingEntity->object->getHeight()/2*scale};
      wxSize size {movingEntity->object->getWidth()*scale, movingEntity->object->getHeight()*scale};
    
      dc.StretchBlit(pos.x, pos.y, size.GetWidth(), size.GetHeight(), &tiles_dc, 0, 0, movingEntity->object->getWidth(), movingEntity->object->getHeight());
    
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

  int x = (pos.x + vX) / (TILE_WIDTH * scale);
  int y = (pos.y + vY) / (TILE_HEIGHT * scale);
  
  changeBuffer.insert({x,y});
  
  Refresh();
}

void MapeditWidget::OnClick(wxMouseEvent& event)
{
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
      int x = (event.GetPosition().x + vX) / scale - (addingEntity->getWidth() / 2);
      int y = (event.GetPosition().y + vY) / scale - (addingEntity->getHeight() / 2);
      
      auto data = std::make_shared<MapObjectEntry>();
      data->object = addingEntity;
      data->position = std::make_pair(x,y);
      
      frame->commitAction(std::make_shared<Undoable>("Add " + addingEntity->getType(), [=] () {
        map->addObject(data);
        
        Refresh();
      }, [=] () {
        map->removeObject(data);
        
        Refresh();
      }));
      
      frame->SetIsAddingEntity(false);
      addingEntity = nullptr;
    } else if (movingEntity != nullptr)
    {
      int x = (event.GetPosition().x + vX) / scale - (movingEntity->object->getWidth() / 2);
      int y = (event.GetPosition().y + vY) / scale - (movingEntity->object->getHeight() / 2);
      auto oldPos = movingEntity->position;
      MapObjectEntry* me = movingEntity;
      
      frame->commitAction(std::make_shared<Undoable>("Move " + movingEntity->object->getType(), [=] () {
        me->position = std::make_pair(x,y);
        
        Refresh();
      }, [=] () {
        me->position = oldPos;
        
        Refresh();
      }));
      
      frame->SetIsAddingEntity(false);
      movingEntity = nullptr;
    } else {
      int x = (event.GetPosition().x + vX) / scale;
      int y = (event.GetPosition().y + vY) / scale;
      
      if (selectedEntity)
      {
        if ((x > selectedEntity->position.first) && (x < selectedEntity->position.first + selectedEntity->object->getWidth())
          && (y > selectedEntity->position.second) && (y < selectedEntity->position.second + selectedEntity->object->getHeight()))
        {
          movingEntity = selectedEntity.get();
          frame->SetIsAddingEntity(true);
        } else {
          selectedEntity.reset();
        }
        
        Refresh();
      } else {
        for (auto object : map->getObjects())
        {
          if ((x >= object->position.first) && (x <= object->position.first + object->object->getWidth())
            && (y >= object->position.second) && (y <= object->position.second + object->object->getHeight()))
          {
            selectedEntity = object;
            
            Refresh();
            
            break;
          }
        }
      }
    }
  }
  
  if (isSettingPos)
  {
    int x = (event.GetPosition().x + vX) / scale - (PLAYER_WIDTH[currentPlayer] / 2);
    int y = (event.GetPosition().y + vY) / scale - (PLAYER_HEIGHT[currentPlayer] / 2);
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
      
      int x = (event.GetPosition().x + vX) / scale;
      int y = (event.GetPosition().y + vY) / scale;
      
      if ((x > selectedEntity->position.first) && (x < selectedEntity->position.first + selectedEntity->object->getWidth())
        && (y > selectedEntity->position.second) && (y < selectedEntity->position.second + selectedEntity->object->getHeight()))
      {
        map->removeObject(selectedEntity);
        selectedEntity.reset();
        
        Refresh();
      }
    }
  }
}

void MapeditWidget::OnMouseMove(wxMouseEvent& event)
{
  mousePos = event.GetPosition();
  mouseIsIn = true;
  
  if (editMode == EditTiles)
  {
    if (mouseIsDown)
    {
      SetTile(event.GetPosition());
    }
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
  scale = zoom;
  
  SetVirtualSize(MAP_WIDTH*TILE_WIDTH*scale, MAP_HEIGHT*TILE_HEIGHT*scale);
  
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
  
  Refresh();
}

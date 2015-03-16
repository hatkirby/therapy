#include "widget.h"
#include "frame.h"

IMPLEMENT_DYNAMIC_CLASS(MapeditWidget,wxScrolledWindow)

BEGIN_EVENT_TABLE(MapeditWidget, wxScrolledWindow)
	EVT_PAINT(MapeditWidget::OnPaint)
  EVT_LEFT_DOWN(MapeditWidget::OnClick)
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
  tiles = wxBitmap(wxImage("../../../res/tiles.png"));
  
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
      dc.StretchBlit(x*TILE_WIDTH*scale-vX, y*TILE_HEIGHT*scale-vY, TILE_WIDTH*scale, TILE_HEIGHT*scale, &tiles_dc, tile%8*TILE_WIDTH, tile/8*TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT);
    }
  }
  
  for (auto object : map->getObjects())
  {
    tiles_dc.SelectObject(wxNullBitmap);
    
    wxBitmap sprite = object.object->getSprite();
    tiles_dc.SelectObject(sprite);
    
    dc.StretchBlit(object.position.first*scale-vX, object.position.second*scale-vY, object.object->getWidth()*scale, object.object->getHeight()*scale, &tiles_dc, 0, 0, object.object->getWidth(), object.object->getHeight());
  }
  
  if (editMode == EditTiles)
  {
    if (mouseIsIn)
    {
      int tile = tileWidget->getSelected();
      int x = (mousePos.x + vX) / (TILE_WIDTH * scale);
      int y = (mousePos.y + vY) / (TILE_HEIGHT * scale);
    
      tiles_dc.SelectObject(wxNullBitmap);
      tiles_dc.SelectObject(tiles);
      dc.StretchBlit(x*TILE_WIDTH*scale-vX, y*TILE_HEIGHT*scale-vY, TILE_WIDTH*scale, TILE_HEIGHT*scale, &tiles_dc, tile%8*TILE_WIDTH, tile/8*TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT);
    
      wxPen pen(*wxGREEN, 2);
      dc.SetPen(pen);
      dc.SetBrush(*wxTRANSPARENT_BRUSH);
      dc.DrawRectangle(x*TILE_WIDTH*scale-vX, y*TILE_HEIGHT*scale-vY, TILE_WIDTH*scale, TILE_HEIGHT*scale);
    }
  } else if (editMode == EditEntities)
  {
    if ((addingEntity != nullptr) && (mouseIsIn))
    {
      wxBitmap sprite = addingEntity->getSprite();
      tiles_dc.SelectObject(wxNullBitmap);
      tiles_dc.SelectObject(sprite);
      
      dc.StretchBlit(mousePos.x - addingEntity->getWidth()/2*scale, mousePos.y - addingEntity->getHeight()/2*scale, addingEntity->getWidth()*scale, addingEntity->getHeight()*scale, &tiles_dc, 0, 0, addingEntity->getWidth(), addingEntity->getHeight());
      
      wxPen pen(*wxGREEN, 2);
      dc.SetPen(pen);
      dc.SetBrush(*wxTRANSPARENT_BRUSH);
      dc.DrawRectangle(mousePos.x - addingEntity->getWidth()/2*scale, mousePos.y - addingEntity->getHeight()/2*scale, addingEntity->getWidth()*scale, addingEntity->getHeight()*scale);
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
  
  map->setTileAt(x, y, tileWidget->getSelected());
  Refresh();
}

void MapeditWidget::OnClick(wxMouseEvent& event)
{
  mouseIsDown = true;
  
  if (editMode == EditTiles)
  {
    SetTile(event.GetPosition());
  } else if (editMode == EditEntities)
  {
    if (addingEntity != nullptr)
    {
      int vX, vY;
      GetViewStart(&vX, &vY);
      int vXX, vYX;
      GetScrollPixelsPerUnit(&vXX, &vYX);
      vX *= vXX;
      vY *= vYX;
      
      int x = (event.GetPosition().x + vX) / scale - (addingEntity->getWidth() / 2);
      int y = (event.GetPosition().y + vY) / scale - (addingEntity->getHeight() / 2);

      MapObjectEntry data;
      data.object = addingEntity;
      data.position = std::make_pair(x,y);
      map->getObjects().push_back(data);
      
      addingEntity = nullptr;
      
      frame->FinishAddingEntity();
      
      Refresh();
    }
  }
  
  event.Skip();
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
}

void MapeditWidget::CancelAddingEntity()
{
  addingEntity = nullptr;
}

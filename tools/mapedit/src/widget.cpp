#include "widget.h"

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

void MapeditWidget::OnPaint(wxPaintEvent& event)
{
  wxPaintDC dc(this);
  wxMemoryDC tiles_dc;
  tiles_dc.SelectObject(tiles);
  int vX, vY;
  GetViewStart(&vX, &vY);

  for (int y=0; y<MAP_HEIGHT; y++)
  {
    for (int x=0; x<MAP_WIDTH; x++)
    {
      int tile = map->getTileAt(x, y);
      dc.StretchBlit(x*TILE_WIDTH*scale-vX, y*TILE_HEIGHT*scale-vY, TILE_WIDTH*scale, TILE_HEIGHT*scale, &tiles_dc, tile%8*TILE_WIDTH, tile/8*TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT);
    }
  }
  
  if (mouseIsIn)
  {
    int tile = tileWidget->getSelected();
    int x = (mousePos.x + vX) / (TILE_WIDTH * scale);
    int y = (mousePos.y + vY) / (TILE_HEIGHT * scale);
    
    dc.StretchBlit(x*TILE_WIDTH*scale-vX, y*TILE_HEIGHT*scale-vY, TILE_WIDTH*scale, TILE_HEIGHT*scale, &tiles_dc, tile%8*TILE_WIDTH, tile/8*TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT);
    
    wxPen pen(*wxGREEN, 2);
    dc.SetPen(pen);
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.DrawRectangle(x*TILE_WIDTH*scale-vX, y*TILE_HEIGHT*scale-vY, TILE_WIDTH*scale, TILE_HEIGHT*scale);
  }
}

void MapeditWidget::SetTile(wxPoint pos)
{
  int vX, vY;
  GetViewStart(&vX, &vY);

  int x = (pos.x + vX) / (TILE_WIDTH * scale);
  int y = (pos.y + vY) / (TILE_HEIGHT * scale);
  
  map->setTileAt(x, y, tileWidget->getSelected());
  Refresh();
}

void MapeditWidget::OnClick(wxMouseEvent& event)
{
  mouseIsDown = true;
  
  SetTile(event.GetPosition());
  
  event.Skip();
}

void MapeditWidget::OnMouseMove(wxMouseEvent& event)
{
  if (mouseIsDown)
  {
    SetTile(event.GetPosition());
  }
  
  mouseIsIn = true;
  mousePos = event.GetPosition();
  Refresh();
}

void MapeditWidget::OnMouseUp(wxMouseEvent& event)
{
  mouseIsDown = false;
}

void MapeditWidget::OnMouseOut(wxMouseEvent& event)
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
  SetZoomSize(scale-1);
}

void MapeditWidget::SetZoomSize(int zoom)
{
  scale = zoom;
  
  SetVirtualSize(MAP_WIDTH*TILE_WIDTH*scale, MAP_HEIGHT*TILE_HEIGHT*scale);
  
  GetParent()->Refresh();
}

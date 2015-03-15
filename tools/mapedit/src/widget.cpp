#include "widget.h"

IMPLEMENT_DYNAMIC_CLASS(MapeditWidget,wxControl)

BEGIN_EVENT_TABLE(MapeditWidget, wxControl)
	EVT_PAINT(MapeditWidget::OnPaint)
  EVT_LEFT_DOWN(MapeditWidget::OnClick)
  EVT_LEFT_UP(MapeditWidget::OnMouseUp)
  EVT_MOTION(MapeditWidget::OnMouseMove)
END_EVENT_TABLE()

MapeditWidget::MapeditWidget()
{
  Init();
}

MapeditWidget::MapeditWidget(wxWindow* parent, wxWindowID winid, Map* map, TileWidget* tileWidget, const wxPoint& pos, const wxSize& size)
  : wxControl(parent, winid, pos, size), map(map), tileWidget(tileWidget)
{
  Init();
}

void MapeditWidget::Init()
{
  tiles = wxBitmap(wxImage("../../../res/tiles.png"));
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
  wxRegionIterator upd(GetUpdateRegion());
  int vW = upd.GetW();
  int vH = upd.GetH();

  double endWidth = TILE_WIDTH * ((double) vW / (double) GAME_WIDTH);
  double endHeight = TILE_HEIGHT * ((double) vH / (double) GAME_HEIGHT);
  
  for (int y=0; y<MAP_HEIGHT; y++)
  {
    for (int x=0; x<MAP_WIDTH; x++)
    {
      int tile = map->mapdata[x+y*MAP_WIDTH];
      dc.StretchBlit(x*endWidth, y*endHeight, endWidth, endHeight, &tiles_dc, tile%8*TILE_WIDTH, tile/8*TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT);
    }
  }
}

void MapeditWidget::SetTile(wxPoint pos)
{
  wxRegionIterator upd(GetUpdateRegion());
  int vW = upd.GetW();
  int vH = upd.GetH();

  double endWidth = TILE_WIDTH * ((double) vW / (double) GAME_WIDTH);
  double endHeight = TILE_HEIGHT * ((double) vH / (double) GAME_HEIGHT);
  
  int x = pos.x / endWidth;
  int y = pos.y / endHeight;
  
  map->mapdata[x+y*MAP_WIDTH] = tileWidget->getSelected();
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
}

void MapeditWidget::OnMouseUp(wxMouseEvent& event)
{
  mouseIsDown = false;
}

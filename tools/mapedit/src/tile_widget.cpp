#include "tile_widget.h"
#include "map.h"

IMPLEMENT_DYNAMIC_CLASS(TileWidget,wxControl)

BEGIN_EVENT_TABLE(TileWidget, wxControl)
	EVT_PAINT(TileWidget::OnPaint)
  EVT_LEFT_DOWN(TileWidget::OnClick)
END_EVENT_TABLE()

TileWidget::TileWidget()
{
  Init();
}

TileWidget::TileWidget(wxWindow* parent, wxWindowID winid, int numTilesWidth, const wxPoint& pos, const wxSize& size)
  : wxControl(parent, winid, pos, size), numTilesWidth(numTilesWidth)
{
  Init();
}

int TileWidget::getSelected()
{
  return selected;
}

void TileWidget::Init()
{
  tiles = wxBitmap(wxImage("../../../res/tiles.png"));
  numTiles = 50;
}

void TileWidget::OnPaint(wxPaintEvent& event)
{
  wxPaintDC dc(this);
  wxMemoryDC tiles_dc;
  tiles_dc.SelectObject(tiles);
  wxRegionIterator upd(GetUpdateRegion());
  int vW = upd.GetW();
  int vH = upd.GetH();
  
  wxPen pen(*wxGREEN, 2);
  dc.SetPen(pen);
  dc.SetBrush(*wxTRANSPARENT_BRUSH);

  double endWidth = (double) vW / (double) numTilesWidth;
  double endHeight = TILE_HEIGHT * (endWidth / (double) TILE_WIDTH);
  
  for (int i=0; i<numTiles; i++)
  {
    dc.StretchBlit(i%numTilesWidth*endWidth, i/numTilesWidth*endHeight, endWidth, endHeight, &tiles_dc, i%8*TILE_WIDTH, i/8*TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT);
    
    if (i == selected)
    {
      dc.DrawRectangle(i%numTilesWidth*endWidth, i/numTilesWidth*endHeight, endWidth, endHeight);
    }
  }
}

void TileWidget::OnClick(wxMouseEvent& event)
{
  wxRegionIterator upd(GetUpdateRegion());
  int vW = upd.GetW();
  int vH = upd.GetH();
  double endWidth = (double) vW / (double) numTilesWidth;
  double endHeight = TILE_HEIGHT * (endWidth / (double) TILE_WIDTH);
  
  wxPoint pos = event.GetPosition();
  int x = pos.x / endWidth;
  int y = pos.y / endHeight;
  
  selected = x+y*numTilesWidth;
  
  Refresh();
  event.Skip();
}

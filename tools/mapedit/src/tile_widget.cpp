#include "tile_widget.h"
#include "map.h"

IMPLEMENT_DYNAMIC_CLASS(TileWidget,wxScrolledCanvas)

BEGIN_EVENT_TABLE(TileWidget, wxScrolledCanvas)
	EVT_PAINT(TileWidget::OnPaint)
  EVT_LEFT_DOWN(TileWidget::OnClick)
END_EVENT_TABLE()

TileWidget::TileWidget()
{
  Init();
}

TileWidget::TileWidget(wxWindow* parent, wxWindowID winid, int width, int scale, const wxPoint& pos, const wxSize& size)
  : wxScrolledCanvas(parent, winid, pos, size), numTilesWidth(width), scale(scale)
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
  
  this->FitInside();
  this->SetScrollRate(5, 5);
  
  SetVirtualSize(numTilesWidth*TILE_WIDTH*scale, (numTiles / numTilesWidth + 1) * TILE_HEIGHT*scale);
}

void TileWidget::OnPaint(wxPaintEvent& event)
{
  wxPaintDC dc(this);
  wxMemoryDC tiles_dc;
  tiles_dc.SelectObject(tiles);
  
  int vX, vY, vW, vH, aW, aH;
  GetViewStart(&vX, &vY);
  GetVirtualSize(&vW, &vH);
  GetSize(&aW, &aH);

  for (int i=0; i<numTiles; i++)
  {
    dc.StretchBlit(i%numTilesWidth*TILE_WIDTH*scale-vX, i/numTilesWidth*TILE_HEIGHT*scale-vY, TILE_WIDTH*scale, TILE_HEIGHT*scale, &tiles_dc, i%8*TILE_WIDTH, i/8*TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT);
    
    if (i == selected)
    {
      wxPen pen(*wxGREEN, 2);
      dc.SetPen(pen);
      dc.SetBrush(*wxTRANSPARENT_BRUSH);
      dc.DrawRectangle(i%numTilesWidth*TILE_WIDTH*scale - vX, i/numTilesWidth*TILE_HEIGHT*scale - vY, TILE_WIDTH*scale, TILE_HEIGHT*scale);
    }
  }
}

void TileWidget::OnClick(wxMouseEvent& event)
{
  int vX, vY, vW, vH;
  GetViewStart(&vX, &vY);
  GetVirtualSize(&vW, &vH);
  
  wxPoint pos = event.GetPosition();
  int x = (pos.x + vX) / (TILE_WIDTH * scale);
  int y = (pos.y + vY) / (TILE_HEIGHT * scale);
  
  selected = x+y*numTilesWidth;
  
  Refresh();
  event.Skip();
}

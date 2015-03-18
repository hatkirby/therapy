#include "map_render.h"
#include "map.h"
#include "game.h"
#include "consts.h"

MapRenderComponent::MapRenderComponent(const Map& map) : screen(GAME_WIDTH, GAME_HEIGHT)
{
  screen.fill(screen.entirety(), 0, 0, 0);
  
  Texture tiles("res/tiles.png");
  
  for (int i=0; i<MAP_WIDTH*MAP_HEIGHT; i++)
  {
    int tile = map.getMapdata()[i];
    int x = i % MAP_WIDTH;
    int y = i / MAP_WIDTH;
    Rectangle dst {x*TILE_WIDTH, y*TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT};
    Rectangle src {tile%8*TILE_WIDTH, tile/8*TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT};

    if (tile > 0)
    {
      screen.blit(tiles, src, dst);
    }
  }
  
  Texture font("res/font.bmp");
  std::string map_name = map.getTitle();
  int start_x = (40/2) - (map_name.length()/2);
  for (size_t i=0; i<map_name.length(); i++)
  {
    Rectangle srcRect {map_name[i] % 16 * 8, map_name[i] / 16 * 8, 8, 8};
    Rectangle dstRect {(start_x + (int)i)*8, 24*8, 8, 8};
    screen.blit(font, srcRect, dstRect);
  }
}

void MapRenderComponent::render(Game&, Entity&, Texture& buffer)
{
  buffer.blit(screen, screen.entirety(), buffer.entirety());
}

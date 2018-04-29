#include "mapping.h"
#include "components/mappable.h"
#include "components/realizable.h"
#include "systems/realizing.h"
#include "game.h"
#include "consts.h"

template <typename Storage>
inline void addBoundary(
  Storage& boundaries,
  int axis,
  int lower,
  int upper,
  MappableComponent::Boundary::Type type)
{
  boundaries.emplace(std::piecewise_construct,
    std::tie(axis),
    std::tie(axis, lower, upper, type));
}

void MappingSystem::render(Texture& texture)
{
  auto& realizable = game_.getEntityManager().
    getComponent<RealizableComponent>(
      game_.getSystemManager().getSystem<RealizingSystem>().getSingleton());

  id_type map = realizable.activeMap;

  auto& mappable = game_.getEntityManager().
    getComponent<MappableComponent>(map);

  for (int i = 0; i < MAP_WIDTH * MAP_HEIGHT; i++)
  {
    int x = i % MAP_WIDTH;
    int y = i / MAP_WIDTH;
    int tile = mappable.tiles[i];

    if (tile > 0)
    {
      Rectangle dst {
        x * TILE_WIDTH,
        y * TILE_HEIGHT,
        TILE_WIDTH,
        TILE_HEIGHT};

      Rectangle src {
        (tile % TILESET_COLS) * TILE_WIDTH,
        (tile / TILESET_COLS) * TILE_HEIGHT,
        TILE_WIDTH,
        TILE_HEIGHT};

      game_.getRenderer().blit(
        mappable.tileset,
        texture,
        std::move(src),
        std::move(dst));
    }
  }

  int startX = ((GAME_WIDTH / TILE_WIDTH) / 2) - (mappable.title.size() / 2);

  for (size_t i = 0; i < mappable.title.size(); i++)
  {
    Rectangle src {
      (mappable.title[i] % FONT_COLS) * TILE_WIDTH,
      (mappable.title[i] / FONT_COLS) * TILE_HEIGHT,
      TILE_WIDTH,
      TILE_HEIGHT};

    Rectangle dst {
      (startX + static_cast<int>(i)) * TILE_WIDTH,
      24 * TILE_HEIGHT,
      TILE_WIDTH,
      TILE_HEIGHT};

    game_.getRenderer().blit(
      mappable.font,
      texture,
      std::move(src),
      std::move(dst));
  }
}

void MappingSystem::generateBoundaries(id_type mapEntity)
{
  auto& mappable = game_.getEntityManager().
    getComponent<MappableComponent>(mapEntity);

  addBoundary(
    mappable.leftBoundaries,
    -WALL_GAP,
    0,
    MAP_HEIGHT * TILE_HEIGHT,
    MappableComponent::Boundary::Type::adjacency);

  addBoundary(
    mappable.rightBoundaries,
    GAME_WIDTH + WALL_GAP,
    0,
    MAP_HEIGHT * TILE_HEIGHT,
    MappableComponent::Boundary::Type::adjacency);

  addBoundary(
    mappable.upBoundaries,
    -WALL_GAP,
    0,
    GAME_WIDTH,
    MappableComponent::Boundary::Type::adjacency);

  addBoundary(
    mappable.downBoundaries,
    MAP_HEIGHT * TILE_HEIGHT + WALL_GAP,
    0,
    GAME_WIDTH,
    MappableComponent::Boundary::Type::adjacency);

  for (size_t i = 0; i < MAP_WIDTH * MAP_HEIGHT; i++)
  {
    size_t x = i % MAP_WIDTH;
    size_t y = i / MAP_WIDTH;
    int tile = mappable.tiles[i];

    if ((tile >= 5) && (tile <= 7))
    {
      addBoundary(
        mappable.downBoundaries,
        y * TILE_HEIGHT,
        x * TILE_WIDTH,
        (x + 1) * TILE_WIDTH,
        MappableComponent::Boundary::Type::platform);
    } else if ((tile > 0) && (tile < 28))
    {
      addBoundary(
        mappable.rightBoundaries,
        x * TILE_WIDTH,
        y * TILE_HEIGHT,
        (y+1) * TILE_HEIGHT,
        MappableComponent::Boundary::Type::wall);

      addBoundary(
        mappable.leftBoundaries,
        (x+1) * TILE_WIDTH,
        y * TILE_HEIGHT,
        (y+1) * TILE_HEIGHT,
        MappableComponent::Boundary::Type::wall);

      addBoundary(
        mappable.downBoundaries,
        y * TILE_HEIGHT,
        x * TILE_WIDTH,
        (x+1) * TILE_WIDTH,
        MappableComponent::Boundary::Type::wall);

      addBoundary(
        mappable.upBoundaries,
        (y+1) * TILE_HEIGHT,
        x * TILE_WIDTH,
        (x+1) * TILE_WIDTH,
        MappableComponent::Boundary::Type::wall);
    } else if (tile == 42)
    {
      addBoundary(
        mappable.rightBoundaries,
        x * TILE_WIDTH,
        y * TILE_HEIGHT,
        (y+1) * TILE_HEIGHT,
        MappableComponent::Boundary::Type::danger);

      addBoundary(
        mappable.leftBoundaries,
        (x+1) * TILE_WIDTH,
        y * TILE_HEIGHT,
        (y+1) * TILE_HEIGHT,
        MappableComponent::Boundary::Type::danger);

      addBoundary(
        mappable.downBoundaries,
        y * TILE_HEIGHT + 1,
        x * TILE_WIDTH,
        (x+1) * TILE_WIDTH,
        MappableComponent::Boundary::Type::danger);

      addBoundary(
        mappable.upBoundaries,
        (y+1) * TILE_HEIGHT,
        x * TILE_WIDTH,
        (x+1) * TILE_WIDTH,
        MappableComponent::Boundary::Type::danger);
    }
  }
}

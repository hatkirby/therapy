#include "mapping.h"
#include "components/mappable.h"
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
  auto entities = game_.getEntityManager().getEntitiesWithComponents<
    MappableComponent>();

  for (id_type entity : entities)
  {
    auto& mappable = game_.getEntityManager().
      getComponent<MappableComponent>(entity);

    const Map& map = game_.getWorld().getMap(mappable.getMapId());

    for (int i = 0; i < MAP_WIDTH * MAP_HEIGHT; i++)
    {
      int x = i % MAP_WIDTH;
      int y = i / MAP_WIDTH;
      int tile = map.getTiles()[i];

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
          mappable.getTileset(),
          texture,
          std::move(src),
          std::move(dst));
      }
    }

    int startX = ((GAME_WIDTH / TILE_WIDTH) / 2) - (map.getTitle().size() / 2);
    for (size_t i = 0; i < map.getTitle().size(); i++)
    {
      Rectangle src {
        (map.getTitle()[i] % FONT_COLS) * TILE_WIDTH,
        (map.getTitle()[i] / FONT_COLS) * TILE_HEIGHT,
        TILE_WIDTH,
        TILE_HEIGHT};

      Rectangle dst {
        (startX + static_cast<int>(i)) * TILE_WIDTH,
        24 * TILE_HEIGHT,
        TILE_WIDTH,
        TILE_HEIGHT};

      game_.getRenderer().blit(
        mappable.getFont(),
        texture,
        std::move(src),
        std::move(dst));
    }
  }
}

void MappingSystem::loadMap(size_t mapId)
{
  id_type mapEntity = game_.getEntityManager().emplaceEntity();

  auto& mappable = game_.getEntityManager().
    emplaceComponent<MappableComponent>(mapEntity,
      Texture("res/tiles.png"),
      Texture("res/font.bmp"));

  mappable.setMapId(mapId);

  const Map& map = game_.getWorld().getMap(mappable.getMapId());

  addBoundary(
    mappable.getLeftBoundaries(),
    -WALL_GAP,
    0,
    MAP_HEIGHT * TILE_HEIGHT,
    MappableComponent::Boundary::Type::adjacency);

  addBoundary(
    mappable.getRightBoundaries(),
    GAME_WIDTH + WALL_GAP,
    0,
    MAP_HEIGHT * TILE_HEIGHT,
    MappableComponent::Boundary::Type::adjacency);

  addBoundary(
    mappable.getUpBoundaries(),
    -WALL_GAP,
    0,
    GAME_WIDTH,
    MappableComponent::Boundary::Type::adjacency);

  addBoundary(
    mappable.getDownBoundaries(),
    MAP_HEIGHT * TILE_HEIGHT + WALL_GAP,
    0,
    GAME_WIDTH,
    MappableComponent::Boundary::Type::adjacency);

  for (size_t i = 0; i < MAP_WIDTH * MAP_HEIGHT; i++)
  {
    size_t x = i % MAP_WIDTH;
    size_t y = i / MAP_WIDTH;
    int tile = map.getTiles()[i];

    if ((tile >= 5) && (tile <= 7))
    {
      addBoundary(
        mappable.getDownBoundaries(),
        y * TILE_HEIGHT,
        x * TILE_WIDTH,
        (x + 1) * TILE_WIDTH,
        MappableComponent::Boundary::Type::platform);
    } else if ((tile > 0) && (tile < 28))
    {
      addBoundary(
        mappable.getRightBoundaries(),
        x * TILE_WIDTH,
        y * TILE_HEIGHT,
        (y+1) * TILE_HEIGHT,
        MappableComponent::Boundary::Type::wall);

      addBoundary(
        mappable.getLeftBoundaries(),
        (x+1) * TILE_WIDTH,
        y * TILE_HEIGHT,
        (y+1) * TILE_HEIGHT,
        MappableComponent::Boundary::Type::wall);

      addBoundary(
        mappable.getDownBoundaries(),
        y * TILE_HEIGHT,
        x * TILE_WIDTH,
        (x+1) * TILE_WIDTH,
        MappableComponent::Boundary::Type::wall);

      addBoundary(
        mappable.getUpBoundaries(),
        (y+1) * TILE_HEIGHT,
        x * TILE_WIDTH,
        (x+1) * TILE_WIDTH,
        MappableComponent::Boundary::Type::wall);
    } else if (tile == 42)
    {
      addBoundary(
        mappable.getRightBoundaries(),
        x * TILE_WIDTH,
        y * TILE_HEIGHT,
        (y+1) * TILE_HEIGHT,
        MappableComponent::Boundary::Type::danger);

      addBoundary(
        mappable.getLeftBoundaries(),
        (x+1) * TILE_WIDTH,
        y * TILE_HEIGHT,
        (y+1) * TILE_HEIGHT,
        MappableComponent::Boundary::Type::danger);

      addBoundary(
        mappable.getDownBoundaries(),
        y * TILE_HEIGHT,
        x * TILE_WIDTH,
        (x+1) * TILE_WIDTH,
        MappableComponent::Boundary::Type::danger);

      addBoundary(
        mappable.getUpBoundaries(),
        (y+1) * TILE_HEIGHT,
        x * TILE_WIDTH,
        (x+1) * TILE_WIDTH,
        MappableComponent::Boundary::Type::danger);
    }
  }
}

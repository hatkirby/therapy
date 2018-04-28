#include "realizing.h"
#include <stdexcept>
#include <libxml/parser.h>
#include <cstring>
#include "game.h"
#include "consts.h"
#include "components/realizable.h"
#include "components/mappable.h"
#include "components/animatable.h"
#include "components/playable.h"
#include "components/ponderable.h"
#include "components/transformable.h"
#include "systems/mapping.h"
#include "systems/animating.h"
#include "systems/pondering.h"

inline xmlChar* getProp(xmlNodePtr node, const char* attr)
{
  xmlChar* key = xmlGetProp(node, reinterpret_cast<const xmlChar*>(attr));
  if (key == nullptr)
  {
    throw std::invalid_argument("Error parsing world file");
  }

  return key;
}

// TODO: neither the XML doc nor any of the emplaced entities are properly
// destroyed if this method throws an exception.
EntityManager::id_type RealizingSystem::initSingleton(std::string filename)
{
  id_type world = game_.getEntityManager().emplaceEntity();

  auto& realizable = game_.getEntityManager().
    emplaceComponent<RealizableComponent>(world);

  auto& mapping = game_.getSystemManager().getSystem<MappingSystem>();

  xmlDocPtr doc = xmlParseFile(filename.c_str());
  if (doc == nullptr)
  {
    throw std::invalid_argument("Cannot find world file");
  }

  xmlNodePtr top = xmlDocGetRootElement(doc);
  if (top == nullptr)
  {
    throw std::invalid_argument("Error parsing world file");
  }

  if (xmlStrcmp(top->name, reinterpret_cast<const xmlChar*>("world")))
  {
    throw std::invalid_argument("Error parsing world file");
  }

  xmlChar* key = nullptr;

  key = getProp(top, "startx");
  realizable.startingX = atoi(reinterpret_cast<char*>(key));
  xmlFree(key);

  key = getProp(top, "starty");
  realizable.startingY = atoi(reinterpret_cast<char*>(key));
  xmlFree(key);

  key = getProp(top, "startmap");
  realizable.startingMapId = atoi(reinterpret_cast<char*>(key));
  xmlFree(key);

  for (xmlNodePtr node = top->xmlChildrenNode;
    node != nullptr;
    node = node->next)
  {
    if (!xmlStrcmp(node->name, reinterpret_cast<const xmlChar*>("map")))
    {
      id_type map = game_.getEntityManager().emplaceEntity();

      auto& mappable = game_.getEntityManager().
        emplaceComponent<MappableComponent>(map,
          Texture("res/tiles.png"),
          Texture("res/font.bmp"));

      key = getProp(node, "id");
      mappable.mapId = atoi(reinterpret_cast<char*>(key));
      xmlFree(key);

      key = getProp(node, "title");
      mappable.title = reinterpret_cast<char*>(key);
      xmlFree(key);

      for (xmlNodePtr mapNode = node->xmlChildrenNode;
        mapNode != nullptr;
        mapNode = mapNode->next)
      {
        if (!xmlStrcmp(
          mapNode->name,
          reinterpret_cast<const xmlChar*>("environment")))
        {
          key = xmlNodeGetContent(mapNode);
          if (key == nullptr)
          {
            throw std::invalid_argument("Error parsing world file");
          }

          mappable.tiles.clear();
          mappable.tiles.push_back(atoi(strtok(
            reinterpret_cast<char*>(key),
            ",\n")));

          for (size_t i = 1; i < (MAP_WIDTH * MAP_HEIGHT); i++)
          {
            mappable.tiles.push_back(atoi(strtok(nullptr, ",\n")));
          }

          xmlFree(key);
        } else if (!xmlStrcmp(
          mapNode->name,
          reinterpret_cast<const xmlChar*>("adjacent")))
        {
          key = getProp(mapNode, "type");
          std::string adjTypeStr(reinterpret_cast<char*>(key));
          xmlFree(key);

          MappableComponent::Adjacent::Type adjType;
          if (adjTypeStr == "wall")
          {
            adjType = MappableComponent::Adjacent::Type::wall;
          } else if (adjTypeStr == "wrap")
          {
            adjType = MappableComponent::Adjacent::Type::wrap;
          } else if (adjTypeStr == "warp")
          {
            adjType = MappableComponent::Adjacent::Type::warp;
          } else if (adjTypeStr == "reverseWarp")
          {
            adjType = MappableComponent::Adjacent::Type::reverse;
          } else {
            throw std::logic_error("Invalid adjacency type");
          }

          key = getProp(mapNode, "map");
          size_t adjMapId = atoi(reinterpret_cast<char*>(key));
          xmlFree(key);

          key = getProp(mapNode, "dir");
          std::string adjDir(reinterpret_cast<char*>(key));
          xmlFree(key);

          if (adjDir == "left")
          {
            mappable.leftAdjacent = {adjType, adjMapId};
          } else if (adjDir == "right")
          {
            mappable.rightAdjacent = {adjType, adjMapId};
          } else if (adjDir == "up")
          {
            mappable.upAdjacent = {adjType, adjMapId};
          } else if (adjDir == "down")
          {
            mappable.downAdjacent = {adjType, adjMapId};
          } else {
            throw std::logic_error("Invalid adjacency direction");
          }
        }
      }

      mapping.generateBoundaries(map);

      realizable.maps.insert(map);
      realizable.entityByMapId[mappable.mapId] = map;
    }
  }

  xmlFreeDoc(doc);

  loadMap(realizable.entityByMapId[realizable.startingMapId]);

  return world;
}

EntityManager::id_type RealizingSystem::getSingleton() const
{
  std::set<id_type> result =
    game_.getEntityManager().getEntitiesWithComponents<
      RealizableComponent>();

  if (result.empty())
  {
    throw std::logic_error("No realizable entity found");
  } else if (result.size() > 1)
  {
    throw std::logic_error("Multiple realizable entities found");
  }

  return *std::begin(result);
}

void RealizingSystem::loadMap(id_type mapEntity)
{
  id_type world = getSingleton();

  auto& realizable = game_.getEntityManager().
    getComponent<RealizableComponent>(world);

  auto& animating = game_.getSystemManager().getSystem<AnimatingSystem>();
  auto& pondering = game_.getSystemManager().getSystem<PonderingSystem>();

  std::set<id_type> players =
    game_.getEntityManager().getEntitiesWithComponents<
      PlayableComponent>();

  if (realizable.hasActiveMap)
  {
    id_type oldMap = realizable.activeMap;

    auto& oldMappable = game_.getEntityManager().
      getComponent<MappableComponent>(oldMap);

    // Deactivate any map objects from the old map.
    for (id_type prototype : oldMappable.objects)
    {
      leaveActiveMap(prototype);
    }

    // Deactivate players that were on the old map.
    for (id_type player : players)
    {
      auto& playable = game_.getEntityManager().
        getComponent<PlayableComponent>(player);

      if (playable.mapId == oldMap)
      {
        leaveActiveMap(player);
      }
    }
  }

  realizable.hasActiveMap = true;
  realizable.activeMap = mapEntity;

  auto& mappable = game_.getEntityManager().
    getComponent<MappableComponent>(mapEntity);

  // Initialize the new map's objects.
  for (id_type prototype : mappable.objects)
  {
    if (game_.getEntityManager().
      hasComponent<TransformableComponent>(prototype))
    {
      auto& transformable = game_.getEntityManager().
        getComponent<TransformableComponent>(prototype);

      transformable.x = transformable.origX;
      transformable.y = transformable.origY;
      transformable.w = transformable.origW;
      transformable.h = transformable.origH;
    }

    if (game_.getEntityManager().hasComponent<AnimatableComponent>(prototype))
    {
      animating.initPrototype(prototype);
    }

    if (game_.getEntityManager().hasComponent<PonderableComponent>(prototype))
    {
      pondering.initPrototype(prototype);
    }

    enterActiveMap(prototype);
  }

  // Activate any players on the map.
  for (id_type player : players)
  {
    auto& playable = game_.getEntityManager().
      getComponent<PlayableComponent>(player);

    if (playable.mapId == mapEntity)
    {
      enterActiveMap(player);
    }
  }
}

void RealizingSystem::enterActiveMap(id_type entity)
{
  if (game_.getEntityManager().hasComponent<AnimatableComponent>(entity))
  {
    auto& animatable = game_.getEntityManager().
      getComponent<AnimatableComponent>(entity);

    animatable.active = true;
  }

  if (game_.getEntityManager().hasComponent<PonderableComponent>(entity))
  {
    auto& ponderable = game_.getEntityManager().
      getComponent<PonderableComponent>(entity);

    ponderable.active = true;
  }
}

void RealizingSystem::leaveActiveMap(id_type entity)
{
  if (game_.getEntityManager().hasComponent<AnimatableComponent>(entity))
  {
    auto& animatable = game_.getEntityManager().
      getComponent<AnimatableComponent>(entity);

    animatable.active = false;
  }

  if (game_.getEntityManager().hasComponent<PonderableComponent>(entity))
  {
    auto& ponderable = game_.getEntityManager().
      getComponent<PonderableComponent>(entity);

    ponderable.active = false;
  }
}

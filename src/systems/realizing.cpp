#include "realizing.h"
#include <stdexcept>
#include <libxml/parser.h>
#include <cstring>
#include <map>
#include "game.h"
#include "consts.h"
#include "animation.h"
#include "components/realizable.h"
#include "components/mappable.h"
#include "components/animatable.h"
#include "components/playable.h"
#include "components/ponderable.h"
#include "components/transformable.h"
#include "components/automatable.h"
#include "systems/mapping.h"
#include "systems/animating.h"
#include "systems/pondering.h"
#include "systems/automating.h"

inline xmlChar* getProp(xmlNodePtr node, const char* attr)
{
  xmlChar* key = xmlGetProp(node, reinterpret_cast<const xmlChar*>(attr));
  if (key == nullptr)
  {
    throw std::invalid_argument("Error parsing world file");
  }

  return key;
}

void parseAI(
  xmlNodePtr node,
  std::vector<AutomatableComponent::Action>& behavior,
  const std::map<std::string, int>& items)
{
  xmlChar* key = nullptr;

  if (!xmlStrcmp(
    node->name,
    reinterpret_cast<const xmlChar*>("switch")))
  {
    key = getProp(node, "item");
    std::string switchItem = reinterpret_cast<char*>(key);
    xmlFree(key);

    for (xmlNodePtr switchNode = node->xmlChildrenNode;
         switchNode != nullptr;
         switchNode = switchNode->next)
    {
      if (!xmlStrcmp(
        switchNode->name,
        reinterpret_cast<const xmlChar*>("case")))
      {
        key = getProp(switchNode, "value");
        int caseValue = atoi(reinterpret_cast<char*>(key));
        xmlFree(key);

        if (items.at(switchItem) == caseValue)
        {
          for (xmlNodePtr caseNode = switchNode->xmlChildrenNode;
               caseNode != nullptr;
               caseNode = caseNode->next)
          {
            parseAI(
              caseNode,
              behavior,
              items);
          }
        }
      }
    }
  } else if (!xmlStrcmp(
    node->name,
    reinterpret_cast<const xmlChar*>("move")))
  {
    key = getProp(node, "direction");
    std::string direction = reinterpret_cast<char*>(key);
    xmlFree(key);

    key = getProp(node, "length-var");
    std::string lengthVar = reinterpret_cast<char*>(key);
    xmlFree(key);

    key = getProp(node, "speed-var");
    std::string speedVar = reinterpret_cast<char*>(key);
    xmlFree(key);

    double length = items.at(lengthVar);
    double speed = items.at(speedVar);

    AutomatableComponent::Action action;

    if (direction == "left")
    {
      action.speed.x() = -speed;
      action.speed.y() = 0;
    } else if (direction == "right")
    {
      action.speed.x() = speed;
      action.speed.y() = 0;
    } else if (direction == "up")
    {
      action.speed.x() = 0;
      action.speed.y() = -speed;
    } else if (direction == "down")
    {
      action.speed.x() = 0;
      action.speed.y() = speed;
    }

    action.dur = length / speed;

    behavior.push_back(std::move(action));
  }
}

// TODO: neither the XML doc nor any of the emplaced entities are properly
// destroyed if this method throws an exception.
EntityManager::id_type RealizingSystem::initSingleton(
  std::string worldFile,
  std::string prototypeFile)
{
  id_type world = game_.getEntityManager().emplaceEntity();

  auto& realizable = game_.getEntityManager().
    emplaceComponent<RealizableComponent>(world);

  realizable.worldFile = worldFile;
  realizable.prototypeFile = prototypeFile;

  auto& mapping = game_.getSystemManager().getSystem<MappingSystem>();

  xmlChar* key = nullptr;

  // Create a mapping between prototype names and the XML trees defining them.
  xmlDocPtr protoXml = xmlParseFile(prototypeFile.c_str());
  if (protoXml == nullptr)
  {
    throw std::invalid_argument("Cannot find prototypes file");
  }

  xmlNodePtr protoTop = xmlDocGetRootElement(protoXml);
  if (protoTop == nullptr)
  {
    throw std::invalid_argument("Error parsing prototypes file");
  }

  if (xmlStrcmp(protoTop->name, reinterpret_cast<const xmlChar*>("entities")))
  {
    throw std::invalid_argument("Error parsing prototypes file");
  }

  std::map<std::string, xmlNodePtr> prototypes;

  for (xmlNodePtr node = protoTop->xmlChildrenNode;
       node != nullptr;
       node = node->next)
  {
    if (!xmlStrcmp(node->name, reinterpret_cast<const xmlChar*>("entity")))
    {
      key = getProp(node, "id");
      std::string prototypeId = reinterpret_cast<char*>(key);
      xmlFree(key);

      prototypes[prototypeId] = node;
    }
  }

  // Create entities from the world definition.
  xmlDocPtr doc = xmlParseFile(worldFile.c_str());
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

  key = getProp(top, "startx");
  realizable.startingPos.x() = atoi(reinterpret_cast<char*>(key));
  xmlFree(key);

  key = getProp(top, "starty");
  realizable.startingPos.y() = atoi(reinterpret_cast<char*>(key));
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
          reinterpret_cast<const xmlChar*>("entity")))
        {
          id_type mapObject = game_.getEntityManager().emplaceEntity();

          key = getProp(mapNode, "type");
          std::string prototypeId = reinterpret_cast<char*>(key);
          xmlFree(key);

          xmlNodePtr prototypeNode = prototypes[prototypeId];

          // Set the coordinates from the object definition.
          auto& transformable = game_.getEntityManager().
            emplaceComponent<TransformableComponent>(mapObject);

          key = getProp(mapNode, "x");
          transformable.origPos.x() = atoi(reinterpret_cast<char*>(key));
          xmlFree(key);

          key = getProp(mapNode, "y");
          transformable.origPos.y() = atoi(reinterpret_cast<char*>(key));
          xmlFree(key);

          // Set the sprite and size using the prototype definition.
          key = getProp(prototypeNode, "sprite");
          std::string spritePath = reinterpret_cast<char*>(key);
          xmlFree(key);

          key = getProp(prototypeNode, "width");
          transformable.origSize.w() = atoi(reinterpret_cast<char*>(key));
          xmlFree(key);

          key = getProp(prototypeNode, "height");
          transformable.origSize.h() = atoi(reinterpret_cast<char*>(key));
          xmlFree(key);

          AnimationSet objectAnim(
            spritePath.c_str(),
            transformable.origSize.w(),
            transformable.origSize.h(),
            1);

          objectAnim.emplaceAnimation("static", 0, 1, 1);

          auto& animatable = game_.getEntityManager().
            emplaceComponent<AnimatableComponent>(
              mapObject,
              std::move(objectAnim));

          animatable.origAnimation = "static";

          // Create a physics body.
          game_.getSystemManager().getSystem<PonderingSystem>().
            initializeBody(mapObject, PonderableComponent::Type::vacuumed);

          // Look for any object configuration.
          std::map<std::string, int> items;

          for (xmlNodePtr objectNode = mapNode->xmlChildrenNode;
            objectNode != nullptr;
            objectNode = objectNode->next)
          {
            if (!xmlStrcmp(
              objectNode->name,
              reinterpret_cast<const xmlChar*>("item")))
            {
              key = getProp(objectNode, "id");
              std::string itemName = reinterpret_cast<char*>(key);
              xmlFree(key);

              key = xmlNodeGetContent(objectNode);
              int itemVal = atoi(reinterpret_cast<char*>(key));
              xmlFree(key);

              items[itemName] = itemVal;
            }
          }

          // Add any AI behaviors.
          std::vector<double> behaviorWeights;

          for (xmlNodePtr protoSubNode = prototypeNode->xmlChildrenNode;
            protoSubNode != nullptr;
            protoSubNode = protoSubNode->next)
          {
            if (!xmlStrcmp(
              protoSubNode->name,
              reinterpret_cast<const xmlChar*>("ai")))
            {
              if (!game_.getEntityManager().
                hasComponent<AutomatableComponent>(mapObject))
              {
                game_.getEntityManager().
                  emplaceComponent<AutomatableComponent>(mapObject);
              }

              auto& automatable = game_.getEntityManager().
                getComponent<AutomatableComponent>(mapObject);

              key = getProp(protoSubNode, "chance");
              behaviorWeights.push_back(atof(reinterpret_cast<char*>(key)));
              xmlFree(key);

              std::vector<AutomatableComponent::Action> behavior;

              for (xmlNodePtr aiNode = protoSubNode->xmlChildrenNode;
                aiNode != nullptr;
                aiNode = aiNode->next)
              {
                parseAI(
                  aiNode,
                  behavior,
                  items);
              }

              automatable.behaviors.push_back(std::move(behavior));
            }
          }

          mappable.objects.push_back(mapObject);
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
  xmlFreeDoc(protoXml);

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
  auto& automating = game_.getSystemManager().getSystem<AutomatingSystem>();

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

      transformable.pos = transformable.origPos;
      transformable.size = transformable.origSize;
    }

    if (game_.getEntityManager().hasComponent<AnimatableComponent>(prototype))
    {
      animating.initPrototype(prototype);
    }

    if (game_.getEntityManager().hasComponent<PonderableComponent>(prototype))
    {
      pondering.initPrototype(prototype);
    }

    if (game_.getEntityManager().hasComponent<AutomatableComponent>(prototype))
    {
      automating.initPrototype(prototype);
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

  if (game_.getEntityManager().hasComponent<AutomatableComponent>(entity))
  {
    auto& automatable = game_.getEntityManager().
      getComponent<AutomatableComponent>(entity);

    automatable.active = true;
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

  if (game_.getEntityManager().hasComponent<AutomatableComponent>(entity))
  {
    auto& automatable = game_.getEntityManager().
      getComponent<AutomatableComponent>(entity);

    automatable.active = false;
  }
}

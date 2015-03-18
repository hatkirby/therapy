#include "game.h"
#include <cstdlib>
#include <libxml/parser.h>
#include "renderer.h"
#include "muxer.h"
#include "map.h"
#include "components/user_movement.h"
#include "components/player_physics.h"
#include "components/player_sprite.h"
#include "components/map_render.h"
#include "components/map_collision.h"
#include "consts.h"

Game::Game(const char* mapfile)
{ 
  // Load maps
  xmlDocPtr doc = xmlParseFile(mapfile);
  if (doc == nullptr)
  {
    exit(2);
  }
  
  xmlNodePtr top = xmlDocGetRootElement(doc);
  if (top == nullptr)
  {
    exit(2);
  }
  
  if (xmlStrcmp(top->name, (const xmlChar*) "world"))
  {
    exit(2);
  }
  
  for (xmlNodePtr node = top->xmlChildrenNode; node != NULL; node = node->next)
  {
    if (!xmlStrcmp(node->name, (const xmlChar*) "startpos"))
    {
      xmlChar* idKey = xmlGetProp(node, (xmlChar*) "id");
      if (idKey == 0) exit(2);
      startMap = atoi((char*) idKey);
      xmlFree(idKey);
      
      xmlChar* posKey = xmlGetProp(node, (xmlChar*) "pos");
      if (posKey == 0) exit(2);
      sscanf((char*) posKey, "%d,%d", &startPos.first, &startPos.second);
      xmlFree(posKey);
    } else if (!xmlStrcmp(node->name, (const xmlChar*) "map"))
    {
      xmlChar* idKey = xmlGetProp(node, (xmlChar*) "id");
      if (idKey == 0) exit(2);
      int theId = atoi((char*) idKey);
      xmlFree(idKey);
      
      Map map {theId};
      
      for (xmlNodePtr mapNode = node->xmlChildrenNode; mapNode != NULL; mapNode = mapNode->next)
      {
        if (!xmlStrcmp(mapNode->name, (const xmlChar*) "name"))
        {
          xmlChar* key = xmlNodeListGetString(doc, mapNode->xmlChildrenNode, 1);
          if (key != 0)
          {
            map.setTitle((char*) key);
          }
          
          xmlFree(key);
        } else if (!xmlStrcmp(mapNode->name, (const xmlChar*) "environment"))
        {
          xmlChar* key = xmlNodeListGetString(doc, mapNode->xmlChildrenNode, 1);
          int* mapdata = (int*) malloc(MAP_WIDTH*MAP_HEIGHT*sizeof(int));
          mapdata[0] = atoi(strtok((char*) key, ",\n"));
          for (int i=1; i<(MAP_WIDTH*MAP_HEIGHT); i++)
          {
            mapdata[i] = atoi(strtok(NULL, ",\n"));
          }
          map.setMapdata(mapdata);
          xmlFree(key);
        } else if (!xmlStrcmp(mapNode->name, (const xmlChar*) "leftmap"))
        {
          xmlChar* typeKey = xmlGetProp(mapNode, (xmlChar*) "type");
          if (typeKey == 0) exit(2);
          map.setLeftMoveType(Map::moveTypeForShort((char*) typeKey));
          xmlFree(typeKey);
          
          if (Map::moveTypeTakesMap(map.getLeftMoveType()))
          {
            xmlChar* idKey = xmlGetProp(mapNode, (xmlChar*) "map");
            if (idKey == 0) exit(2);
            map.setLeftMapID(atoi((char*) idKey));
            xmlFree(idKey);
          }
        } else if (!xmlStrcmp(mapNode->name, (const xmlChar*) "rightmap"))
        {
          xmlChar* typeKey = xmlGetProp(mapNode, (xmlChar*) "type");
          if (typeKey == 0) exit(2);
          map.setRightMoveType(Map::moveTypeForShort((char*) typeKey));
          xmlFree(typeKey);
          
          if (Map::moveTypeTakesMap(map.getRightMoveType()))
          {
            xmlChar* idKey = xmlGetProp(mapNode, (xmlChar*) "map");
            if (idKey == 0) exit(2);
            map.setRightMapID(atoi((char*) idKey));
            xmlFree(idKey);
          }
        } else if (!xmlStrcmp(mapNode->name, (const xmlChar*) "upmap"))
        {
          xmlChar* typeKey = xmlGetProp(mapNode, (xmlChar*) "type");
          if (typeKey == 0) exit(2);
          map.setUpMoveType(Map::moveTypeForShort((char*) typeKey));
          xmlFree(typeKey);
          
          if (Map::moveTypeTakesMap(map.getUpMoveType()))
          {
            xmlChar* idKey = xmlGetProp(mapNode, (xmlChar*) "map");
            if (idKey == 0) exit(2);
            map.setUpMapID(atoi((char*) idKey));
            xmlFree(idKey);
          }
        } else if (!xmlStrcmp(mapNode->name, (const xmlChar*) "downmap"))
        {
          xmlChar* typeKey = xmlGetProp(mapNode, (xmlChar*) "type");
          if (typeKey == 0) exit(2);
          map.setDownMoveType(Map::moveTypeForShort((char*) typeKey));
          xmlFree(typeKey);
          
          if (Map::moveTypeTakesMap(map.getDownMoveType()))
          {
            xmlChar* idKey = xmlGetProp(mapNode, (xmlChar*) "map");
            if (idKey == 0) exit(2);
            map.setDownMapID(atoi((char*) idKey));
            xmlFree(idKey);
          }
        } else if (!xmlStrcmp(mapNode->name, (const xmlChar*) "entities"))
        {
          for (xmlNodePtr entityNode = mapNode->xmlChildrenNode; entityNode != NULL; entityNode = entityNode->next)
          {
            if (!xmlStrcmp(entityNode->name, (const xmlChar*) "entity"))
            {
              Map::EntityData data;
          
              for (xmlNodePtr entityDataNode = entityNode->xmlChildrenNode; entityDataNode != NULL; entityDataNode = entityDataNode->next)
              {
                if (!xmlStrcmp(entityDataNode->name, (const xmlChar*) "entity-type"))
                {
                  xmlChar* key = xmlNodeListGetString(doc, entityDataNode->xmlChildrenNode, 1);
                  data.name = (char*) key;
                  xmlFree(key);
                } else if (!xmlStrcmp(entityDataNode->name, (const xmlChar*) "entity-position"))
                {
                  xmlChar* key = xmlNodeListGetString(doc, entityDataNode->xmlChildrenNode, 1);
                  sscanf((char*) key, "%d,%d", &data.position.first, &data.position.second);
                  xmlFree(key);
                }
              }
        
              map.addEntity(data);
            }
          }
        }
      }
      
      maps[theId] = map;
    }
  }
  
  xmlFreeDoc(doc);
  
  // Set up entities
  player = std::make_shared<Entity>();
  player->position = startPos;
  player->size = std::make_pair(10.0,12.0);
  
  auto player_input = std::make_shared<UserMovementComponent>();
  player->addComponent(player_input);
  
  auto player_physics = std::make_shared<PlayerPhysicsComponent>();
  player->addComponent(player_physics);
  
  auto player_anim = std::make_shared<PlayerSpriteComponent>();
  player->addComponent(player_anim);
  
  Map& startingMap = maps[startMap];
  save = {&startingMap, player->position};
  
  loadMap(startingMap, player->position);
}

void key_callback(GLFWwindow* window, int key, int, int action, int)
{
  Game* game = (Game*) glfwGetWindowUserPointer(window);
  
  if ((key == GLFW_KEY_ESCAPE) && (action == GLFW_PRESS))
  {
    game->shouldQuit = true;
  }
  
  for (auto entity : game->entities)
  {
    entity->input(*game, key, action);
  }
}

void Game::execute(GLFWwindow* window)
{
  glfwSwapInterval(1);
  glfwSetWindowUserPointer(window, this);
  glfwSetKeyCallback(window, key_callback);
  
  Texture buffer(GAME_WIDTH, GAME_HEIGHT);

  double lastTime = glfwGetTime();
  const double dt = 0.01;
  double accumulator = 0.0;
  
  while (!(shouldQuit || glfwWindowShouldClose(window)))
  {
    double currentTime = glfwGetTime();
    double frameTime = currentTime - lastTime;
    lastTime = currentTime;
    
    // Should we load a new world?
    if (newWorld)
    {
      newWorld = false;
      entities.clear();
      entities = std::move(nextEntities);
      
      player->position = nextPosition;
    }
    
    // Handle input
    glfwPollEvents();
    
    // Tick!
    accumulator += frameTime;
    while (accumulator >= dt)
    {
      for (auto entity : entities)
      {
        entity->tick(*this, dt);
      }
      
      accumulator -= dt;
    }
    
    // Do any scheduled tasks
    for (auto& task : scheduled)
    {
      task.first -= frameTime;
      
      if (task.first <= 0)
      {
        task.second();
      }
    }
    
    scheduled.remove_if([] (std::pair<double, std::function<void ()>> value) { return value.first <= 0; });
  
    // Do rendering
    buffer.fill(buffer.entirety(), 0, 0, 0);
    for (auto entity : entities)
    {
      entity->render(*this, buffer);
    }

    buffer.renderScreen();
  }
}

void Game::loadMap(const Map& map, std::pair<double, double> position)
{
  auto mapEn = std::make_shared<Entity>();
  
  auto map_render = std::make_shared<MapRenderComponent>(map);
  mapEn->addComponent(map_render);
  
  auto map_collision = std::make_shared<MapCollisionComponent>(map);
  mapEn->addComponent(map_collision);
  
  // Map in the back, player on top, rest of entities in between
  nextEntities.clear();
  nextEntities.push_back(mapEn);
  map.createEntities(nextEntities);
  nextEntities.push_back(player);
  
  newWorld = true;
  
  currentMap = &map;
  nextPosition = position;
}

void Game::detectCollision(Entity& collider, std::pair<double, double> old_position)
{
  for (auto entity : entities)
  {
    entity->detectCollision(*this, collider, old_position);
  }
}

void Game::saveGame()
{
  save = {currentMap, player->position};
}

void Game::schedule(double time, std::function<void ()> callback)
{
  scheduled.emplace_front(time, std::move(callback));
}

void Game::playerDie()
{
  player->send(*this, Message::Type::die);
  
  playSound("res/Hit_Hurt5.wav", 0.25);
  
  schedule(0.75, [&] () {
    if (*currentMap != *save.map)
    {
      loadMap(*save.map, save.position);
    } else {
      player->position = save.position;
    }
    
    player->send(*this, Message::Type::stopDying);
  });
}

const Map& Game::getMap(int id) const
{
  return maps.at(id);
}

#include "entityfactory.h"
#include <libxml/parser.h>
#include "muxer.h"
#include <cstdio>
#include <map>
#include "components/static_image.h"
#include "components/simple_collider.h"
#include "components/physics_body.h"
#include "game.h"

struct EntityData {
  char* sprite;
  char* action;
  bool hasPhysics;
  int width;
  int height;
};

static std::map<std::string, EntityData> factories;

std::shared_ptr<Entity> EntityFactory::createNamedEntity(const std::string name)
{
  auto it = factories.find(name);
  EntityData data = factories[name];
  if (it == factories.end())
  {
    xmlDocPtr doc = xmlParseFile(("../entities/" + name + ".xml").c_str());
    if (doc == nullptr)
    {
      fprintf(stderr, "Error reading entity %s\n", name.c_str());
      exit(-1);
    }
  
    xmlNodePtr top = xmlDocGetRootElement(doc);
    if (top == nullptr)
    {
      fprintf(stderr, "Empty entity %s\n", name.c_str());
      exit(-1);
    }
  
    if (xmlStrcmp(top->name, (const xmlChar*) "entity-def"))
    {
      fprintf(stderr, "Invalid entity definition %s\n", name.c_str());
      exit(-1);
    }
  
    for (xmlNodePtr node = top->xmlChildrenNode; node != NULL; node = node->next)
    {
      if (!xmlStrcmp(node->name, (const xmlChar*) "sprite"))
      {
        xmlChar* key = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
        data.sprite = (char*) calloc(xmlStrlen(key)+1, sizeof(char));
        strcpy(data.sprite, (char*) key);
        xmlFree(key);
      } else if (!xmlStrcmp(node->name, (const xmlChar*) "action"))
      {
        xmlChar* key = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
        data.action = (char*) calloc(xmlStrlen(key)+1, sizeof(char));
        strcpy(data.action, (char*) key);
        xmlFree(key);
      } else if (!xmlStrcmp(node->name, (const xmlChar*) "size"))
      {
        xmlChar* key = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
        data.hasPhysics = true;
        sscanf((char*) key, "%d,%d", &data.width, &data.height);
        xmlFree(key);
      }
    }
  
    xmlFreeDoc(doc);
    
    factories[name] = data;
  }
  
  auto entity = std::make_shared<Entity>();
  
  if (data.sprite)
  {
    auto component = std::make_shared<StaticImageComponent>(data.sprite);
    entity->addComponent(component);
  }
  
  if (data.action)
  {
    if (!strcmp(data.action, "save"))
    {
      auto component = std::make_shared<SimpleColliderComponent>([&] (Game& game, Entity&) {
        playSound("../res/Pickup_Coin23.wav", 0.25);
  
        game.saveGame();
      });
      entity->addComponent(component);
    }
  }
  
  if (data.hasPhysics)
  {
    auto component = std::make_shared<PhysicsBodyComponent>();
    entity->addComponent(component);
    
    entity->size = std::make_pair(data.width, data.height);
  }
  
  return entity;
}

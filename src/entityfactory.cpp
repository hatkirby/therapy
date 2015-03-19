#include "entityfactory.h"
#include <libxml/parser.h>
#include "muxer.h"
#include <cstdio>
#include <map>
#include <list>
#include "components/static_image.h"
#include "components/simple_collider.h"
#include "components/physics_body.h"
#include "components/ai.h"
#include "game.h"

void parseEntityAIData(AI& ai, xmlNodePtr node, const std::map<std::string, int>& items)
{
  xmlChar* key;
  
  for (xmlNodePtr aiNode = node->xmlChildrenNode; aiNode != NULL; aiNode = aiNode->next)
  {
    if (!xmlStrcmp(aiNode->name, (xmlChar*) "move"))
    {
      MoveAIAction::Direction dir;
      int len;
      int speed;
      
      key = xmlGetProp(aiNode, (xmlChar*) "direction");
      if (key == 0) exit(2);
      if (!xmlStrcmp(key, (xmlChar*) "left"))
      {
        dir = MoveAIAction::Direction::Left;
      } else if (!xmlStrcmp(key, (xmlChar*) "right"))
      {
        dir = MoveAIAction::Direction::Right;
      } else if (!xmlStrcmp(key, (xmlChar*) "up"))
      {
        dir = MoveAIAction::Direction::Up;
      } else if (!xmlStrcmp(key, (xmlChar*) "down"))
      {
        dir = MoveAIAction::Direction::Down;
      } else {
        exit(2);
      }
      xmlFree(key);
      
      key = xmlGetProp(aiNode, (xmlChar*) "length");
      if (key != 0)
      {
        len = atoi((char*) key);
      } else {
        key = xmlGetProp(aiNode, (xmlChar*) "length-var");
        if (key == 0) exit(2);
        std::string varName = (char*) key;
        len = items.at(varName);
      }
      xmlFree(key);
      
      key = xmlGetProp(aiNode, (xmlChar*) "speed");
      if (key != 0)
      {
        speed = atoi((char*) key);
      } else {
        key = xmlGetProp(aiNode, (xmlChar*) "speed-var");
        if (key == 0) exit(2);
        std::string varName = (char*) key;
        speed = items.at(varName);
      }
      xmlFree(key);
      
      ai.addAction(std::make_shared<MoveAIAction>(dir, len, speed));
    } else if (!xmlStrcmp(aiNode->name, (xmlChar*) "switch"))
    {
      key = xmlGetProp(aiNode, (xmlChar*) "item");
      if (key == 0) exit(2);
      std::string switchItem = (char*) key;
      xmlFree(key);
      
      for (xmlNodePtr switchNode = aiNode->xmlChildrenNode; switchNode != NULL; switchNode = switchNode->next)
      {
        if (!xmlStrcmp(switchNode->name, (xmlChar*) "case"))
        {
          key = xmlGetProp(switchNode, (xmlChar*) "value");
          if (key == 0) exit(2);
          int caseValue = atoi((char*) key);
          xmlFree(key);
          
          if (items.at(switchItem) == caseValue)
          {
            parseEntityAIData(ai, switchNode, items);
          }
        }
      }
    }
  }
}

std::shared_ptr<Entity> EntityFactory::createNamedEntity(const std::string name, const std::map<std::string, int>& items)
{
  xmlDocPtr doc = xmlParseFile("res/entities.xml");
  if (doc == nullptr)
  {
    fprintf(stderr, "Error reading entities\n");
    exit(-1);
  }

  xmlNodePtr top = xmlDocGetRootElement(doc);
  if (top == nullptr)
  {
    fprintf(stderr, "Empty entities file\n");
    exit(-1);
  }

  if (xmlStrcmp(top->name, (const xmlChar*) "entities"))
  {
    fprintf(stderr, "Invalid entities definition\n");
    exit(-1);
  }
  
  auto entity = std::make_shared<Entity>();

  xmlChar* key;
  for (xmlNodePtr node = top->xmlChildrenNode; node != NULL; node = node->next)
  {
    if (!xmlStrcmp(node->name, (xmlChar*) "entity"))
    {
      key = xmlGetProp(node, (xmlChar*) "id");
      if (key == 0) exit(-1);
      std::string entityID = (char*) key;
      xmlFree(key);
      
      if (entityID == name)
      {
        key = xmlGetProp(node, (xmlChar*) "sprite");
        if (key == 0) exit(-1);
        auto spriteComponent = std::make_shared<StaticImageComponent>((char*) key);
        entity->addComponent(spriteComponent);
        xmlFree(key);
        
        auto physicsComponent = std::make_shared<PhysicsBodyComponent>();
        entity->addComponent(physicsComponent);
      
        key = xmlGetProp(node, (xmlChar*) "width");
        if (key == 0) exit(-1);
        entity->size.first = atoi((char*) key);
        xmlFree(key);
      
        key = xmlGetProp(node, (xmlChar*) "height");
        if (key == 0) exit(-1);
        entity->size.second = atoi((char*) key);
        xmlFree(key);
        
        bool addAI = false;
        auto aiComponent = std::make_shared<AIComponent>();
        
        for (xmlNodePtr entityNode = node->xmlChildrenNode; entityNode != NULL; entityNode = entityNode->next)
        {
          if (!xmlStrcmp(entityNode->name, (xmlChar*) "ai"))
          {
            addAI = true;
            
            xmlChar* chanceKey = xmlGetProp(entityNode, (xmlChar*) "chance");
            if (chanceKey == 0) exit(2);
            int chance = atoi((char*) chanceKey);
            xmlFree(chanceKey);
            
            AI& ai = aiComponent->emplaceAI(chance);
            parseEntityAIData(ai, entityNode, items);
          }
        }
        
        if (addAI)
        {
          entity->addComponent(aiComponent);
        }
        
        break;
      }
    }
  }

  xmlFreeDoc(doc);
  
  return entity;
}

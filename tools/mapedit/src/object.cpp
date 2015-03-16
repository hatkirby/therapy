#include "object.h"
#include <dirent.h>
#include <libxml/parser.h>
#include <memory>

static std::map<std::string, std::shared_ptr<MapObject>> allObjects;
static bool objsInit = false;

const std::map<std::string, std::shared_ptr<MapObject>> MapObject::getAllObjects()
{
  if (!objsInit)
  {
    DIR* dir = opendir("../../../entities/");
    if (dir != NULL)
    {
      struct dirent* ent;
      while ((ent = readdir(dir)) != NULL)
      {
        std::string path = ent->d_name;
        if ((path.length() >= 4) && (path.substr(path.length() - 4, 4) == ".xml"))
        {
          std::string name = path.substr(0, path.length() - 4);
          auto obj = std::make_shared<MapObject>(name.c_str());
        
          allObjects[name] = obj;
        }
      }
    }
    
    objsInit = true;
  }
  
  return allObjects;
}

MapObject::MapObject(const char* filename)
{
  type = filename;
  
  xmlDocPtr doc = xmlParseFile(("../../../entities/" + std::string(filename) + ".xml").c_str());
  if (doc == nullptr) throw MapObjectLoadException(filename);

  xmlNodePtr top = xmlDocGetRootElement(doc);
  if (top == nullptr) throw MapObjectLoadException(filename);

  if (xmlStrcmp(top->name, (const xmlChar*) "entity-def"))
  {
    throw MapObjectLoadException(filename);
  }

  for (xmlNodePtr node = top->xmlChildrenNode; node != NULL; node = node->next)
  {
    if (!xmlStrcmp(node->name, (const xmlChar*) "sprite"))
    {
      xmlChar* key = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
      std::string spriteFile = (char*) key;
      xmlFree(key);
      
      sprite = wxImage("../../" + spriteFile);
    } else if (!xmlStrcmp(node->name, (const xmlChar*) "action"))
    {
      xmlChar* key = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
      action = (char*) key;
      xmlFree(key);
    } else if (!xmlStrcmp(node->name, (const xmlChar*) "size"))
    {
      xmlChar* key = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
      sscanf((char*) key, "%d,%d", &width, &height);
      xmlFree(key);
    }
  }

  xmlFreeDoc(doc);
}

wxBitmap MapObject::getSprite() const
{
  return sprite;
}

std::string MapObject::getAction() const
{
  return action;
}

int MapObject::getWidth() const
{
  return width;
}

int MapObject::getHeight() const
{
  return height;
}

std::string MapObject::getType() const
{
  return type;
}

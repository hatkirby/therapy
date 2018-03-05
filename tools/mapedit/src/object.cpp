#include "object.h"
#include <dirent.h>
#include <libxml/parser.h>
#include <memory>
#include "world.h"

static std::map<std::string, MapObject> allObjects;
static bool objsInit = false;

const std::map<std::string, MapObject>& MapObject::getAllObjects()
{
  if (!objsInit)
  {
    try
    {
      xmlDocPtr doc = xmlParseFile("res/entities.xml");
      if (doc == nullptr)
      {
        throw MapObjectLoadException("can't open file");
      }
  
      xmlNodePtr top = xmlDocGetRootElement(doc);
      if (top == nullptr)
      {
        throw MapObjectLoadException("missing root element");
      }
  
      if (xmlStrcmp(top->name, (const xmlChar*) "entities"))
      {
        throw MapObjectLoadException("root element is not entities");
      }

      for (xmlNodePtr node = top->xmlChildrenNode; node != NULL; node = node->next)
      {
        if (!xmlStrcmp(node->name, (const xmlChar*) "entity"))
        {
          xmlChar* idKey = xmlGetProp(node, (xmlChar*) "id");
          if (idKey == 0) throw MapObjectLoadException("entity missing id");
          std::string theID = (char*) idKey;
          xmlFree(idKey);
      
          allObjects.emplace(theID, theID);
          MapObject& mapObject = allObjects.at(theID);
      
          xmlChar* nameKey = xmlGetProp(node, (xmlChar*) "name");
          if (nameKey == 0) throw MapObjectLoadException("entity missing name");
          mapObject.name = (char*) nameKey;
          xmlFree(nameKey);
      
          xmlChar* spriteKey = xmlGetProp(node, (xmlChar*) "sprite");
          if (spriteKey == 0) throw MapObjectLoadException("entity missing sprite");
          mapObject.sprite = wxImage((char*) spriteKey);
          xmlFree(spriteKey);
      
          xmlChar* widthKey = xmlGetProp(node, (xmlChar*) "width");
          if (widthKey == 0) throw MapObjectLoadException("entity missing width");
          mapObject.width = atoi((char*) widthKey);
          xmlFree(widthKey);
      
          xmlChar* heightKey = xmlGetProp(node, (xmlChar*) "height");
          if (heightKey == 0) throw MapObjectLoadException("entity missing height");
          mapObject.height = atoi((char*) heightKey);
          xmlFree(heightKey);
      
          for (xmlNodePtr entityNode = node->xmlChildrenNode; entityNode != NULL; entityNode = entityNode->next)
          {
            if (!xmlStrcmp(entityNode->name, (const xmlChar*) "input"))
            {
              xmlChar* key = xmlGetProp(entityNode, (xmlChar*) "id");
              if (key == 0) throw MapObjectLoadException("input missing id");
              std::string inputID = (char*) key;
              xmlFree(key);
          
              Input& input = mapObject.inputs[inputID];
          
              key = xmlGetProp(entityNode, (xmlChar*) "name");
              if (key == 0) throw MapObjectLoadException("input missing name");
              input.name = (char*) key;
              xmlFree(key);
          
              key = xmlGetProp(entityNode, (xmlChar*) "type");
              if (key == 0) throw MapObjectLoadException("input missing type");
              std::string inputType = (char*) key;
              xmlFree(key);
          
              if (inputType == "choice")
              {
                input.type = Input::Type::Choice;
                
                for (xmlNodePtr choiceNode = entityNode->xmlChildrenNode; choiceNode != NULL; choiceNode = choiceNode->next)
                {
                  if (!xmlStrcmp(choiceNode->name, (xmlChar*) "value"))
                  {
                    key = xmlGetProp(choiceNode, (xmlChar*) "id");
                    if (key == 0) throw MapObjectLoadException("input value missing id");
                    int valueId = atoi((char*) key);
                    xmlFree(key);
                    
                    key = xmlNodeGetContent(choiceNode);
                    if (key == 0) throw MapObjectLoadException("input value missing content");
                    std::string choiceText = (char*) key;
                    xmlFree(key);
                    
                    input.choices[valueId] = choiceText;
                  }
                }
              } else if (inputType == "slider")
              {
                input.type = Input::Type::Slider;
            
                key = xmlGetProp(entityNode, (xmlChar*) "minvalue");
                if (key == 0) throw MapObjectLoadException("integer input missing minvalue");
                input.minvalue = atoi((char*) key);
                xmlFree(key);
            
                key = xmlGetProp(entityNode, (xmlChar*) "maxvalue");
                if (key == 0) throw MapObjectLoadException("integer input missing maxvalue");
                input.maxvalue = atoi((char*) key);
                xmlFree(key);
              }
            }
          }
        }
      }
    } catch (std::exception& ex)
    {
      wxMessageBox(ex.what(), "Error loading objects", wxOK | wxCENTRE | wxICON_ERROR);
      exit(3);
    }
    
    objsInit = true;
  }
  
  return allObjects;
}

MapObject::MapObject(std::string id) : id(id)
{
  
}

std::string MapObject::getID() const
{
  return id;
}

std::string MapObject::getName() const
{
  return name;
}

wxBitmap MapObject::getSprite() const
{
  return sprite;
}

int MapObject::getWidth() const
{
  return width;
}

int MapObject::getHeight() const
{
  return height;
}

const std::map<std::string, MapObject::Input>& MapObject::getInputs() const
{
  return inputs;
}

const MapObject::Input& MapObject::getInput(std::string id) const
{
  return inputs.at(id);
}

bool MapObject::operator==(const MapObject& other) const
{
  return id == other.id;
}

bool MapObject::operator!=(const MapObject& other) const
{
  return id != other.id;
}

MapObjectEntry::MapObjectEntry(
  const MapObject& object,
  int posx,
  int posy,
  size_t index) :
    object(object),
    position(std::make_pair(posx, posy)),
    index(index)
{
}

const MapObject& MapObjectEntry::getObject() const
{
  return object;
}

std::pair<int, int> MapObjectEntry::getPosition() const
{
  return position;
}

MapObjectEntry::Item& MapObjectEntry::getItem(std::string str)
{
  return items[str];
}

const std::map<std::string, MapObjectEntry::Item>& MapObjectEntry::getItems() const
{
  return items;
}

size_t MapObjectEntry::getIndex() const
{
  return index;
}

void MapObjectEntry::addItem(std::string id, Item& item)
{
  items[id] = item;
}

void MapObjectEntry::setPosition(int x, int y)
{
  position = std::make_pair(x, y);
}

bool MapObjectEntry::operator==(const MapObjectEntry& other) const
{
  return (object == other.object) && (position == other.position);
}

bool MapObjectEntry::operator!=(const MapObjectEntry& other) const
{
  return (object != other.object) && (position != other.position);
}

VariableChoiceValidator::VariableChoiceValidator(World& world, MapObjectEntry::Item& item) : world(world), item(item)
{
  
}

wxObject* VariableChoiceValidator::Clone() const
{
  return new VariableChoiceValidator(world, item);
}

bool VariableChoiceValidator::TransferFromWindow()
{
  wxChoice* choice = (wxChoice*) GetWindow();
  int sel = choice->GetSelection();
  int val = (intptr_t) choice->GetClientData(sel);
  item.intvalue = val;
  world.setDirty(true);
  
  return true;
}

bool VariableChoiceValidator::TransferToWindow()
{
  wxChoice* choice = (wxChoice*) GetWindow();
  for (size_t i=0; i<choice->GetCount(); i++)
  {
    if ((intptr_t) choice->GetClientData(i) == item.intvalue)
    {
      choice->SetSelection(i);
      return true;
    }
  }
  
  return false;
}

bool VariableChoiceValidator::Validate(wxWindow*)
{
  return true;
}

SliderItemValidator::SliderItemValidator(World& world, MapObjectEntry::Item& item) : world(world), item(item)
{
  
}

wxObject* SliderItemValidator::Clone() const
{
  return new SliderItemValidator(world, item);
}

bool SliderItemValidator::TransferFromWindow()
{
  wxSlider* slider = (wxSlider*) GetWindow();
  item.intvalue = slider->GetValue();
  world.setDirty(true);
  
  return true;
}

bool SliderItemValidator::TransferToWindow()
{
  wxSlider* slider = (wxSlider*) GetWindow();
  slider->SetValue(item.intvalue);
  
  return true;
}

bool SliderItemValidator::Validate(wxWindow*)
{
  return true;
}

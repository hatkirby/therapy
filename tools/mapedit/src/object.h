#ifndef OBJECT_H
#define OBJECT_H

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <string>
#include <map>

class World;

class MapObjectLoadException: public std::exception
{
  public:
    MapObjectLoadException(std::string stuff) : stuff(stuff) {}
    
    virtual const char* what() const throw()
    {
      return ("An error occured loading map objects: " + stuff).c_str();
    }
    
  private:
    std::string stuff;
};

class MapObject {
  public:
    MapObject(std::string id);
    
    static const std::map<std::string, MapObject>& getAllObjects();
    
    struct Input {
      enum class Type {
        Slider,
        Choice
      };
      
      std::string name;
      Type type;
      int minvalue;
      int maxvalue;
      std::map<int, std::string> choices;
    };
    
    std::string getID() const;
    std::string getName() const;
    wxBitmap getSprite() const;
    int getWidth() const;
    int getHeight() const;
    const std::map<std::string, Input>& getInputs() const;
    const Input& getInput(std::string id) const;
    
    bool operator==(const MapObject& other) const;
    bool operator!=(const MapObject& other) const;
    
  private:
    const std::string id;
    std::string name;
    wxBitmap sprite;
    int width;
    int height;
    std::map<std::string, Input> inputs;
};

class MapObjectEntry {
  public:
    MapObjectEntry(const MapObject& object, int posx, int posy);
    
    struct Item {
      MapObject::Input::Type type;
      int intvalue;
    };
    
    const MapObject& getObject() const;
    std::pair<int, int> getPosition() const;
    Item& getItem(std::string str);
    const std::map<std::string, Item>& getItems() const;
    
    void setPosition(int x, int y);
    void addItem(std::string id, Item& item);
    
    bool operator==(const MapObjectEntry& other) const;
    bool operator!=(const MapObjectEntry& other) const;
  
  private:
    const MapObject& object;
    std::pair<int, int> position;
    std::map<std::string, Item> items; 
};

class VariableChoiceValidator : public wxValidator {
  public:
    VariableChoiceValidator(World& world, MapObjectEntry::Item& item);
    wxObject* Clone() const;
    bool TransferFromWindow();
    bool TransferToWindow();
    bool Validate(wxWindow* parent);
    
  private:
    World& world;
    MapObjectEntry::Item& item;
};

class SliderItemValidator : public wxValidator {
  public:
    SliderItemValidator(World& world, MapObjectEntry::Item& item);
    wxObject* Clone() const;
    bool TransferFromWindow();
    bool TransferToWindow();
    bool Validate(wxWindow* parent);
    
  private:
    World& world;
    MapObjectEntry::Item& item;
};

#endif

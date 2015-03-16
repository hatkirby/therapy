#ifndef OBJECT_H
#define OBJECT_H

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <string>
#include <map>

class MapObjectLoadException: public std::exception
{
  public:
    MapObjectLoadException(std::string mapname) : mapname(mapname) {}
    
    virtual const char* what() const throw()
    {
      return ("An error occured loading map object " + mapname).c_str();
    }
    
  private:
    std::string mapname;
};

class MapObject {
  public:
    MapObject(const char* filename);
    
    static const std::map<std::string, std::shared_ptr<MapObject>> getAllObjects();
    
    std::string getType() const;
    wxBitmap getSprite() const;
    std::string getAction() const;
    int getWidth() const;
    int getHeight() const;
    
  private:
    std::string type;
    wxBitmap sprite;
    std::string action;
    int width;
    int height;
};

#endif

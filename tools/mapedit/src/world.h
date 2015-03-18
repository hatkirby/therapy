#ifndef WORLD_H
#define WORLD_H

class World;

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/treectrl.h>
#include <map>
#include <memory>
#include <list>

class MapeditFrame;
class Map;

class World {
  public:
    World();
    World(std::string filename);
    std::shared_ptr<Map> newMap();
    std::shared_ptr<Map> getMap(int id) const;
    void setDirty(bool dirty);
    bool getDirty() const;
    std::string getFilename() const;
    void setParent(MapeditFrame* parent);
    void save(std::string filename, wxTreeCtrl* mapTree);
    Map* getLastMap() const;
    void setLastMap(Map* map);
    std::list<std::shared_ptr<Map>> getRootMaps() const;
    const std::map<int, std::shared_ptr<Map>> getMaps() const;
    bool getEmpty() const;
    Map* getStartingMap() const;
    std::pair<double, double> getStartingPosition() const;
    void setStart(Map* map, std::pair<double, double> startPos); 
    
  private:
    MapeditFrame* parent;
    std::map<int, std::shared_ptr<Map>> maps;
    int nextMapID = 0;
    bool dirty = false;
    std::string filename;
    int lastmap = 0;
    std::list<int> rootChildren;
    bool empty = false;
    int startingMap = 0;
    std::pair<int, int> startingPosition {100, 100};
};

#endif

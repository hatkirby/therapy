#ifndef MAP_H
#define MAP_H

class Map {
  public:
    Map(const char* filename);
    Map(Map& map);
    Map(Map&& map);
    ~Map();
    Map& operator= (Map other);
    friend void swap(Map& first, Map& second);
    
    const int* mapdata();
    const char* title();
    Map* getLeftMap();
    Map* getRightMap();
    void setLeftMap(Map* m);
    void setRightMap(Map* m);
  private:
    Map();
    
    int* m_mapdata;
    char* m_title;
    Map* m_leftMap;
    Map* m_rightMap;
};

#endif

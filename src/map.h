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
    
    const int* mapdata() const;
    const char* title() const;
    const Map* getLeftMap() const;
    const Map* getRightMap() const;
    void setLeftMap(const Map* m);
    void setRightMap(const Map* m);
  private:
    Map();
    
    int* m_mapdata;
    char* m_title;
    const Map* m_leftMap = nullptr;
    const Map* m_rightMap = nullptr;
};

#endif

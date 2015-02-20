#ifndef MAP_H
#define MAP_H

class Map {
  public:
    Map(char* filename);
    ~Map();
    const int* mapdata();
    const char* title();
    Map* getLeftMap();
    Map* getRightMap();
    void setLeftMap(Map* m);
    void setRightMap(Map* m);
  private:
    int* m_mapdata;
    char* m_title;
    Map* m_leftMap = 0;
    Map* m_rightMap = 0;
};

#endif

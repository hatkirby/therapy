#ifndef MAP_H_74055FC0
#define MAP_H_74055FC0

#include <vector>
#include <string>
#include <list>
#include <stdexcept>
#include <map>

class Map {
public:

  class Adjacent {
  public:

    enum class Type {
      wall,
      wrap,
      warp,
      reverse
    };

    Adjacent(
      Type type = Type::wall,
      int mapId = -1) :
        type_(type),
        mapId_(mapId)
    {
    }

    inline Type getType() const
    {
      return type_;
    }

    inline int getMapId() const
    {
      return mapId_;
    }

  private:

    Type type_;
    int mapId_;
  };

  Map(
    int id,
    std::vector<int> tiles,
    std::string title,
    Adjacent leftAdjacent,
    Adjacent rightAdjacent,
    Adjacent upAdjacent,
    Adjacent downAdjacent) :
      id_(id),
      tiles_(std::move(tiles)),
      title_(std::move(title)),
      leftAdjacent_(std::move(leftAdjacent)),
      rightAdjacent_(std::move(rightAdjacent)),
      upAdjacent_(std::move(upAdjacent)),
      downAdjacent_(std::move(downAdjacent))
  {
  }

  inline size_t getId() const
  {
    return id_;
  }

  inline const std::vector<int>& getTiles() const
  {
    return tiles_;
  }

  inline const std::string& getTitle() const
  {
    return title_;
  }

  inline const Adjacent& getLeftAdjacent() const
  {
    return leftAdjacent_;
  }

  inline const Adjacent& getRightAdjacent() const
  {
    return rightAdjacent_;
  }

  inline const Adjacent& getUpAdjacent() const
  {
    return upAdjacent_;
  }

  inline const Adjacent& getDownAdjacent() const
  {
    return downAdjacent_;
  }

private:

  int id_;
  std::vector<int> tiles_;
  std::string title_;
  Adjacent leftAdjacent_;
  Adjacent rightAdjacent_;
  Adjacent upAdjacent_;
  Adjacent downAdjacent_;
};

#endif /* end of include guard: MAP_H_74055FC0 */

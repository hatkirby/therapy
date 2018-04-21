#ifndef MAP_H_74055FC0
#define MAP_H_74055FC0

#include <vector>
#include <string>
#include <list>
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

  class Object {
  public:

    Object(
      std::string type,
      double x,
      double y,
      size_t index,
      std::map<std::string, int> items) :
        type_(std::move(type)),
        x_(x),
        y_(y),
        index_(index),
        items_(std::move(items))
    {
    }

    inline const std::string& getType() const
    {
      return type_;
    }

    inline double getX() const
    {
      return x_;
    }

    inline double getY() const
    {
      return y_;
    }

    inline size_t getIndex() const
    {
      return index_;
    }

    inline const std::map<std::string, int>& getItems() const
    {
      return items_;
    }

  private:

    std::string type_;
    double x_;
    double y_;
    size_t index_;
    std::map<std::string, int> items_;
  };

  using object_storage_type = std::list<Object>;

  Map(
    int id,
    std::vector<int> tiles,
    std::string title,
    Adjacent leftAdjacent,
    Adjacent rightAdjacent,
    Adjacent upAdjacent,
    Adjacent downAdjacent,
    object_storage_type objects) :
      id_(id),
      tiles_(std::move(tiles)),
      title_(std::move(title)),
      leftAdjacent_(std::move(leftAdjacent)),
      rightAdjacent_(std::move(rightAdjacent)),
      upAdjacent_(std::move(upAdjacent)),
      downAdjacent_(std::move(downAdjacent)),
      objects_(std::move(objects))
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

  inline const object_storage_type& getObjects() const
  {
    return objects_;
  }

private:

  int id_;
  std::vector<int> tiles_;
  std::string title_;
  Adjacent leftAdjacent_;
  Adjacent rightAdjacent_;
  Adjacent upAdjacent_;
  Adjacent downAdjacent_;
  object_storage_type objects_;
};

#endif /* end of include guard: MAP_H_74055FC0 */

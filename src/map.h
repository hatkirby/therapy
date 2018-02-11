#ifndef MAP_H_74055FC0
#define MAP_H_74055FC0

#include <vector>
#include <string>
#include <list>
#include <stdexcept>
#include <map>

class Map {
public:

  Map(
    int id,
    std::vector<int> tiles,
    std::string title) :
      id_(id),
      tiles_(std::move(tiles)),
      title_(std::move(title))
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

private:

  int id_;
  std::vector<int> tiles_;
  std::string title_;
};

#endif /* end of include guard: MAP_H_74055FC0 */

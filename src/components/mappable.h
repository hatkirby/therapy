#ifndef MAPPABLE_H_0B0316FB
#define MAPPABLE_H_0B0316FB

#include <map>
#include "component.h"
#include "renderer.h"
#include "map.h"

class MappableComponent : public Component {
public:

  class Boundary {
  public:

    enum class Type {
      wall,
      wrap,
      teleport,
      reverse,
      platform,
      danger
    };

    Boundary(
      double axis,
      double lower,
      double upper,
      Type type) :
        axis_(axis),
        lower_(lower),
        upper_(upper),
        type_(type)
    {
    }

    inline double getAxis() const
    {
      return axis_;
    }

    inline double getLower() const
    {
      return lower_;
    }

    inline double getUpper() const
    {
      return upper_;
    }

    inline Type getType() const
    {
      return type_;
    }

  private:

    double axis_;
    double lower_;
    double upper_;
    Type type_;
  };

  MappableComponent(
    Texture tileset,
    Texture font) :
      tileset_(std::move(tileset)),
      font_(std::move(font))
  {
  }

  using asc_boundaries_type =
    std::multimap<
      double,
      Boundary,
      std::less<double>>;

  using desc_boundaries_type =
    std::multimap<
      double,
      Boundary,
      std::greater<double>>;

  inline size_t getMapId() const
  {
    return mapId_;
  }

  inline void setMapId(size_t v)
  {
    mapId_ = v;
  }

  inline desc_boundaries_type& getLeftBoundaries()
  {
    return leftBoundaries_;
  }

  inline asc_boundaries_type& getRightBoundaries()
  {
    return rightBoundaries_;
  }

  inline desc_boundaries_type& getUpBoundaries()
  {
    return upBoundaries_;
  }

  inline asc_boundaries_type& getDownBoundaries()
  {
    return downBoundaries_;
  }

  inline const Texture& getTileset() const
  {
    return tileset_;
  }

  inline void setTileset(Texture v)
  {
    tileset_ = std::move(v);
  }

  inline const Texture& getFont() const
  {
    return font_;
  }

  inline void setFont(Texture v)
  {
    font_ = std::move(v);
  }

private:

  size_t mapId_ = -1;

  desc_boundaries_type leftBoundaries_;
  asc_boundaries_type rightBoundaries_;
  desc_boundaries_type upBoundaries_;
  asc_boundaries_type downBoundaries_;
  Texture tileset_;
  Texture font_;
};

#endif /* end of include guard: MAPPABLE_H_0B0316FB */

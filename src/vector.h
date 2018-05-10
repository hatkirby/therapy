#ifndef COORDINATES_H_A45D34FB
#define COORDINATES_H_A45D34FB

template <typename T>
class vec2 {
public:

  T coords[2];

  vec2() : coords{0, 0}
  {
  }

  vec2(T x, T y) : coords{x, y}
  {
  }

  inline T& x()
  {
    return coords[0];
  }

  inline const T& x() const
  {
    return coords[0];
  }

  inline T& w()
  {
    return coords[0];
  }

  inline const T& w() const
  {
    return coords[0];
  }

  inline T& y()
  {
    return coords[1];
  }

  inline const T& y() const
  {
    return coords[1];
  }

  inline T& h()
  {
    return coords[1];
  }

  inline const T& h() const
  {
    return coords[1];
  }

  template <typename R>
  operator vec2<R>() const
  {
    return vec2<R>(x(), y());
  }

  vec2 operator+(const vec2& other) const
  {
    return vec2(x() + other.x(), y() + other.y());
  }

  vec2& operator+=(const vec2& other)
  {
    x() += other.x();
    y() += other.y();

    return *this;
  }

  vec2 operator-(const vec2& other) const
  {
    return vec2(x() - other.x(), y() - other.y());
  }

  vec2 operator-=(const vec2& other)
  {
    x() -= other.x();
    y() -= other.y();

    return *this;
  }

  vec2 operator-() const
  {
    return vec2(-x(), -y());
  }

  vec2 operator*(T s) const
  {
    return vec2(x() * s, y() * s);
  }

  vec2& operator*=(T s)
  {
    x() *= s;
    y() *= s;

    return *this;
  }

};

using vec2d = vec2<double>;
using vec2i = vec2<int>;

#endif /* end of include guard: COORDINATES_H_A45D34FB */

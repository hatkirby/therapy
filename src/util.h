#ifndef ALGORITHMS_H_1DDC517E
#define ALGORITHMS_H_1DDC517E

#include <fstream>
#include <sstream>
#include <cstring>

template< typename ContainerT, typename PredicateT >
void erase_if( ContainerT& items, const PredicateT& predicate ) {
  for( auto it = items.begin(); it != items.end(); ) {
    if( predicate(*it) ) it = items.erase(it);
    else ++it;
  }
};

struct chlit
{
  chlit(char c) : c_(c) { }
  char c_;
};

inline std::istream& operator>>(std::istream& is, chlit x)
{
  char c;
  if (is >> c && c != x.c_)
  {
    is.setstate(std::iostream::failbit);
  }

  return is;
}

std::string slurp(std::ifstream& in);

void flipImageData(
  unsigned char* data,
  int width,
  int height,
  int comps);

#endif /* end of include guard: ALGORITHMS_H_1DDC517E */

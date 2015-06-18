#ifndef ALGORITHMS_H_1DDC517E
#define ALGORITHMS_H_1DDC517E

template< typename ContainerT, typename PredicateT >
void erase_if( ContainerT& items, const PredicateT& predicate ) {
  for( auto it = items.begin(); it != items.end(); ) {
    if( predicate(*it) ) it = items.erase(it);
    else ++it;
  }
};

#endif /* end of include guard: ALGORITHMS_H_1DDC517E */

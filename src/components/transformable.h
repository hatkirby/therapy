#ifndef LOCATABLE_H_39E526CA
#define LOCATABLE_H_39E526CA

#include "component.h"
#include "vector.h"

class TransformableComponent : public Component {
public:

  /**
   * The coordinates of the entity.
   *
   * Note that ponderable entities sometimes have special behavior related to
   * their coordinates, specifically that ferried bodies will behave oddly if
   * their coordinates are changed outside of the PonderingSystem. Before doing
   * so, use PonderingSystem::unferry on the body to ensure that it is not
   * ferried.
   */
  vec2d pos;

  /**
   * The size of the entity.
   */
  vec2i size;

  /**
   * For prototypes, the original coordinates and size of the entity.
   *
   * @managed_by RealizingSystem
   */
  vec2d origPos;
  vec2i origSize;
};

#endif /* end of include guard: LOCATABLE_H_39E526CA */

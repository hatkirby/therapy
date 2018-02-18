#ifndef PLAYABLE_H_DDC566C3
#define PLAYABLE_H_DDC566C3

#include "component.h"

class PlayableComponent : public Component {
public:

  bool changingMap = false;
  int newMapId = -1;
  double newMapX = 0;
  double newMapY = 0;
};

#endif /* end of include guard: PLAYABLE_H_DDC566C3 */

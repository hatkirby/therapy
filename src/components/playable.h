#ifndef PLAYABLE_H_DDC566C3
#define PLAYABLE_H_DDC566C3

#include "component.h"
#include <functional>

class PlayableComponent : public Component {
public:

  using MapChangeCallback = std::function<void()>;

  bool changingMap = false;
  int newMapId = -1;
  double newMapX = 0;
  double newMapY = 0;
  MapChangeCallback newMapCallback;

  int checkpointMapId = -1;
  double checkpointX = 0;
  double checkpointY = 0;

};

#endif /* end of include guard: PLAYABLE_H_DDC566C3 */

#ifndef STATE_H
#define STATE_H

#include "renderer.h"

class State {
public:
  virtual void input(int key, int action) = 0;
  virtual void tick() = 0;
  virtual void render(Texture* tex) = 0;
};

#endif

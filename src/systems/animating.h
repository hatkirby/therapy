#ifndef ANIMATING_H_5BBF0094
#define ANIMATING_H_5BBF0094

#include "system.h"
#include <string>
#include "renderer/texture.h"

class AnimatingSystem : public System {
public:

  AnimatingSystem(Game& game) : System(game)
  {
  }

  void tick(double dt);

  void render(Texture& texture);

  void initPrototype(id_type entity);

  void startAnimation(id_type entity, std::string animation);

};

#endif /* end of include guard: ANIMATING_H_5BBF0094 */

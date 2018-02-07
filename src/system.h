#ifndef SYSTEM_H_B61A8CEA
#define SYSTEM_H_B61A8CEA

class Game;

class System {
public:
  System(Game& game)
    : game(game) {}

  virtual ~System() = default;

  virtual void tick(double dt) = 0;

protected:

  Game& game;
};

#endif /* end of include guard: SYSTEM_H_B61A8CEA */

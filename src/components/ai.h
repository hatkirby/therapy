#ifndef AI_H
#define AI_H

#include <list>
#include <map>
#include <string>
#include <memory>

#include "entity.h"

class AIAction {
  public:
    virtual void start(Game& game, Entity& entity) = 0;
    virtual void perform(Game& game, Entity& entity, double dt) = 0;
    virtual bool isDone() const = 0;
};

class AIActionContainer {
  public:
    void addAction(std::shared_ptr<AIAction> action);
    virtual void start(Game& game, Entity& entity);
    virtual void perform(Game& game, Entity& entity, double dt);
    virtual bool isDone() const;
  
  private:
    std::list<std::shared_ptr<AIAction>> actions;
    std::list<std::shared_ptr<AIAction>>::iterator currentAction {end(actions)};
};

class AI : public AIActionContainer {
  public:
    AI(int chance);
    
    int getChance() const;
    
  private:
    int chance;
};

class AIComponent : public Component {
  public:
    AI& emplaceAI(int chance);
    void tick(Game& game, Entity& entity, double dt);

  private:
    int maxChance = 0;
    std::list<AI> ais;
    AI* currentAI = nullptr;
};

class MoveAIAction : public AIAction {
  public:
    enum class Direction {
      Left,
      Right,
      Up,
      Down
    };
    
    MoveAIAction(Direction dir, int len, int speed);
    
    void start(Game& game, Entity& entity);
    void perform(Game& game, Entity& entity, double dt);
    bool isDone() const;
    
  private:
    Direction dir;
    int len;
    int speed;
    double remaining;
};

#endif /* end of include guard: AI_H */

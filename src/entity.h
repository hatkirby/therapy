#ifndef ENTITY_H
#define ENTITY_H

#include <list>
#include "renderer.h"

class Game;
class Map;
class Entity;
class Component;

class Message {
  public:
    enum class Type {
      walkLeft,
      walkRight,
      stopWalking,
      setHorizontalVelocity,
      setVerticalVelocity,
      collision,
      jump,
      stopJump,
      drop,
      canDrop,
      cantDrop,
      die,
      stopDying,
      hitTheGround
    };
    
    Message(Type type) : type(type) {}
    
    Type type;
    Entity* collisionEntity;
    int dropAxis;
    double velocity;
};

class Entity {
  public:
    void addComponent(std::shared_ptr<Component> c);
    void send(Game& game, const Message& msg);
    void tick(Game& game, double dt);
    void input(Game& game, int key, int action);
    void render(Game& game, Texture& buffer);
    void detectCollision(Game& game, Entity& collider, std::pair<double, double> old_position);
    
    std::pair<double, double> position;
    std::pair<int, int> size;
    
  private:
    std::list<std::shared_ptr<Component>> components;
};

class Component {
  public:
    virtual void receive(Game&, Entity&, const Message&) {}
    virtual void render(Game&, Entity&, Texture&) {}
    virtual void tick(Game&, Entity&, double) {}
    virtual void input(Game&, Entity&, int, int) {}
    virtual void detectCollision(Game&, Entity&, Entity&, std::pair<double, double>) {}
};

#endif

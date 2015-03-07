#ifndef ENTITY_H
#define ENTITY_H

class Entity;
class Component;
class Locatable;
class Collidable;

#include <list>
#include "renderer.h"
#include "world.h"

enum message_type {
  CM_WALK_LEFT,
  CM_WALK_RIGHT,
  CM_STOP_WALKING,
  CM_COLLISION,
  CM_JUMP,
  CM_STOP_JUMP,
  CM_DROP,
  CM_CAN_DROP,
  CM_CANT_DROP
};

typedef struct {
  message_type type;
  Entity* collisionEntity;
  int dropAxis;
} message_t;

class Entity {
  public:
    Entity(World* world) : world(world) {}
    ~Entity() {};
    void addComponent(std::shared_ptr<Component> c);
    void send(message_t msg);
    void tick();
    void input(int key, int action);
    void render(Texture* buffer);
    
    World* world;
    
  private:
    std::list<std::shared_ptr<Component>> components;
};

class Component {
  public:
    Component(Entity& entity) : entity(entity) {}
    virtual ~Component() {};
    virtual void receive(message_t msg) {(void)msg;}
    virtual void render(Texture* tex) {(void)tex;}
    virtual void tick() {}
    virtual void input(int key, int action) {(void)key; (void)action;}
    
    Entity& entity;
};

class Locatable {
  public:
    std::pair<double, double> position;
    std::pair<int, int> size;
    std::pair<double, double> velocity;
    std::pair<double, double> accel;
};

class Collidable {
  public:
    virtual void detectCollision(Entity& player, Locatable& physics, std::pair<double, double> old_position) = 0;
};

#endif

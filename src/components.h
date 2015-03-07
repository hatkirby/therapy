#ifndef COMPONENTS_H
#define COMPONENTS_H

#include "entity.h"
#include <utility>
#include <list>
#include "map.h"

class UserMovementComponent : public Component {
  public:
    UserMovementComponent(Entity& parent) : Component(parent) {};
    void input(int key, int action);
      
  private:
    bool holdingLeft = false;
    bool holdingRight = false;
};

class PhysicsBodyComponent : public Component, public Collidable, public Locatable {
  public:
    PhysicsBodyComponent(Entity& parent) : Component(parent) {};
    void receive(message_t msg);
    void tick();
    void detectCollision(Entity& player, Locatable& physics, std::pair<double, double> old_position);
};

class PlayerSpriteComponent : public Component {
  public:
    PlayerSpriteComponent(Entity& parent, Locatable& physics);
    ~PlayerSpriteComponent();
    void render(Texture* buffer);
    void receive(message_t msg);
    void tick();
    
  private:
    Locatable& physics;
    Texture* sprite;
    int animFrame = 0;
    bool facingLeft = false;
    bool isMoving = false;
};

class PlayerPhysicsComponent : public Component, public Locatable {
  public:
    PlayerPhysicsComponent(Entity& parent);
    void tick();
    void receive(message_t msg);
    
  private:
    double jump_velocity;
    double jump_gravity;
    double jump_gravity_short;
    int direction = 0;
    bool canDrop = false;
};

class MapRenderComponent : public Component {
  public:
    MapRenderComponent(Entity& parent, Map* map);
    ~MapRenderComponent();
    void render(Texture* buffer);
    
  private:
    Texture* screen;
};

enum direction_t {
  up, left, down, right
};

typedef struct {
  int axis;
  int lower;
  int upper;
  int type;
} collision_t;

class MapCollisionComponent : public Component, public Collidable {
  public:
    MapCollisionComponent(Entity& parent, Map* map);
    void detectCollision(Entity& player, Locatable& physics, std::pair<double, double> old_position);
    
  private:
    void add_collision(int axis, int lower, int upper, direction_t dir, int type);
    
    std::list<collision_t> left_collisions;
    std::list<collision_t> right_collisions;
    std::list<collision_t> up_collisions;
    std::list<collision_t> down_collisions;
    Map* map;
};

#endif

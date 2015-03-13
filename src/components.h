#ifndef COMPONENTS_H
#define COMPONENTS_H

#include "entity.h"
#include <utility>
#include <list>
#include "map.h"
#include <memory>

class UserMovementComponent : public Component {
  public:
    void input(Game& game, Entity& entity, int key, int action);
      
  private:
    bool holdingLeft = false;
    bool holdingRight = false;
};

class PhysicsBodyComponent : public Component {
  public:
    void receive(Game& game, Entity& entity, const Message& msg);
    void tick(Game& game, Entity& entity, double dt);
    void detectCollision(Game& game, Entity& entity, Entity& collider, std::pair<double, double> old_position);
    
  protected:    
    std::pair<double, double> velocity;
    std::pair<double, double> accel;
};

class PlayerSpriteComponent : public Component {
  public:
    void render(Game& game, Entity& entity, Texture& buffer);
    void receive(Game& game, Entity& entity, const Message& msg);
    
  private:
    Texture sprite{"../res/Starla.png"};
    int animFrame = 0;
    bool facingLeft = false;
    bool isMoving = false;
    bool dying = false;
};

class PlayerPhysicsComponent : public PhysicsBodyComponent {
  public:
    PlayerPhysicsComponent();
    void tick(Game& game, Entity& entity, double dt);
    void receive(Game& game, Entity& entity, const Message& msg);
    
  private:
    double jump_velocity;
    double jump_gravity;
    double jump_gravity_short;
    int direction = 0;
    bool canDrop = false;
    bool frozen = false;
};

class MapRenderComponent : public Component {
  public:
    MapRenderComponent(const Map& map);
    void render(Game& game, Entity& entity, Texture& buffer);
    
  private:
    Texture screen{GAME_WIDTH, GAME_HEIGHT};
};

class MapCollisionComponent : public Component {
  public:
    MapCollisionComponent(const Map& map);
    void detectCollision(Game& game, Entity& entity, Entity& collider, std::pair<double, double> old_position);
    
  private:
    enum class Direction {
      up, left, down, right
    };

    struct Collision {
      enum class Type {
        wall,
        wrap,
        teleport,
        reverse,
        platform,
        danger
      };
      
      int axis;
      int lower;
      int upper;
      Type type;
    };
    
    void addCollision(int axis, int lower, int upper, Direction dir, Collision::Type type);
    bool processCollision(Game& game, Entity& collider, Collision collision, Direction dir);
    
    std::list<Collision> left_collisions;
    std::list<Collision> right_collisions;
    std::list<Collision> up_collisions;
    std::list<Collision> down_collisions;
    const Map& map;
};

class StaticImageComponent : public Component {
  public:
    StaticImageComponent(const char* filename);
    void render(Game& game, Entity& entity, Texture& buffer);
    
  private:
    Texture sprite;
};

class SimpleColliderComponent : public Component {
  public:
    SimpleColliderComponent(std::function<void (Entity& collider)> callback);
    void receive(Game& game, Entity& entity, const Message& msg);
    
  private:
    std::function<void (Entity& collider)> callback;
};

#endif

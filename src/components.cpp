#include "components.h"
#include "game.h"
#include "muxer.h"
#include "map.h"

// User movement component

void UserMovementComponent::input(Game& game, Entity& entity, int key, int action)
{
  if (action == GLFW_PRESS)
  {
    if (key == GLFW_KEY_LEFT)
    {
      holdingLeft = true;
      
      if (!frozen)
      {
        entity.send(game, Message::Type::walkLeft);
      }
    } else if (key == GLFW_KEY_RIGHT)
    {
      holdingRight = true;
      
      if (!frozen)
      {
        entity.send(game, Message::Type::walkRight);
      }
    } else if (key == GLFW_KEY_UP)
    {
      if (!frozen)
      {
        entity.send(game, Message::Type::jump);
      }
    } else if (key == GLFW_KEY_DOWN)
    {
      if (!frozen)
      {
        entity.send(game, Message::Type::canDrop);
      }
    }
  } else if (action == GLFW_RELEASE)
  {
    if (key == GLFW_KEY_LEFT)
    {
      holdingLeft = false;
      
      if (!frozen)
      {
        if (holdingRight)
        {
          entity.send(game, Message::Type::walkRight);
        } else {
          entity.send(game, Message::Type::stopWalking);
        }
      }
    } else if (key == GLFW_KEY_RIGHT)
    {
      holdingRight = false;
      
      if (!frozen)
      {
        if (holdingLeft)
        {
          entity.send(game, Message::Type::walkLeft);
        } else {
          entity.send(game, Message::Type::stopWalking);
        }
      }
    } else if (key == GLFW_KEY_DOWN)
    {
      if (!frozen)
      {
        entity.send(game, Message::Type::cantDrop);
      }
    } else if (key == GLFW_KEY_UP)
    {
      if (!frozen)
      {
        entity.send(game, Message::Type::stopJump);
      }
    }
  }
}

void UserMovementComponent::receive(Game& game, Entity& entity, const Message& msg)
{
  if (msg.type == Message::Type::die)
  {
    frozen = true;
    
    entity.send(game, Message::Type::stopWalking);
  } else if (msg.type == Message::Type::stopDying)
  {
    frozen = false;
    
    if (holdingLeft)
    {
      entity.send(game, Message::Type::walkLeft);
    } else if (holdingRight)
    {
      entity.send(game, Message::Type::walkRight);
    }
  }
}

// Physics component

void PhysicsBodyComponent::receive(Game&, Entity&, const Message& msg)
{
  if (msg.type == Message::Type::walkLeft)
  {
    velocity.first = -90;
  } else if (msg.type == Message::Type::walkRight)
  {
    velocity.first = 90;
  } else if (msg.type == Message::Type::stopWalking)
  {
    velocity.first = 0.0;
  } else if (msg.type == Message::Type::stopMovingHorizontally)
  {
    velocity.first = 0.0;
  } else if (msg.type == Message::Type::stopMovingVertically)
  {
    velocity.second = 0.0;
  }
}

void PhysicsBodyComponent::tick(Game&, Entity& entity, double dt)
{
  // Accelerate
  velocity.first += accel.first * dt;
  velocity.second += accel.second * dt;
  
  // Terminal velocity
#define TERMINAL_VELOCITY_X (2 * TILE_WIDTH * FRAMES_PER_SECOND)
#define TERMINAL_VELOCITY_Y (2 * TILE_HEIGHT * FRAMES_PER_SECOND)
  if (velocity.first < -TERMINAL_VELOCITY_X) velocity.first = -TERMINAL_VELOCITY_X;
  if (velocity.first > TERMINAL_VELOCITY_X) velocity.first = TERMINAL_VELOCITY_X;
  if (velocity.second < -TERMINAL_VELOCITY_Y) velocity.second = -TERMINAL_VELOCITY_Y;
  if (velocity.second > TERMINAL_VELOCITY_Y) velocity.second = TERMINAL_VELOCITY_Y;
  
  // Do the movement
  entity.position.first += velocity.first * dt;
  entity.position.second += velocity.second * dt;
}

void PhysicsBodyComponent::detectCollision(Game& game, Entity& entity, Entity& collider, std::pair<double, double> old_position)
{
  // If already colliding, do nothing!
  if ((old_position.first + collider.size.first > entity.position.first)
    && (old_position.first < entity.position.first + entity.size.first)
    && (old_position.second + collider.size.second > entity.position.second)
    && (old_position.second < entity.position.second + entity.size.second))
  {
    return;
  }
  
  // If newly colliding, SHOCK AND HORROR!
  if ((collider.position.first + collider.size.first > entity.position.first)
    && (collider.position.first < entity.position.first + entity.size.first)
    && (collider.position.second + collider.size.second > entity.position.second)
    && (collider.position.second < entity.position.second + entity.size.second))
  {
    Message msg(Message::Type::collision);
    msg.collisionEntity = &collider;
    
    entity.send(game, msg);
  }
}

// Render player

void PlayerSpriteComponent::render(Game&, Entity& entity, Texture& buffer)
{
  animFrame++;
  
  int frame = 0;
  if (isMoving)
  {
    frame += 2;
    
    if (animFrame % 20 < 10)
    {
      frame += 2;
    }
  }
  
  if (facingLeft)
  {
    frame++;
  }
  
  double alpha = 1.0;
  if (dying && (animFrame % 4 < 2))
  {
    alpha = 0.0;
  }
  
  Rectangle src_rect {frame*10, 0, 10, 12};
  Rectangle dst_rect {(int) entity.position.first, (int) entity.position.second, entity.size.first, entity.size.second};
  buffer.blit(sprite, src_rect, dst_rect, alpha);
}

void PlayerSpriteComponent::receive(Game&, Entity&, const Message& msg)
{
  if (msg.type == Message::Type::walkLeft)
  {
    facingLeft = true;
    isMoving = true;
  } else if (msg.type == Message::Type::walkRight)
  {
    facingLeft = false;
    isMoving = true;
  } else if (msg.type == Message::Type::stopWalking)
  {
    isMoving = false;
  } else if (msg.type == Message::Type::die)
  {
    dying = true;
    isMoving = false;
  } else if (msg.type == Message::Type::stopDying)
  {
    dying = false;
  }
}

// Player physics

#define JUMP_VELOCITY(h, l) (-2 * (h) / (l))
#define JUMP_GRAVITY(h, l) (2 * ((h) / (l)) / (l))

PlayerPhysicsComponent::PlayerPhysicsComponent()
{
  jump_velocity = JUMP_VELOCITY(TILE_HEIGHT*4.5, 0.3);
  jump_gravity = JUMP_GRAVITY(TILE_HEIGHT*4.5, 0.3);
  jump_gravity_short = JUMP_GRAVITY(TILE_HEIGHT*3.5, 0.233);
  
  accel.second = jump_gravity_short;
}

void PlayerPhysicsComponent::receive(Game&, Entity& entity, const Message& msg)
{
  if (msg.type == Message::Type::walkLeft)
  {
    velocity.first = -90;
    direction = -1;
  } else if (msg.type == Message::Type::walkRight)
  {
    velocity.first = 90;
    direction = 1;
  } else if (msg.type == Message::Type::stopWalking)
  {
    velocity.first = 0.0;
    direction = 0;
  } else if (msg.type == Message::Type::stopMovingHorizontally)
  {
    velocity.first = 0.0;
  } else if (msg.type == Message::Type::stopMovingVertically)
  {
    velocity.second = 0.0;
  } else if (msg.type == Message::Type::jump)
  {
    playSound("../res/Randomize87.wav", 0.25);
    
    velocity.second = jump_velocity;
    accel.second = jump_gravity;
  } else if (msg.type == Message::Type::stopJump)
  {
    accel.second = jump_gravity_short;
  } else if (msg.type == Message::Type::canDrop)
  {
    canDrop = true;
  } else if (msg.type == Message::Type::cantDrop)
  {
    canDrop = false;
  } else if (msg.type == Message::Type::drop)
  {
    if (canDrop)
    {
      canDrop = false;
    } else {
      entity.position.second = msg.dropAxis - entity.size.second;
      velocity.second = 0;
    }
  } else if (msg.type == Message::Type::die)
  {
    frozen = true;
  } else if (msg.type == Message::Type::stopDying)
  {
    frozen = false;
  }
}

void PlayerPhysicsComponent::tick(Game& game, Entity& entity, double dt)
{
  // If frozen, do nothing
  if (frozen)
  {
    return;
  }
  
  // Continue walking even if blocked earlier
  if (velocity.first == 0)
  {
    if (direction < 0)
    {
      velocity.first = -90;
    } else if (direction > 0)
    {
      velocity.first = 90;
    }
  }
  
  // Increase gravity at the height of jump
  if ((accel.second == jump_gravity) && (velocity.second >= 0))
  {
    accel.second = jump_gravity_short;
  }
  
  // Do the movement
  std::pair<double, double> old_position = entity.position;
  PhysicsBodyComponent::tick(game, entity, dt);
    
  // Check for collisions
  game.detectCollision(entity, old_position);
}

// Map rendering

MapRenderComponent::MapRenderComponent(const Map& map)
{
  screen.fill(screen.entirety(), 0, 0, 0);
  
  Texture tiles("../res/tiles.png");
  
  for (int i=0; i<MAP_WIDTH*(MAP_HEIGHT-1); i++)
  {
    int tile = map.getMapdata()[i];
    int x = i % MAP_WIDTH;
    int y = i / MAP_WIDTH;
    Rectangle dst {x*TILE_WIDTH, y*TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT};
    Rectangle src {tile%8*TILE_WIDTH, tile/8*TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT};

    if (tile > 0)
    {
      screen.blit(tiles, src, dst);
    }
  }
  
  Texture font("../res/font.bmp");
  const char* map_name = map.getTitle();
  int start_x = (40/2) - (strlen(map_name)/2);
  for (size_t i=0; i<strlen(map_name); i++)
  {
    Rectangle srcRect {map_name[i] % 16 * 8, map_name[i] / 16 * 8, 8, 8};
    Rectangle dstRect {(start_x + (int)i)*8, 24*8, 8, 8};
    screen.blit(font, srcRect, dstRect);
  }
}

void MapRenderComponent::render(Game&, Entity&, Texture& buffer)
{
  buffer.blit(screen, screen.entirety(), buffer.entirety());
}

// Map collision

MapCollisionComponent::MapCollisionComponent(const Map& map) : map(map)
{
  addCollision(-6, 0, GAME_WIDTH, Direction::left, (map.getLeftMap() == nullptr) ? Collision::Type::wrap : Collision::Type::teleport);
  addCollision(GAME_WIDTH+6, 0, GAME_WIDTH, Direction::right, (map.getRightMap() == nullptr) ? Collision::Type::reverse : Collision::Type::teleport);
  
  for (int i=0; i<MAP_WIDTH*(MAP_HEIGHT-1); i++)
  {
    int x = i % MAP_WIDTH;
    int y = i / MAP_WIDTH;
    int tile = map.getMapdata()[i];
    
    if ((tile > 0) && (tile < 28) && (!((tile >= 5) && (tile <= 7))))
    {
      addCollision(x*TILE_WIDTH, y*TILE_HEIGHT, (y+1)*TILE_HEIGHT, Direction::right, Collision::Type::wall);
      addCollision((x+1)*TILE_WIDTH, y*TILE_HEIGHT, (y+1)*TILE_HEIGHT, Direction::left, Collision::Type::wall);
      addCollision(y*TILE_HEIGHT, x*TILE_WIDTH, (x+1)*TILE_WIDTH, Direction::down, Collision::Type::wall);
      addCollision((y+1)*TILE_HEIGHT, x*TILE_WIDTH, (x+1)*TILE_WIDTH, Direction::up, Collision::Type::wall);
    } else if ((tile >= 5) && (tile <= 7))
    {
      addCollision(y*TILE_HEIGHT, x*TILE_WIDTH, (x+1)*TILE_WIDTH, Direction::down, Collision::Type::platform);
    } else if (tile == 42)
    {
      addCollision(y*TILE_HEIGHT, x*TILE_WIDTH, (x+1)*TILE_WIDTH, Direction::down, Collision::Type::danger);
    }
  }
}

void MapCollisionComponent::addCollision(int axis, int lower, int upper, Direction dir, Collision::Type type)
{
  std::list<Collision>::iterator it;
  
  switch (dir)
  {
    case Direction::up:
      it = up_collisions.begin();
      for (; it!=up_collisions.end(); it++)
      {
        if (it->axis < axis) break;
      }
    
      up_collisions.insert(it, {axis, lower, upper, type});
      
      break;
    case Direction::down:
      it = down_collisions.begin();
      for (; it!=down_collisions.end(); it++)
      {
        if (it->axis > axis) break;
      }
    
      down_collisions.insert(it, {axis, lower, upper, type});
      
      break;
    case Direction::left:
      it = left_collisions.begin();
      for (; it!=left_collisions.end(); it++)
      {
        if (it->axis < axis) break;
      }
  
      left_collisions.insert(it, {axis, lower, upper, type});
      
      break;
    case Direction::right:
      it = right_collisions.begin();
      for (; it!=right_collisions.end(); it++)
      {
        if (it->axis > axis) break;
      }
  
      right_collisions.insert(it, {axis, lower, upper, type});
      
      break;
  }
}

void MapCollisionComponent::detectCollision(Game& game, Entity&, Entity& collider, std::pair<double, double> old_position)
{
  int fixed_x = (int) collider.position.first;
  int fixed_y = (int) collider.position.second;
  int fixed_ox = (int) old_position.first;
  int fixed_oy = (int) old_position.second;
  
  if (fixed_x < fixed_ox)
  {
    for (auto collision : left_collisions)
    {
      if (collision.axis > fixed_ox) continue;
      if (collision.axis < fixed_x) break;
  
      if ((fixed_oy+collider.size.second > collision.lower) && (fixed_oy < collision.upper))
      {
        // We have a collision!
        if (processCollision(game, collider, collision, Direction::left))
        {
          collider.position.second = old_position.second;
          
          return;
        }
    
        break;
      }
    }
  } else if (fixed_x > fixed_ox)
  {
    for (auto collision : right_collisions)
    {
      if (collision.axis < fixed_ox+collider.size.first) continue;
      if (collision.axis > fixed_x+collider.size.first) break;
  
      if ((fixed_oy+collider.size.second > collision.lower) && (fixed_oy < collision.upper))
      {
        // We have a collision!
        if (processCollision(game, collider, collision, Direction::right))
        {
          collider.position.second = old_position.second;
          
          return;
        }
    
        break;
      }
    }
  }
  
  fixed_x = (int) collider.position.first;
  fixed_y = (int) collider.position.second;
  
  if (fixed_y < fixed_oy)
  {
    for (auto collision : up_collisions)
    {
      if (collision.axis > fixed_oy) continue;
      if (collision.axis < fixed_y) break;
  
      if ((fixed_x+collider.size.first > collision.lower) && (fixed_x < collision.upper))
      {
        // We have a collision!
        if (processCollision(game, collider, collision, Direction::up))
        {
          return;
        }
    
        break;
      }
    }
  } else if (fixed_y > fixed_oy)
  {
    for (auto collision : down_collisions)
    {
      if (collision.axis < fixed_oy+collider.size.second) continue;
      if (collision.axis > fixed_y+collider.size.second) break;

      if ((fixed_x+collider.size.first > collision.lower) && (fixed_x < collision.upper))
      {
        // We have a collision!
        if (processCollision(game, collider, collision, Direction::down))
        {
          return;
        }
        
        break;
      }
    }
  }
}

bool MapCollisionComponent::processCollision(Game& game, Entity& collider, Collision collision, Direction dir)
{
  if (collision.type == Collision::Type::wall)
  {
    if (dir == Direction::left)
    {
      collider.position.first = collision.axis;
      collider.send(game, Message::Type::stopMovingHorizontally);
    } else if (dir == Direction::right)
    {
      collider.position.first = collision.axis - collider.size.first;
      collider.send(game, Message::Type::stopMovingHorizontally);
    } else if (dir == Direction::up)
    {
      collider.position.second = collision.axis;
      collider.send(game, Message::Type::stopMovingVertically);
    } else if (dir == Direction::down)
    {
      collider.position.second = collision.axis - collider.size.second;
      collider.send(game, Message::Type::stopMovingVertically);
    }
  } else if (collision.type == Collision::Type::wrap)
  {
    if (dir == Direction::left)
    {
      collider.position.first = GAME_WIDTH-collider.size.first/2;
    } else if (dir == Direction::right)
    {
      collider.position.first = -collider.size.first/2;
    } else if (dir == Direction::up)
    {
      collider.position.second = GAME_HEIGHT-collider.size.second/2-1;
    } else if (dir == Direction::down)
    {
      collider.position.second = -collider.size.second/2;
    }
  } else if (collision.type == Collision::Type::teleport)
  {
    if (dir == Direction::left)
    {
      collider.position.first = GAME_WIDTH-collider.size.first/2;
      game.loadMap(*map.getLeftMap());
    } else if (dir == Direction::right)
    {
      collider.position.first = -collider.size.first/2;
      game.loadMap(*map.getRightMap());
    }
    
    return true;
  } else if (collision.type == Collision::Type::reverse)
  {
    if (dir == Direction::right)
    {
      collider.position.first = collision.axis - collider.size.first;
      collider.send(game, Message::Type::walkLeft);
    }
  } else if (collision.type == Collision::Type::platform)
  {
    Message msg(Message::Type::drop);
    msg.dropAxis = collision.axis;

    collider.send(game, msg);
  } else if (collision.type == Collision::Type::danger)
  {
    game.playerDie();
  }
  
  return false;
}

// Static image

StaticImageComponent::StaticImageComponent(const char* filename) : sprite(Texture(filename))
{
  
}

void StaticImageComponent::render(Game&, Entity& entity, Texture& buffer)
{
  buffer.blit(sprite, sprite.entirety(), {(int) entity.position.first, (int) entity.position.second, entity.size.first, entity.size.second});
}

// Simple collision

SimpleColliderComponent::SimpleColliderComponent(std::function<void (Game& game, Entity& collider)> callback) : callback(callback)
{
  
}

void SimpleColliderComponent::receive(Game& game, Entity&, const Message& msg)
{
  if (msg.type == Message::Type::collision)
  {
    callback(game, *msg.collisionEntity);
  }
}

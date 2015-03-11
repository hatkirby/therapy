#include "components.h"
#include "game.h"

// User movement component

void UserMovementComponent::input(Game& game, Entity& entity, int key, int action)
{
  if (action == GLFW_PRESS)
  {
    if (key == GLFW_KEY_LEFT)
    {
      holdingLeft = true;
      
      Message msg(Message::Type::walkLeft);
      entity.send(game, msg);
    } else if (key == GLFW_KEY_RIGHT)
    {
      holdingRight = true;
      
      Message msg(Message::Type::walkRight);
      entity.send(game, msg);
    } else if (key == GLFW_KEY_UP)
    {
      Message msg(Message::Type::jump);
      entity.send(game, msg);
    } else if (key == GLFW_KEY_DOWN)
    {
      Message msg(Message::Type::canDrop);
      entity.send(game, msg);
    }
  } else if (action == GLFW_RELEASE)
  {
    if (key == GLFW_KEY_LEFT)
    {
      holdingLeft = false;
      
      if (holdingRight)
      {
        Message msg(Message::Type::walkRight);
        entity.send(game, msg);
      } else {
        Message msg(Message::Type::stopWalking);
        entity.send(game, msg);
      }
    } else if (key == GLFW_KEY_RIGHT)
    {
      holdingRight = false;
      
      if (holdingLeft)
      {
        Message msg(Message::Type::walkLeft);
        entity.send(game, msg);
      } else {
        Message msg(Message::Type::stopWalking);
        entity.send(game, msg);
      }
    } else if (key == GLFW_KEY_DOWN)
    {
      Message msg(Message::Type::cantDrop);
      entity.send(game, msg);
    } else if (key == GLFW_KEY_UP)
    {
      Message msg(Message::Type::stopJump);
      entity.send(game, msg);
    }
  }
}

// Physics component

void PhysicsBodyComponent::receive(Game&, Entity&, const Message& msg)
{
  if (msg.type == Message::Type::walkLeft)
  {
    velocity.first = -1.5;
  } else if (msg.type == Message::Type::walkRight)
  {
    velocity.first = 1.5;
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

void PhysicsBodyComponent::tick(Game&, Entity& entity)
{
  velocity.first += accel.first;
  velocity.second += accel.second;
  
  entity.position.first += velocity.first;
  entity.position.second += velocity.second;
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
  int frame = 0;
  if (isMoving)
  {
    frame += 2;
    
    if (animFrame < 10)
    {
      frame += 2;
    }
  }
  if (facingLeft) frame++;
  
  Rectangle src_rect {frame*10, 0, 10, 12};
  Rectangle dst_rect {(int) entity.position.first, (int) entity.position.second, entity.size.first, entity.size.second};
  buffer.blit(sprite, src_rect, dst_rect);
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
  }
}

void PlayerSpriteComponent::tick(Game&, Entity&)
{
  animFrame++;
  animFrame %= 20;
}

// Player physics

#define JUMP_VELOCITY(h, l) (-2 * (h) / (l))
#define JUMP_GRAVITY(h, l) (2 * ((h) / (l)) / (l))

PlayerPhysicsComponent::PlayerPhysicsComponent()
{
  jump_velocity = JUMP_VELOCITY(TILE_HEIGHT*4.5, 0.3*FRAMES_PER_SECOND);
  jump_gravity = JUMP_GRAVITY(TILE_HEIGHT*4.5, 0.3*FRAMES_PER_SECOND);
  jump_gravity_short = JUMP_GRAVITY(TILE_HEIGHT*3.5, 0.233*FRAMES_PER_SECOND);
  
  accel.second = jump_gravity_short;
}

void PlayerPhysicsComponent::receive(Game&, Entity& entity, const Message& msg)
{
  if (msg.type == Message::Type::walkLeft)
  {
    velocity.first = -1.5;
    direction = -1;
  } else if (msg.type == Message::Type::walkRight)
  {
    velocity.first = 1.5;
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
  }
}

void PlayerPhysicsComponent::tick(Game& game, Entity& entity)
{
  // Continue walking even if blocked earlier
  if (velocity.first == 0)
  {
    if (direction < 0)
    {
      velocity.first = -1.5;
    } else if (direction > 0)
    {
      velocity.first = 1.5;
    }
  }
  
  // Increase gravity at the height of jump
  if ((accel.second == jump_gravity) && (velocity.second >= 0))
  {
    accel.second = jump_gravity_short;
  }
  
  // Apply acceleration
  velocity.first += accel.first;
  velocity.second += accel.second;
  
  // Terminal velocity
  if (velocity.first < -16) velocity.first = -16;
  if (velocity.first > 16) velocity.first = 16;
  if (velocity.second < -16) velocity.second = -16;
  if (velocity.second > 16) velocity.second = 16;
  
  // Do the movement
  std::pair<double, double> old_position = entity.position;
  entity.position.first += velocity.first;
  entity.position.second += velocity.second;
  
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
    int tile = map.mapdata()[i];
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
  const char* map_name = map.title();
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

MapCollisionComponent::MapCollisionComponent(const Map& map)
{
  leftMap = map.getLeftMap();
  rightMap = map.getRightMap();
  
  addCollision(-6, 0, GAME_WIDTH, Direction::left, (leftMap == nullptr) ? 1 : 2);
  addCollision(GAME_WIDTH+6, 0, GAME_WIDTH, Direction::right, (rightMap == nullptr) ? 3 : 2);
  
  for (int i=0; i<MAP_WIDTH*(MAP_HEIGHT-1); i++)
  {
    int x = i % MAP_WIDTH;
    int y = i / MAP_WIDTH;
    int tile = map.mapdata()[i];
    
    if ((tile > 0) && (!((tile >= 5) && (tile <= 7))))
    {
      addCollision(x*TILE_WIDTH, y*TILE_HEIGHT, (y+1)*TILE_HEIGHT, Direction::right, 0);
      addCollision((x+1)*TILE_WIDTH, y*TILE_HEIGHT, (y+1)*TILE_HEIGHT, Direction::left, 0);
      addCollision(y*TILE_HEIGHT, x*TILE_WIDTH, (x+1)*TILE_WIDTH, Direction::down, 0);
      addCollision((y+1)*TILE_HEIGHT, x*TILE_WIDTH, (x+1)*TILE_WIDTH, Direction::up, 0);
    } else if ((tile >= 5) && (tile <= 7))
    {
      addCollision(y*TILE_HEIGHT, x*TILE_WIDTH, (x+1)*TILE_WIDTH, Direction::down, 4);
    }
  }
}

void MapCollisionComponent::addCollision(int axis, int lower, int upper, Direction dir, int type)
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
  if (collision.type == 0)
  {
    if (dir == Direction::left)
    {
      collider.position.first = collision.axis;
    
      Message msg(Message::Type::stopMovingHorizontally);
      collider.send(game, msg);
    } else if (dir == Direction::right)
    {
      collider.position.first = collision.axis - collider.size.first;
    
      Message msg(Message::Type::stopMovingHorizontally);
      collider.send(game, msg);
    } else if (dir == Direction::up)
    {
      collider.position.second = collision.axis;
      
      Message msg(Message::Type::stopMovingVertically);
      collider.send(game, msg);
    } else if (dir == Direction::down)
    {
      collider.position.second = collision.axis - collider.size.second;
      
      Message msg(Message::Type::stopMovingVertically);
      collider.send(game, msg);
    }
  } else if (collision.type == 1)
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
  } else if (collision.type == 2)
  {
    if (dir == Direction::left)
    {
      collider.position.first = GAME_WIDTH-collider.size.first/2;
      game.loadMap(*leftMap);
    } else if (dir == Direction::right)
    {
      collider.position.first = -collider.size.first/2;
      game.loadMap(*rightMap);
    }
    
    return true;
  } else if (collision.type == 3)
  {
    if (dir == Direction::right)
    {
      collider.position.first = collision.axis - collider.size.first;
    
      Message msg(Message::Type::walkLeft);
      collider.send(game, msg);
    }
  } else if (collision.type == 4)
  {
    Message msg(Message::Type::drop);
    msg.dropAxis = collision.axis;

    collider.send(game, msg);
  }
  
  return false;
}

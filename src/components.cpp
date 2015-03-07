#include "components.h"
#include "game.h"

// User movement component

void UserMovementComponent::input(int key, int action)
{
  if (action == GLFW_PRESS)
  {
    if (key == GLFW_KEY_LEFT)
    {
      holdingLeft = true;
      
      message_t msg;
      msg.type = CM_WALK_LEFT;
      
      entity.send(msg);
    } else if (key == GLFW_KEY_RIGHT)
    {
      holdingRight = true;
      
      message_t msg;
      msg.type = CM_WALK_RIGHT;
      
      entity.send(msg);
    } else if (key == GLFW_KEY_UP)
    {
      message_t msg;
      msg.type = CM_JUMP;
      
      entity.send(msg);
    } else if (key == GLFW_KEY_DOWN)
    {
      message_t msg;
      msg.type = CM_CAN_DROP;
      
      entity.send(msg);
    }
  } else if (action == GLFW_RELEASE)
  {
    if (key == GLFW_KEY_LEFT)
    {
      holdingLeft = false;
      
      if (holdingRight)
      {
        message_t msg;
        msg.type = CM_WALK_RIGHT;
        
        entity.send(msg);
      } else {
        message_t msg;
        msg.type = CM_STOP_WALKING;
        
        entity.send(msg);
      }
    } else if (key == GLFW_KEY_RIGHT)
    {
      holdingRight = false;
      
      if (holdingLeft)
      {
        message_t msg;
        msg.type = CM_WALK_LEFT;
        
        entity.send(msg);
      } else {
        message_t msg;
        msg.type = CM_STOP_WALKING;
        
        entity.send(msg);
      }
    } else if (key == GLFW_KEY_DOWN)
    {
      message_t msg;
      msg.type = CM_CANT_DROP;
      
      entity.send(msg);
    } else if (key == GLFW_KEY_UP)
    {
      message_t msg;
      msg.type = CM_STOP_JUMP;
      
      entity.send(msg);
    }
  }
}

// Physics component

void PhysicsBodyComponent::receive(message_t msg)
{
  if (msg.type == CM_WALK_LEFT)
  {
    velocity.first = -1.5;
  } else if (msg.type == CM_WALK_RIGHT)
  {
    velocity.first = 1.5;
  } else if (msg.type == CM_STOP_WALKING)
  {
    velocity.first = 0.0;
  }
}

void PhysicsBodyComponent::tick()
{
  velocity.first += accel.first;
  velocity.second += accel.second;
  
  position.first += velocity.first;
  position.second += velocity.second;
}

void PhysicsBodyComponent::detectCollision(Entity& player, Locatable& physics, std::pair<double, double> old_position)
{
  // If already colliding, do nothing!
  if ((old_position.first + physics.size.first > this->position.first)
    && (old_position.first < this->position.first + this->size.first)
    && (old_position.second + physics.size.second > this->position.second)
    && (old_position.second < this->position.second + this->size.second))
  {
    return;
  }
  
  // If newly colliding, SHOCK AND HORROR!
  if ((physics.position.first + physics.size.first > this->position.first)
    && (physics.position.first < this->position.first + this->size.first)
    && (physics.position.second + physics.size.second > this->position.second)
    && (physics.position.second < this->position.second + this->size.second))
  {
    message_t msg;
    msg.type = CM_COLLISION;
    msg.collisionEntity = &player;
    
    entity.send(msg);
  }
}

// Render player

PlayerSpriteComponent::PlayerSpriteComponent(Entity& entity, Locatable& physics) : Component(entity), physics(physics)
{
  sprite = loadTextureFromFile("../res/Starla.png");
}

PlayerSpriteComponent::~PlayerSpriteComponent()
{
  destroyTexture(sprite);
}

void PlayerSpriteComponent::render(Texture* buffer)
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
  
  Rectangle src_rect(frame*10, 0, 10, 12);
  Rectangle dst_rect((int) physics.position.first, (int) physics.position.second, physics.size.first, physics.size.second);
  blitTexture(sprite, buffer, &src_rect, &dst_rect);
}

void PlayerSpriteComponent::receive(message_t msg)
{
  if (msg.type == CM_WALK_LEFT)
  {
    facingLeft = true;
    isMoving = true;
  } else if (msg.type == CM_WALK_RIGHT)
  {
    facingLeft = false;
    isMoving = true;
  } else if (msg.type == CM_STOP_WALKING)
  {
    isMoving = false;
  }
}

void PlayerSpriteComponent::tick()
{
  animFrame++;
  animFrame %= 20;
}

// Player physics

#define JUMP_VELOCITY(h, l) (-2 * (h) / (l))
#define JUMP_GRAVITY(h, l) (2 * ((h) / (l)) / (l))

PlayerPhysicsComponent::PlayerPhysicsComponent(Entity& entity) : Component(entity)
{
  jump_velocity = JUMP_VELOCITY(TILE_HEIGHT*4.5, 0.3*FRAMES_PER_SECOND);
  jump_gravity = JUMP_GRAVITY(TILE_HEIGHT*4.5, 0.3*FRAMES_PER_SECOND);
  jump_gravity_short = JUMP_GRAVITY(TILE_HEIGHT*3.5, 0.233*FRAMES_PER_SECOND);
  
  accel.second = jump_gravity_short;
}

void PlayerPhysicsComponent::receive(message_t msg)
{
  if (msg.type == CM_WALK_LEFT)
  {
    velocity.first = -1.5;
    direction = -1;
  } else if (msg.type == CM_WALK_RIGHT)
  {
    velocity.first = 1.5;
    direction = 1;
  } else if (msg.type == CM_STOP_WALKING)
  {
    velocity.first = 0.0;
    direction = 0;
  } else if (msg.type == CM_JUMP)
  {
    velocity.second = jump_velocity;
    accel.second = jump_gravity;
  } else if (msg.type == CM_STOP_JUMP)
  {
    accel.second = jump_gravity_short;
  } else if (msg.type == CM_CAN_DROP)
  {
    canDrop = true;
  } else if (msg.type == CM_CANT_DROP)
  {
    canDrop = false;
  } else if (msg.type == CM_DROP)
  {
    if (canDrop)
    {
      canDrop = false;
    } else {
      position.second = msg.dropAxis - size.second;
      velocity.second = 0;
    }
  }
}

void PlayerPhysicsComponent::tick()
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
  std::pair<double, double> old_position = std::make_pair(position.first, position.second);
  position.first += velocity.first;
  position.second += velocity.second;
  
  // Check for collisions
  for (auto it = entity.world->bodies.begin(); it != entity.world->bodies.end(); it++)
  {
    auto poop = *it;
    poop->detectCollision(entity, *this, old_position);
  }
}

// Map rendering

MapRenderComponent::MapRenderComponent(Entity& entity, Map* map) : Component(entity)
{
  screen = createTexture(GAME_WIDTH, GAME_HEIGHT);
  fillTexture(screen, NULL, 0, 0, 0);
  
  Texture* tiles = loadTextureFromFile("../res/tiles.png");
  
  for (int i=0; i<MAP_WIDTH*(MAP_HEIGHT-1); i++)
  {
    int tile = map->mapdata()[i];
    int x = i % MAP_WIDTH;
    int y = i / MAP_WIDTH;
    Rectangle dst(x*TILE_WIDTH, y*TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT);
    Rectangle src(tile%8*TILE_WIDTH, tile/8*TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT);
    
    if (tile > 0)
    {
      blitTexture(tiles, screen, &src, &dst);
    }
  }
  
  destroyTexture(tiles);
  
  Texture* font = loadTextureFromFile("../res/font.bmp");
  const char* map_name = map->title();
  int start_x = (40/2) - (strlen(map_name)/2);
  for (size_t i=0; i<strlen(map_name); i++)
  {
    Rectangle srcRect(map_name[i] % 16 * 8, map_name[i] / 16 * 8, 8, 8);
    Rectangle dstRect((start_x + i)*8, 24*8, 8, 8);
    blitTexture(font, screen, &srcRect, &dstRect);
  }
  
  destroyTexture(font);
}

MapRenderComponent::~MapRenderComponent()
{
  destroyTexture(screen);
}

void MapRenderComponent::render(Texture* buffer)
{
  blitTexture(screen, buffer, NULL, NULL);
}

// Map collision

MapCollisionComponent::MapCollisionComponent(Entity& entity, Map* map) : Component(entity)
{
  this->map = map;
  
  add_collision(-6, 0, GAME_WIDTH, left, (map->getLeftMap() == NULL) ? 1 : 2);
  add_collision(GAME_WIDTH+6, 0, GAME_WIDTH, right, (map->getRightMap() == NULL) ? 3 : 2);
  
  for (int i=0; i<MAP_WIDTH*(MAP_HEIGHT-1); i++)
  {
    int x = i % MAP_WIDTH;
    int y = i / MAP_WIDTH;
    int tile = map->mapdata()[i];
    
    if ((tile > 0) && (!((tile >= 5) && (tile <= 7))))
    {
      add_collision(x*TILE_WIDTH, y*TILE_HEIGHT, (y+1)*TILE_HEIGHT, right, 0);
      add_collision((x+1)*TILE_WIDTH, y*TILE_HEIGHT, (y+1)*TILE_HEIGHT, left, 0);
      add_collision(y*TILE_HEIGHT, x*TILE_WIDTH, (x+1)*TILE_WIDTH, down, 0);
      add_collision((y+1)*TILE_HEIGHT, x*TILE_WIDTH, (x+1)*TILE_WIDTH, up, 0);
    } else if ((tile >= 5) && (tile <= 7))
    {
      add_collision(y*TILE_HEIGHT, x*TILE_WIDTH, (x+1)*TILE_WIDTH, down, 3);
    }
  }
}

void MapCollisionComponent::add_collision(int axis, int lower, int upper, direction_t dir, int type)
{
  std::list<collision_t>::iterator it;
  
  switch (dir)
  {
    case up:
      it = up_collisions.begin();
      for (; it!=up_collisions.end(); it++)
      {
        if (it->axis < axis) break;
      }
    
      up_collisions.insert(it, {axis, lower, upper, type});
      
      break;
    case down:
      it = down_collisions.begin();
      for (; it!=down_collisions.end(); it++)
      {
        if (it->axis > axis) break;
      }
    
      down_collisions.insert(it, {axis, lower, upper, type});
      
      break;
    case left:
      it = left_collisions.begin();
      for (; it!=left_collisions.end(); it++)
      {
        if (it->axis < axis) break;
      }
  
      left_collisions.insert(it, {axis, lower, upper, type});
      
      break;
    case right:
      it = right_collisions.begin();
      for (; it!=right_collisions.end(); it++)
      {
        if (it->axis > axis) break;
      }
  
      right_collisions.insert(it, {axis, lower, upper, type});
      
      break;
  }
}

void MapCollisionComponent::detectCollision(Entity& player, Locatable& physics, std::pair<double, double> old_position)
{
  int fixed_x = (int) physics.position.first;
  int fixed_y = (int) physics.position.second;
  int fixed_ox = (int) old_position.first;
  int fixed_oy = (int) old_position.second;
  
  if (fixed_x < fixed_ox)
  {
    for (auto it=left_collisions.begin(); it!=left_collisions.end(); it++)
    {
      if (it->axis > fixed_ox) continue;
      if (it->axis < fixed_x) break;
  
      if ((fixed_oy+physics.size.second > it->lower) && (fixed_oy < it->upper))
      {
        // We have a collision!
        if (it->type == 0)
        {
          physics.position.first = it->axis;
          physics.velocity.first = 0;
        } else if (it->type == 1)
        {
          physics.position.first = GAME_WIDTH-physics.size.first/2;
        } else if (it->type == 2)
        {
          physics.position.first = GAME_WIDTH-physics.size.first/2;
          Game::getInstance().loadMap(map->getLeftMap());
        }
    
        break;
      }
    }
  } else if (fixed_x > fixed_ox)
  {
    for (auto it=right_collisions.begin(); it!=right_collisions.end(); it++)
    {
      if (it->axis < fixed_ox+physics.size.first) continue;
      if (it->axis > fixed_x+physics.size.first) break;
  
      if ((fixed_oy+physics.size.second > it->lower) && (fixed_oy < it->upper))
      {
        // We have a collision!
        if (it->type == 0)
        {
          physics.position.first = it->axis - physics.size.first;
          physics.velocity.first = 0;
        } else if (it->type == 1)
        {
          physics.position.first = -physics.size.first/2;
        } else if (it->type == 2)
        {
          physics.position.first = -physics.size.first/2;
          Game::getInstance().loadMap(map->getRightMap());
        } else if (it->type == 3)
        {
          physics.position.first = it->axis - physics.size.first;
          
          message_t msg;
          msg.type = CM_WALK_LEFT;
          player.send(msg);
        }
    
        break;
      }
    }
  }
  
  fixed_x = (int) physics.position.first;
  fixed_y = (int) physics.position.second;
  
  if (fixed_y < fixed_oy)
  {
    for (auto it=up_collisions.begin(); it!=up_collisions.end(); it++)
    {
      if (it->axis > fixed_oy) continue;
      if (it->axis < fixed_y) break;
  
      if ((fixed_x+physics.size.first > it->lower) && (fixed_x < it->upper))
      {
        // We have a collision!
        if (it->type == 0)
        {
          physics.position.second = it->axis;
          physics.velocity.second = 0;
        } else if (it->type == 1)
        {
          physics.position.second = GAME_HEIGHT-physics.size.second/2-1;
        }
    
        break;
      }
    }
  } else if (fixed_y > fixed_oy)
  {
    for (auto it=down_collisions.begin(); it!=down_collisions.end(); it++)
    {
      if (it->axis < fixed_oy+physics.size.second) continue;
      if (it->axis > fixed_y+physics.size.second) break;

      if ((fixed_x+physics.size.first > it->lower) && (fixed_x < it->upper))
      {
        // We have a collision!
        if (it->type == 0)
        {
          physics.position.second = it->axis - physics.size.second;
          physics.velocity.second = 0;
          //mob->onGround = true;
        } else if (it->type == 1)
        {
          physics.position.second = -physics.size.second/2;
        } else if (it->type == 3)
        {
          message_t msg;
          msg.type = CM_DROP;
          msg.dropAxis = it->axis;
          
          player.send(msg);
        }
        
        break;
      }
    }
  }
}

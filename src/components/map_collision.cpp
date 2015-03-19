#include "map_collision.h"
#include "map.h"
#include "game.h"
#include "consts.h"

MapCollisionComponent::MapCollisionComponent(const Map& map) : map(map)
{
  addCollision(-6, 0, MAP_HEIGHT*TILE_HEIGHT, Direction::left, collisionFromMoveType(map.getAdjacent(Map::MoveDir::Left).type));
  addCollision(GAME_WIDTH+6, 0, MAP_HEIGHT*TILE_HEIGHT, Direction::right, collisionFromMoveType(map.getAdjacent(Map::MoveDir::Right).type));
  addCollision(-6, 0, GAME_WIDTH, Direction::up, collisionFromMoveType(map.getAdjacent(Map::MoveDir::Up).type));
  addCollision(MAP_HEIGHT*TILE_HEIGHT+6, 0, GAME_WIDTH, Direction::down, collisionFromMoveType(map.getAdjacent(Map::MoveDir::Down).type));
  
  for (int i=0; i<MAP_WIDTH*MAP_HEIGHT; i++)
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

void MapCollisionComponent::addCollision(double axis, double lower, double
   upper, Direction dir, Collision::Type type)
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
  if (collider.position.first < old_position.first)
  {
    for (auto collision : left_collisions)
    {
      if (collision.axis > old_position.first) continue;
      if (collision.axis < collider.position.first) break;
  
      if ((old_position.second+collider.size.second > collision.lower) && (old_position.second < collision.upper))
      {
        // We have a collision!
        processCollision(game, collider, collision, Direction::left, old_position);
    
        break;
      }
    }
  } else if (collider.position.first > old_position.first)
  {
    for (auto collision : right_collisions)
    {
      if (collision.axis < old_position.first+collider.size.first) continue;
      if (collision.axis > collider.position.first+collider.size.first) break;
  
      if ((old_position.second+collider.size.second > collision.lower) && (old_position.second < collision.upper))
      {
        // We have a collision!
        processCollision(game, collider, collision, Direction::right, old_position);
    
        break;
      }
    }
  }
  
  if (collider.position.second < old_position.second)
  {
    for (auto collision : up_collisions)
    {
      if (collision.axis > old_position.second) continue;
      if (collision.axis < collider.position.second) break;
  
      if ((collider.position.first+collider.size.first > collision.lower) && (collider.position.first < collision.upper))
      {
        // We have a collision!
        processCollision(game, collider, collision, Direction::up, old_position);
    
        break;
      }
    }
  } else if (collider.position.second > old_position.second)
  {
    for (auto collision : down_collisions)
    {
      if (collision.axis < old_position.second+collider.size.second) continue;
      if (collision.axis > collider.position.second+collider.size.second) break;

      if ((collider.position.first+collider.size.first > collision.lower) && (collider.position.first < collision.upper))
      {
        // We have a collision!
        processCollision(game, collider, collision, Direction::down, old_position);
        
        break;
      }
    }
  }
}

void MapCollisionComponent::processCollision(Game& game, Entity& collider, Collision collision, Direction dir, std::pair<double, double> old_position)
{
  if (collision.type == Collision::Type::wall)
  {
    if (dir == Direction::left)
    {
      collider.position.first = collision.axis;
      
      Message msg(Message::Type::setHorizontalVelocity);
      msg.velocity = 0.0;
      collider.send(game, msg);
    } else if (dir == Direction::right)
    {
      collider.position.first = collision.axis - collider.size.first;

      Message msg(Message::Type::setHorizontalVelocity);
      msg.velocity = 0.0;
      collider.send(game, msg);
    } else if (dir == Direction::up)
    {
      collider.position.second = collision.axis;

      Message msg(Message::Type::setVerticalVelocity);
      msg.velocity = 0.0;
      collider.send(game, msg);
    } else if (dir == Direction::down)
    {
      collider.position.second = collision.axis - collider.size.second;
      collider.send(game, Message::Type::hitTheGround);
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
      game.loadMap(game.getWorld().getMap(map.getAdjacent(Map::MoveDir::Left).map), std::make_pair(GAME_WIDTH-collider.size.first/2, old_position.second));
    } else if (dir == Direction::right)
    {
      game.loadMap(game.getWorld().getMap(map.getAdjacent(Map::MoveDir::Right).map), std::make_pair(-collider.size.first/2, old_position.second));
    } else if (dir == Direction::up)
    {
      game.loadMap(game.getWorld().getMap(map.getAdjacent(Map::MoveDir::Up).map), std::make_pair(old_position.first, MAP_HEIGHT*TILE_HEIGHT-collider.size.second/2));
    } else if (dir == Direction::down)
    {
      game.loadMap(game.getWorld().getMap(map.getAdjacent(Map::MoveDir::Down).map), std::make_pair(old_position.first, -collider.size.second/2));
    }
  } else if (collision.type == Collision::Type::reverse)
  {
    // TODO reverse
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
}

MapCollisionComponent::Collision::Type MapCollisionComponent::collisionFromMoveType(Map::MoveType type)
{
  switch (type)
  {
    case Map::MoveType::Wall: return Collision::Type::wall;
    case Map::MoveType::Wrap: return Collision::Type::wrap;
    case Map::MoveType::Warp: return Collision::Type::teleport;
    case Map::MoveType::ReverseWarp: return Collision::Type::reverse;
  }
}

#include "user_movement.h"
#include "renderer.h"

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

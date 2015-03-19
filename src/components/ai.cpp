#include "ai.h"
#include <cstdlib>
#include "entity.h"

void AIActionContainer::addAction(std::shared_ptr<AIAction> action)
{
  actions.push_back(action);
}

void AIActionContainer::start(Game& game, Entity& entity)
{
  currentAction = begin(actions);
  
  if (currentAction != end(actions))
  {
    (*currentAction)->start(game, entity);
  }
}

void AIActionContainer::perform(Game& game, Entity& entity, double dt)
{
  if (!isDone())
  {
    (*currentAction)->perform(game, entity, dt);
  
    if ((*currentAction)->isDone())
    {
      currentAction++;
      
      if (!isDone())
      {
        (*currentAction)->start(game, entity);
      }
    }
  }
}

bool AIActionContainer::isDone() const
{
  return currentAction == end(actions);
}

AI::AI(int chance)
{
  this->chance = chance;
}

int AI::getChance() const
{
  return chance;
}

AI& AIComponent::emplaceAI(int chance)
{
  maxChance += chance;
  ais.emplace_back(chance);
  
  return ais.back();
}

void AIComponent::tick(Game& game, Entity& entity, double dt)
{
  if (currentAI == nullptr)
  {
    int toChoose = rand() % maxChance;
    for (auto& ai : ais)
    {
      if (toChoose < ai.getChance())
      {
        currentAI = &ai;
        break;
      } else {
        toChoose -= ai.getChance();
      }
    }
    
    if (currentAI != nullptr)
    {
      currentAI->start(game, entity);
    }
  }
  
  if (currentAI != nullptr)
  {
    currentAI->perform(game, entity, dt);
  
    if (currentAI->isDone())
    {
      currentAI = nullptr;
    }
  }
}

MoveAIAction::MoveAIAction(Direction dir, int len, int speed)
{
  this->dir = dir;
  this->len = len;
  this->speed = speed;
}

void MoveAIAction::start(Game& game, Entity& entity)
{
  remaining = len;
}

void MoveAIAction::perform(Game&, Entity& entity, double dt)
{
  double dist = dt * speed;
  remaining -= dist;
  
  switch (dir)
  {
    case Direction::Left:
    {
      entity.position.first -= dist;
      break;
    }
    
    case Direction::Right:
    {
      entity.position.first += dist;
      break;
    }
    
    case Direction::Up:
    {
      entity.position.second -= dist;
      break;
    }
    
    case Direction::Down:
    {
      entity.position.second += dist;
      break;
    }
  }
}

bool MoveAIAction::isDone() const
{
  return remaining <= 0.0;
}

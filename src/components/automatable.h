#ifndef AUTOMATABLE_H_3D519131
#define AUTOMATABLE_H_3D519131

#include "component.h"
#include <vector>
#include <random>
#include "vector.h"

class AutomatableComponent : public Component {
public:

  /**
   * Helper class that defines an automatable action.
   */
  class Action {
  public:

    /**
     * The horizontal and vertical speed, in pixels/sec, that the entity should
     * move at.
     */
    vec2d speed;

    /**
     * The duration of the action in seconds.
     */
    double dur;
  };

  /**
   * Helper type that defines a behavior that an entity can exhibit, which is a
   * list of actions that are stepped through in sequence.
   */
  using Behavior = std::vector<Action>;

  /**
   * A group of behaviors that the entity can exhibit, which are picked at
   * random at the start of automation and whenever a behavior completes.
   *
   * @managed_by RealizingSystem
   */
  std::vector<Behavior> behaviors;

  /**
   * A random distribution over the above behaviors.
   *
   * @managed_by RealizingSystem
   */
  std::discrete_distribution<size_t> behaviorDist;

  /**
   * A flag indicating whether a behavior is currently executing.
   *
   * @managed_by AutomatingSystem
   */
  bool behaviorRunning = false;

  /**
   * A flag indicating whether an action is currently executing.
   *
   * @managed_by AutomatingSystem
   */
  bool actionRunning = false;

  /**
   * The index of the currently executing behavior, if there is one.
   *
   * @managed_by AutomatingSystem
   */
  size_t currentBehavior;

  /**
   * The index of the currently executing action, if there is one.
   *
   * @managed_by AutomatingSystem
   */
  size_t currentAction;

  /**
   * The amount of time remaining, in seconds, of the currently executing
   * action.
   *
   * @managed_by AutomatingSystem
   */
  double remaining;

  /**
   * If this flag is disabled, the entity will be ignored by the automating
   * system.
   *
   * @managed_by RealizingSystem
   */
  bool active = false;
};

#endif /* end of include guard: AUTOMATABLE_H_3D519131 */

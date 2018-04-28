#ifndef PLAYABLE_H_DDC566C3
#define PLAYABLE_H_DDC566C3

#include "component.h"
#include "entity_manager.h"

class PlayableComponent : public Component {
public:

  using id_type = EntityManager::id_type;

  /**
   * The entity ID of the map that the player is on.
   *
   * @managed_by PlayingSystem
   */
  id_type mapId;

  /**
   * The map ID and coordinates of the location that the player will spawn after
   * dying. Note that the map ID here is a world description map ID, not an
   * entity ID.
   *
   * @managed_by PlayingSystem
   */
  size_t checkpointMapId;
  double checkpointX;
  double checkpointY;

};

#endif /* end of include guard: PLAYABLE_H_DDC566C3 */

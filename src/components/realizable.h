#ifndef REALIZABLE_H_36D8D71E
#define REALIZABLE_H_36D8D71E

#include "component.h"
#include <set>
#include <map>
#include "entity_manager.h"

class RealizableComponent : public Component {
public:

  using id_type = EntityManager::id_type;

  /**
   * Path to the XML file containing the world definition.
   *
   * @managed_by RealizingSystem
   */
  std::string worldFile;

  /**
   * Starting map and player location for a new game.
   *
   * @managed_by RealizingSystem
   */
  int startingMapId;
  int startingX;
  int startingY;

  /**
   * The set of map entities loaded by this entity. It is only intended for
   * there to be one realizable entity, so this should contain all loaded maps.
   * The realizable entity has ownership of the loaded maps.
   *
   * @managed_by RealizingSystem
   */
  std::set<id_type> maps;

  /**
   * A lookup table that translates a map ID to the entity representing that
   * loaded map.
   *
   * @managed_by RealizingSystem
   */
  std::map<size_t, id_type> entityByMapId;

  /**
   * The entity ID of the currently active map.
   *
   * @managed_by RealizingSystem
   */
  id_type activeMap;

  /**
   * Whether or not a map has been activated yet.
   *
   * @managed_by RealizingSystem
   */
  bool hasActiveMap = false;

  /**
   * The entity ID of the currently active player.
   */
  id_type activePlayer;
};

#endif /* end of include guard: REALIZABLE_H_36D8D71E */

#ifndef REALIZING_H_6853748C
#define REALIZING_H_6853748C

#include <string>
#include <map>
#include "system.h"
#include "vector.h"

class RealizingSystem : public System {
public:

  /**
   * Constructs the realizing system.
   *
   * Note that this must be constructed after the following system:
   * - Mapping
   * - Animating
   * - Pondering
   * - Scripting
   */
  RealizingSystem(
    Game& game,
    std::string worldFile,
    std::string prototypeFile);

  id_type getActiveMap() const
  {
    return activeMap_;
  }

  int getStartingMapId() const
  {
    return startingMapId_;
  }

  vec2i getStartingPos() const
  {
    return startingPos_;
  }

  id_type getEntityByMapId(size_t mapId) const
  {
    return entityByMapId_.at(mapId);
  }

  id_type getActivePlayer() const
  {
    return activePlayer_;
  }

  void setActivePlayer(id_type entity)
  {
    activePlayer_ = entity;
  }

  /**
   * Loads the given map.
   */
  void loadMap(id_type mapEntity);

  /**
   * Treats the given entity as part of the active map.
   */
  void enterActiveMap(id_type entity);

  /**
   * Stops treating the given entity as part of the active map.
   */
  void leaveActiveMap(id_type entity);

private:

  void deactivateMap();

  void activateMap(id_type mapEntity);

  std::string worldFile_;
  std::string prototypeFile_;
  int startingMapId_;
  vec2i startingPos_;
  std::map<size_t, id_type> entityByMapId_;
  id_type activeMap_;
  id_type activePlayer_;
};

#endif /* end of include guard: REALIZING_H_6853748C */

#ifndef MAPPING_H_33FC2294
#define MAPPING_H_33FC2294

#include "system.h"

class MappingSystem : public System {
public:

  MappingSystem(Game& game) : System(game)
  {
  }

  void render(Texture& texture);

  void generateBoundaries(id_type mapEntity);

};

#endif /* end of include guard: MAPPING_H_33FC2294 */

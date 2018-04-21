#ifndef PROTOTYPE_MANAGER_H_83CECCA6
#define PROTOTYPE_MANAGER_H_83CECCA6

#include <string>
#include <map>
#include "prototype.h"

class PrototypeManager {
public:

  PrototypeManager(std::string path);

  inline const Prototype& getPrototype(std::string name) const
  {
    return prototypes_.at(name);
  }

private:

  std::map<std::string, Prototype> prototypes_;
};

#endif /* end of include guard: PROTOTYPE_MANAGER_H_83CECCA6 */

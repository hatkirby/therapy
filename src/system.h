#ifndef SYSTEM_H_B61A8CEA
#define SYSTEM_H_B61A8CEA

class EntityManager;

class System {
  public:
    virtual void tick(EntityManager& manager, float dt) = 0;
};

#endif /* end of include guard: SYSTEM_H_B61A8CEA */

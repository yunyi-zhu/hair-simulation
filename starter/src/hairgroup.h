#ifndef HAIR_SIMULATION_HAIRGROUP_H
#define HAIR_SIMULATION_HAIRGROUP_H

#include "hairsystem.h"
#include "timestepper.h"

class HairGroup {
public:
  HairGroup();

  std::vector<HairSystem> hairs;

   void draw(GLProgram& ctx);
   void step(TimeStepper* timeStepper, float h);
};

#endif //HAIR_SIMULATION_HAIRGROUP_H

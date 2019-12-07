#ifndef HAIR_SIMULATION_HAIRGROUP_H
#define HAIR_SIMULATION_HAIRGROUP_H

#include "hairsystem.h"
#include "symhair.h"
#include "timestepper.h"
static int HAIR_LENGTH = 16;

class HairGroup {
public:
  HairGroup();

  std::vector<HairSystem> hairs;
  std::vector<SymHair> symhairs;

  void draw(GLProgram& ctx, VertexRecorder rec);
  void step(TimeStepper* timeStepper, float h);
  void setHairCurve(float l_input);
  void startWind();
private:
  int indexOf(int h, int w);
};

#endif //HAIR_SIMULATION_HAIRGROUP_H

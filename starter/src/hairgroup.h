#ifndef HAIR_SIMULATION_HAIRGROUP_H
#define HAIR_SIMULATION_HAIRGROUP_H

#include "hairsystem.h"
#include "symhair.h"
#include "timestepper.h"
static int HAIR_LENGTH = 16;

static Vector3f CORE_HAIR_COLOR(0.75, 0.52, 0.76);

static void setCoreHairColorPurple() {
	CORE_HAIR_COLOR = Vector3f(0.75, 0.52, 0.76);
	printf("USE");
	CORE_HAIR_COLOR.print();
}

class HairGroup {
public:
  HairGroup();

  std::vector<HairSystem> hairs;
  std::vector<SymHair> symhairs;

  void draw(GLProgram& ctx, VertexRecorder rec);
  void step(TimeStepper* timeStepper, float h);
  void setHairCurve(float l_input);

private:
  int indexOf(int h, int w);
};

#endif //HAIR_SIMULATION_HAIRGROUP_H

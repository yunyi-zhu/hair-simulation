#ifndef HAIR_SIMULATION_HAIRGROUP_H
#define HAIR_SIMULATION_HAIRGROUP_H

#include "hairsystem.h"
#include "symhair.h"
#include "timestepper.h"

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

   void draw(GLProgram& ctx, VertexRecorder curveRec, VertexRecorder surfaceRec);
   void step(TimeStepper* timeStepper, float h);
   void setCoreHairColorPurple(Vector3f Purple);
   
private:
  int indexOf(int h, int w);
};

#endif //HAIR_SIMULATION_HAIRGROUP_H

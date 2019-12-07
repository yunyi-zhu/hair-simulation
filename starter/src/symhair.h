#ifndef HAIR_SIMULATION_SYMHAIR_H
#define HAIR_SIMULATION_SYMHAIR_H

#include <vector>
#include <vecmath.h>
#include "hairsystem.h"
#include "vertexrecorder.h"

using namespace std;

class SymHair {
public:
  SymHair(Vector3f origin, vector<HairSystem *> hairs, vector<float> weights);
  void draw(GLProgram& ctx, VertexRecorder curveRec, VertexRecorder surfaceRec);

private:
  Vector3f origin;
  vector<HairSystem*> hairs;
  vector<float> weights;
};


#endif //HAIR_SIMULATION_SYMHAIR_H

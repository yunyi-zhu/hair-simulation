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
  void setHairColor(float r, float g, float b);

private:
  Vector3f origin;
  vector<HairSystem*> hairs;
  vector<float> weights;
  Vector3f hairColor;
};


#endif //HAIR_SIMULATION_SYMHAIR_H

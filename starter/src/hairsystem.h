#ifndef HAIRSYSTEM_H
#define HAIRSYSTEM_H

#include <vector>
#include "particlesystem.h"
#include "vertexrecorder.h"

const float GRAVITY = 9.8f;
const float K_DRAG = 0.015f;
const float M = 0.01f;
const float COLLISION_RES = 1000.0f;

const float UNIT_H = 0.5f;
const float HORI_DELTA = 0.3f;
const float VERTI_DELTA = 0.2f;

const float CORE_L = UNIT_H;
const float SUPPORT_L = 0.5 * UNIT_H;

const float CORE_K = 60.0f;
const float SUPPORT_K = 30.0f;
const float SUPPORT_L_3 = 3 * UNIT_H;

class HairSystem : public ParticleSystem
{
public:
  HairSystem() { /* puppet */ };
  HairSystem(Vector3f origin, int length);

  // evalF is called by the integrator at least once per time step
  std::vector<Vector3f> evalF(std::vector<Vector3f> state) override;

  // draw is called once per frame
  void draw(GLProgram& ctx, VertexRecorder curveRec, VertexRecorder surfaceRec);

  void setHairCurve(float l_input);
  void toggleWind();
  void setWindStrength(float strength);
  void setWindDirection(float index);

  // inherits
//   std::vector<Vector3f> m_vVecState;
private:
  // hair length: number of layers
  int H;
  // private variables
  // Spring:
  //  1. Index of first node
  //  2. Index of second node
  //  3. Rest Length
  //  4. K
  std::vector<Vector4f> springs;
  // the indices of the points where they are fixed
  std::vector<int> fixedPtIndex;
  // whether the wind should be blowing
  bool windBlowing;
  Vector3f windDirection;
  float windStrength;
};


#endif

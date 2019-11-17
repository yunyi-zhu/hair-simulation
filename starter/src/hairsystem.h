#ifndef HAIRSYSTEM_H
#define HAIRSYSTEM_H

#include <vector>
#include "particlesystem.h"

class HairSystem : public ParticleSystem
{
public:
  HairSystem();

  // evalF is called by the integrator at least once per time step
  std::vector<Vector3f> evalF(std::vector<Vector3f> state) override;

  // draw is called once per frame
  void draw(GLProgram& ctx);

  // get index
  int indexOf(int i, int j);
  int coreIndexOf(int i);

  // inherits
  // std::vector<Vector3f> m_vVecState;
private:
  // private variables
  // Spring:
  //  1. Index of first node
  //  2. Index of second node
  //  3. Rest Length
  //  4. K
  std::vector<Vector4f> springs;
  // the indices of the points where they are fixed
  std::vector<int> fixedPtIndex;
};


#endif

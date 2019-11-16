#ifndef PENDULUMSYSTEM_H
#define PENDULUMSYSTEM_H

#include <vector>

#include "particlesystem.h"

class PendulumSystem : public ParticleSystem
{
public:
    PendulumSystem();

    std::vector<Vector3f> evalF(std::vector<Vector3f> state) override;
    void draw(GLProgram&);
    void addWireframe();

    // inherits 
    // std::vector<Vector3f> m_vVecState;

private:
	// each spring stores (p1, p2, rest length, stiffness)
	std::vector<Vector4f> springs;
	bool showWireframe;
};

#endif

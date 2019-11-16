#ifndef CLOTHSYSTEM_H
#define CLOTHSYSTEM_H

#include <vector>

#include "particlesystem.h"

class ClothSystem : public ParticleSystem
{
    ///ADD MORE FUNCTION AND FIELDS HERE
public:
    ClothSystem();

    // evalF is called by the integrator at least once per time step
    std::vector<Vector3f> evalF(std::vector<Vector3f> state) override;

    // draw is called once per frame
    void draw(GLProgram& ctx);

    // use smooth shading
    void drawSmooth();

    // get index
    int indexOf(int i, int j);

    void addBreeze();
    void addWireframe();
    void addSmooth();

    void moveDown();
    void moveUp();
    void moveLeft();
    void moveRight();

    // inherits
    // std::vector<Vector3f> m_vVecState;

private:
	std::vector<int> fixedPtInd;
	std::vector<Vector4f> springs;
	bool breeze;
	bool showWireframe;
	bool smooth;
};


#endif

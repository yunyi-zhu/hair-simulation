#include "pendulumsystem.h"

#include <cassert>
#include "camera.h"
#include "vertexrecorder.h"
#include <string>
#include <iostream>

// TODO adjust to number of particles.
const int NUM_PARTICLES = 4;
const float m = 1;
const float kd = 0.5;
using namespace std;

PendulumSystem::PendulumSystem()
{
    showWireframe = true;
    // TODO 4.2 Add particles for simple pendulum
    // TODO 4.3 Extend to multiple particles

    // To add a bit of randomness, use e.g.
    // float f = rand_uniform(-0.5f, 0.5f);
    // in your initial conditions.

    // even index position, odd velocity
    m_vVecState.push_back(Vector3f(0,1,0));
    m_vVecState.push_back(Vector3f::ZERO);
    for (int i = 0; i < NUM_PARTICLES; i++) {
        float f = rand_uniform(-0.5f, 0.5f);
        m_vVecState.push_back(Vector3f(0, f, 0));
        m_vVecState.push_back(Vector3f::ZERO);
    }
    // add springs
    // springs.push_back(Vector4f(0, 1, 3, 10));
    // springs.push_back(Vector4f(1, 2, 0.3, 10));
    // springs.push_back(Vector4f(2, 3, 0.5, 6));
    springs.push_back(Vector4f(0, 1, 0.3, 10));
    springs.push_back(Vector4f(0, 2, 2.5, 10));
    springs.push_back(Vector4f(0, 3, 2.8, 10));
    springs.push_back(Vector4f(0, 4, 3, 10));
    springs.push_back(Vector4f(1, 2, 1.9, 10));
    springs.push_back(Vector4f(1, 3, 2.4, 10));
    // springs.push_back(Vector4f(1, 4, 0.2, 10));
    // springs.push_back(Vector4f(2, 3, 0.2, 10));
    // springs.push_back(Vector4f(2, 4, 0.2, 10));
    // springs.push_back(Vector4f(3, 4, 0.2, 10));
}

vector<Vector3f> PendulumSystem::evalF(std::vector<Vector3f> state)
{
    vector<Vector3f> f;
    // TODO 4.1: implement evalF
    //  - gravity
    //  - viscous drag
    //  - springs

    // fix the first particle
    f.push_back(Vector3f::ZERO);
    f.push_back(Vector3f::ZERO);

    for (int i = 1; i < state.size() / 2; i++) {
        f.push_back(state[2*i + 1]);
        Vector3f gravity(0.0, - 9.8, 0.0);
        Vector3f drag = - kd * state[2*i + 1] / m;
        f.push_back(gravity + drag);
    }

    // Spring forces (p1, p2, rest length, stiffness)
    for (int i = 0; i < springs.size(); i++) {
        float restLen = springs[i][2];
        float stiff = springs[i][3];
        int ind1 = (int) springs[i][0];
        int ind2 = (int) springs[i][1];
        // d and ||d|| in the formula
        Vector3f d = state[2 * ind1] - state[2 * ind2];
        Vector3f springF = - stiff * (d.abs() - restLen) * d.normalized();
        f[2 * ind1 + 1] += springF / m;
        f[2 * ind2 + 1] -= springF / m;
    }

    // fix the first particle
    f[1] = Vector3f::ZERO;

    return f;
}

void PendulumSystem::addWireframe()
{
    showWireframe = !showWireframe;
};

// render the system (ie draw the particles)
void PendulumSystem::draw(GLProgram& gl)
{
    const Vector3f PENDULUM_COLOR(0.73f, 0.0f, 0.83f);
    gl.updateMaterial(PENDULUM_COLOR);
    // TODO 4.2, 4.3
    vector<Vector3f> state = getState();
    for (int i = 0; i < state.size() - 1; i += 2) {
        gl.updateModelMatrix(Matrix4f::translation(state[i]));
        drawSphere(0.075f, 10, 10);
    }

    if (showWireframe) {
        const Vector3f CLOTH_COLOR(0.9f, 0.9f, 0.9f);
        gl.updateMaterial(CLOTH_COLOR);
        gl.disableLighting();
        gl.updateModelMatrix(Matrix4f::identity()); // update uniforms after mode change
        VertexRecorder rec;
        // draw springs
        for (int i = 0; i < springs.size(); i += 1) {
            Vector4f sp = springs[i];
            rec.record(state[2 * sp[0]], CLOTH_COLOR);
            rec.record(state[2 * sp[1]], CLOTH_COLOR);
        }
        glLineWidth(3.0f);
        rec.draw(GL_LINES);
        gl.enableLighting();
    }
}

#include "timestepper.h"

#include <cstdio>

void ForwardEuler::takeStep(ParticleSystem* particleSystem, float stepSize)
{
    // cout << "use Euler" << endl;
    std::vector<Vector3f> oldState = particleSystem -> getState();
    std::vector<Vector3f> f = particleSystem -> evalF(oldState);
    std::vector<Vector3f> newState;
    for (int i = 0; i < oldState.size(); i++) {
	   newState.push_back(oldState[i] + stepSize * f[i]);
    }
    particleSystem -> setState(newState);
}

void Trapezoidal::takeStep(ParticleSystem* particleSystem, float stepSize)
{
    // cout << "use Trapezoidal" << endl;
	std::vector<Vector3f> oldState = particleSystem -> getState();
	std::vector<Vector3f> f0 = particleSystem -> evalF(oldState);
	std::vector<Vector3f> state1;
    for (int i = 0; i < oldState.size(); i++) {
	   state1.push_back(oldState[i] + stepSize * f0[i]);
    }
    std::vector<Vector3f> f1 = particleSystem -> evalF(state1);

    std::vector<Vector3f> newState;
    for (int i = 0; i < oldState.size(); i++) {
	   newState.push_back(oldState[i] + 0.5 * stepSize * (f0[i] + f1[i]));
    }
    particleSystem -> setState(newState);
}


void RK4::takeStep(ParticleSystem* particleSystem, float stepSize)
{
    std::vector<Vector3f> oldState = particleSystem -> getState();
    std::vector<Vector3f> k1 = particleSystem -> evalF(oldState);
    std::vector<Vector3f> s2;
    for (int i = 0; i < oldState.size(); i++) {
       s2.push_back(oldState[i] + 0.5 * stepSize * k1[i]);
    }
    std::vector<Vector3f> k2 = particleSystem -> evalF(s2);

    std::vector<Vector3f> s3;
    for (int i = 0; i < oldState.size(); i++) {
       s3.push_back(oldState[i] + 0.5 * stepSize * k2[i]);
    }
    std::vector<Vector3f> k3 = particleSystem -> evalF(s3);

    std::vector<Vector3f> s4;
    for (int i = 0; i < oldState.size(); i++) {
       s4.push_back(oldState[i] + stepSize * k3[i]);
    }
    std::vector<Vector3f> k4 = particleSystem -> evalF(s4);

    std::vector<Vector3f> newState;
    for (int i = 0; i < oldState.size(); i++) {
       newState.push_back(oldState[i] + (1.0 / 6.0) * stepSize * (k1[i] + 2 * k2[i] + 2 * k3[i] + k4[i]));
    }
    particleSystem -> setState(newState);
}


#include "hairgroup.h"
#include "hairsystem.h"
#include "camera.h"
#include "vertexrecorder.h"
#include <string>
#include <iostream>

using namespace std;

HairGroup::HairGroup() {
  hairs.push_back(HairSystem(Vector3f(0,0,0), 10));
  hairs.push_back(HairSystem(Vector3f(1,0,0), 20));
  hairs.push_back(HairSystem(Vector3f(0,1,0), 30));
  hairs.push_back(HairSystem(Vector3f(0,0,1), 3));
}

void HairGroup::draw(GLProgram& gl) {
  for (int i = 0; i < hairs.size(); i++) {
    hairs[i].draw(gl);
  }
}

void HairGroup::step(TimeStepper* timeStepper, float h) {
  for (int i = 0; i < hairs.size(); i++) {
    timeStepper->takeStep(&hairs[i], h);
  }
}

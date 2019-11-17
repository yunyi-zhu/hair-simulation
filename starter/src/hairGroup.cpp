#include "hairGroup.h"
#include "hairsystem.h"
#include "camera.h"
#include "vertexrecorder.h"
#include <string>
#include <iostream>

using namespace std;

HairGroup::HairGroup() {
  hairRoots.push_back(Vector3f(0,0,0));

  hairs.push_back(HairSystem());
}

void HairGroup::draw(GLProgram& gl) {
  for (int i = 0; i < hairRoots.size(); i++) {
    hairs[i].draw(gl);
  }
}

void HairGroup::step(TimeStepper* timeStepper, float h) {
  for (int i = 0; i < hairRoots.size(); i++) {
    timeStepper->takeStep(&hairs[i], h);
  }
}

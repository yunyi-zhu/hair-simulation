#include "hairgroup.h"
#include "hairsystem.h"
#include "camera.h"
#include "vertexrecorder.h"
#include <string>
#include <iostream>

using namespace std;


const float HEAD_R = 1.0f; // head radius
const int DENSITY_H = 6; // number of hairs each round
const int DENSITY_V = 1; // number of rounds

HairGroup::HairGroup() {

  for (int i = 0; i < DENSITY_V; i++) {
    for (int j = 0; j < DENSITY_H; j++) {
      float offset = 0;
      float lat = ( M_PI / 4.0f ) / DENSITY_V * i;
      float lon = ( M_PI ) / DENSITY_H * j + offset;

      float x = HEAD_R * cos(lat) * cos(lon);
      float y = HEAD_R * sin(lat);
      float z = HEAD_R * cos(lat) * sin(lon);

      hairs.push_back(HairSystem(Vector3f(x, y, z), 16));
    }
  }
}

void HairGroup::draw(GLProgram& gl) {
  for (int i = 0; i < hairs.size(); i++) {
    hairs[i].draw(gl);
  }
  gl.updateModelMatrix(Matrix4f::identity());
//  drawSphere(HEAD_R, 30, 30);
}

void HairGroup::step(TimeStepper* timeStepper, float h) {
  for (int i = 0; i < hairs.size(); i++) {
    timeStepper->takeStep(&hairs[i], h);
  }
}

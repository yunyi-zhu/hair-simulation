#include "hairgroup.h"
#include "hairsystem.h"
#include "camera.h"
#include "vertexrecorder.h"
#include "symhair.h"
#include <string>
#include <iostream>

using namespace std;

const float HEAD_R = 1.0f; // head radius
const int DENSITY_H = 6; // number of hairs each round
const int DENSITY_V = 4; // number of rounds
const int DENSITY_SYM = 5; // number of symhairs between two hairs
const float LAT_OFFSET = 0.1; // more realistic look of top hair

HairGroup::HairGroup() {
  for (int i = 0; i < DENSITY_V; i++) {
    for (int j = 0; j < DENSITY_H; j++) {
      float lat = ( M_PI / 2.0f ) / DENSITY_V * (i + 1) - LAT_OFFSET;
      float lon = ( M_PI ) / DENSITY_H * j;

      float x = HEAD_R * cos(lat) * cos(lon);
      float y = HEAD_R * sin(lat);
      float z = HEAD_R * cos(lat) * sin(lon);

      hairs.push_back(HairSystem(Vector3f(x, y, z), 16));
    }
  }

  // add symhairs
  for (int i = 0; i < DENSITY_V; i++) {
    for (int j = 0; j < DENSITY_H; j++) {
      if (i != DENSITY_V-1) {
        for (int k = 1; k < DENSITY_SYM; k++) {
          Vector3f origin = Vector3f::ZERO;
          vector<HairSystem*> follow_hairs{&hairs[indexOf(i, j)], &hairs[indexOf(i+1, j)]};
          float weight_right = (1.0 * k) / DENSITY_SYM;
          vector<float> weights{ 1 - weight_right, weight_right };

          symhairs.push_back(SymHair(origin, follow_hairs, weights));
        }
      }

      if (j != DENSITY_H-1) {
        for (int k = 1; k < DENSITY_SYM; k++) {
          Vector3f origin = Vector3f::ZERO;
          vector<HairSystem*> follow_hairs{&hairs[indexOf(i, j)], &hairs[indexOf(i, j+1)]};
          float weight_right = (1.0 * k) / DENSITY_SYM;
          vector<float> weights{ 1 - weight_right, weight_right };

          symhairs.push_back(SymHair(origin, follow_hairs, weights));
        }
      }

      if (i != DENSITY_V-1 && j != DENSITY_H-1) {
        for (int k1 = 1; k1 < DENSITY_SYM; k1++) {
          for (int k2 = 1; k2 < DENSITY_SYM; k2++) {
            Vector3f origin = Vector3f::ZERO;
            vector<HairSystem*> follow_hairs{
              &hairs[indexOf(i, j)],
              &hairs[indexOf(i, j+1)],
              &hairs[indexOf(i+1, j)],
              &hairs[indexOf(i+1, j+1)]
            };
            float weight_right_1 = (1.0 * k1) / DENSITY_SYM;
            float weight_right_2 = (1.0 * k1) / DENSITY_SYM;
            vector<float> weights{
              (1 - weight_right_1) * (1 - weight_right_2),
              weight_right_1 * (1 - weight_right_2),
              (1 - weight_right_1) * weight_right_2,
              weight_right_1 * weight_right_2
            };

            symhairs.push_back(SymHair(origin, follow_hairs, weights));
          }
        }
      }
    }
  }
}

void HairGroup::draw(GLProgram& gl) {
  gl.updateModelMatrix(Matrix4f::identity());
  for (int i = 0; i < hairs.size(); i++) {
    hairs[i].draw(gl);
  }

  for (int i = 0; i < symhairs.size(); i++) {
    symhairs[i].draw(gl);
  }

  gl.updateModelMatrix(Matrix4f::identity());
  drawSphere(HEAD_R, 30, 30);
}

void HairGroup::step(TimeStepper* timeStepper, float h) {
  for (int i = 0; i < hairs.size(); i++) {
    timeStepper->takeStep(&hairs[i], h);
  }
}

int HairGroup::indexOf(int h, int w) {
  return h * DENSITY_H + w;
}
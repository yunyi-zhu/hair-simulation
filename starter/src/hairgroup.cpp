#include "hairgroup.h"
#include "hairsystem.h"
#include "camera.h"
#include "vertexrecorder.h"
#include "symhair.h"
#include <string>
#include <iostream>

using namespace std;

const float HEAD_R = 1.0f; // head radius
const int DENSITY_H = 4; // number of hairs each round
const int DENSITY_V = 3; // number of rounds
const int DENSITY_SYM = 0; // number of symhairs between two hairs
const float LAT_OFFSET = 0.05; // more realistic look of top hair

static Vector3f positionFromLatLon(float lat, float lon) {
  float x = HEAD_R * cos(lat) * cos(lon);
  float y = HEAD_R * sin(lat);
  float z = HEAD_R * cos(lat) * sin(lon);
  return Vector3f(x, y, z);
}

HairGroup::HairGroup() {
  vector<float> lats;
  vector<float> lons;

  for (int i = 0; i < DENSITY_V; i++) {
    for (int j = 0; j < DENSITY_H; j++) {
      float lat = ( M_PI / 2.0f ) / DENSITY_V * (i + 1) - LAT_OFFSET;
      float lon = ( M_PI ) / DENSITY_H * j;

      hairs.push_back(HairSystem(positionFromLatLon(lat, lon), HAIR_LENGTH));
      lats.push_back(lat);
      lons.push_back(lon);
    }
  }

  // hair interpolation
  for (int i = 0; i < DENSITY_V; i++) {
    for (int j = 0; j < DENSITY_H; j++) {
      if (i != DENSITY_V-1) {
        for (int k = 1; k < DENSITY_SYM; k++) {
          vector<HairSystem*> follow_hairs{&hairs[indexOf(i, j)], &hairs[indexOf(i+1, j)]};
          float weight_right = (1.0 * k) / DENSITY_SYM;
          vector<float> weights{ 1 - weight_right, weight_right };

          float lat = lats[indexOf(i, j)] * ( 1 - weight_right ) + lats[indexOf(i+1, j)] * weight_right;
          float lon = lons[indexOf(i, j)] * ( 1 - weight_right ) + lons[indexOf(i+1, j)] * weight_right;
          Vector3f origin = positionFromLatLon(lat, lon);

          symhairs.push_back(SymHair(origin, follow_hairs, weights));
        }
      }

      if (j != DENSITY_H-1) {
        for (int k = 1; k < DENSITY_SYM; k++) {
          vector<HairSystem*> follow_hairs{&hairs[indexOf(i, j)], &hairs[indexOf(i, j+1)]};
          float weight_right = (1.0 * k) / DENSITY_SYM;
          vector<float> weights{ 1 - weight_right, weight_right };

          float lat = lats[indexOf(i, j)] * ( 1 - weight_right ) + lats[indexOf(i, j+1)] * weight_right;
          float lon = lons[indexOf(i, j)] * ( 1 - weight_right ) + lons[indexOf(i, j+1)] * weight_right;
          Vector3f origin = positionFromLatLon(lat, lon);

          symhairs.push_back(SymHair(origin, follow_hairs, weights));
        }
      }

      if (i != DENSITY_V-1 && j != DENSITY_H-1) {
        for (int k1 = 1; k1 < DENSITY_SYM; k1++) {
          for (int k2 = 1; k2 < DENSITY_SYM; k2++) {
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

            float lat = weights[0] * lats[indexOf(i, j)] +
                    weights[1] * lats[indexOf(i, j+1)] +
                    weights[2] * lats[indexOf(i+1, j)] +
                    weights[3] * lats[indexOf(i+1, j+1)];

            float lon = weights[0] * lons[indexOf(i, j)] +
                        weights[1] * lons[indexOf(i, j+1)] +
                        weights[2] * lons[indexOf(i+1, j)] +
                        weights[3] * lons[indexOf(i+1, j+1)];
            Vector3f origin = positionFromLatLon(lat, lon);

            symhairs.push_back(SymHair(origin, follow_hairs, weights));
          }
        }
      }
    }
  }
}

void HairGroup::draw(GLProgram& gl, VertexRecorder rec) {
  gl.updateModelMatrix(Matrix4f::identity());
  int hairs_size = hairs.size();
  int symhairs_size = symhairs.size();
  for (int i = 0; i < hairs_size; i++) {
    hairs[i].draw(gl, rec);
  }

  for (int i = 0; i < symhairs_size; i++) {
    symhairs[i].draw(gl, rec);
  }

  gl.enableLighting();
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  const Vector3f HEAD_COLOR(0.8f, 0.8f, 0.8f);
  gl.updateMaterial(HEAD_COLOR);
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

void HairGroup::setHairCurve(float l_input) {
  int H = hairs.size();
  for (int i = 0; i < H; i++) {
    hairs[i].setHairCurve(l_input);
  }
}

void HairGroup::startWind(){
  int H = hairs.size();
  for (int i = 0; i < H; i++) {
    hairs[i].startWind();
  }
}
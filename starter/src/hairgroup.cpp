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
      float lon = - M_PI * 2 / 3 + ( M_PI ) / DENSITY_H * j;

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

  windBlowing = false;
}

void HairGroup::draw(GLProgram& gl, VertexRecorder curveRec, VertexRecorder surfaceRec) {
  const Vector3f HAIR_COLOR(1.0f, 1.0f, 1.0f);
  // cout << CORE_HAIR_COLOR[0] << endl;
  gl.updateMaterial(CORE_HAIR_COLOR);

  for (int i = 0; i < hairs.size(); i++) {
    hairs[i].draw(gl, curveRec, surfaceRec);
  }

  gl.updateMaterial(CORE_HAIR_COLOR);
  for (int i = 0; i < symhairs.size(); i++) {
    symhairs[i].draw(gl, curveRec, surfaceRec);
  }

  gl.enableLighting();
  gl.updateModelMatrix(Matrix4f::identity());
  const Vector3f HEAD_COLOR(0.9f, 0.9f, 0.9f);
  gl.updateMaterial(HEAD_COLOR);
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

void HairGroup::toggleWind(){
  int H = hairs.size();
  for (int i = 0; i < H; i++) {
    hairs[i].toggleWind();
  }
  windBlowing = !windBlowing;
}

void HairGroup::setWindStrength(float strength) {
  int H = hairs.size();
  for (int i = 0; i < H; i++) {
    hairs[i].setWindStrength(strength);
  }
}

void HairGroup::setWindDirection(float index) {
  int H = hairs.size();
  for (int i = 0; i < H; i++) {
    hairs[i].setWindDirection(index);
  }
}

void HairGroup::setHairColor(float r, float g, float b) {
  int H = hairs.size();
  for (int i = 0; i < H; i++) {
    hairs[i].setHairColor(r, g, b);
  }

  int H2 = symhairs.size();
  for (int i = 0; i < H2; i++) {
    symhairs[i].setHairColor(r, g, b);
  }
}
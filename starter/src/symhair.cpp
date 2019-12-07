#include "symhair.h"
#include "hairsystem.h"
#include "camera.h"
#include "vertexrecorder.h"
#include "curve.h"
#include <string>
#include <iostream>

using namespace std;

SymHair::SymHair(Vector3f origin_input,
        vector<HairSystem*> hair_inputs,
        vector<float> weight_inputs) {
  origin = origin_input;
  hairs = hair_inputs;
  weights = weight_inputs;
}

void SymHair::draw(GLProgram& gl) {
  const Vector3f HAIR_COLOR(0.9f, 0.9f, 0.9f);
  gl.updateMaterial(HAIR_COLOR);

  gl.disableLighting();
  gl.updateModelMatrix(Matrix4f::identity());
  VertexRecorder rec;

  vector<Vector3f> points;
  points.push_back(origin);
  points.push_back(origin);
  points.push_back(origin);
  points.push_back(origin);

  int hair_len = (hairs[0]->getState()).size() / 2;
  for (int i = 1; i < hair_len; i++) {
    Vector3f point = Vector3f::ZERO;
    for (int j = 0; j < hairs.size(); j++) {
      point += hairs[j]->getState()[2 * i] * weights[j];
    }
    points.push_back(point);
  }

  Curve curve = evalBspline(points, 10);
  // record curve, can change color
  const Vector3f WHITE(1, 1, 1);
  for (int i = 0; i < (int)curve.size() - 1; ++i) {
    rec.record_poscolor(curve[i].V, WHITE);
    rec.record_poscolor(curve[i + 1].V, WHITE);
  }
  glLineWidth(6.0f);
  rec.draw(GL_LINES);
  gl.enableLighting(); // reset to default lighting model
}
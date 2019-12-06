#include "symhair.h"
#include "hairsystem.h"
#include "camera.h"
#include "vertexrecorder.h"
#include "curve.h"
#include <string>
#include <iostream>

using namespace std;

SymHair::SymHair(Vector3f origin,
        vector <HairSystem> hairs,
        vector<float> weights) {
  origin = origin;
  hairs = hairs;
  weights = weights;
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
  for (int i = 1; i < hairs.size(); i++) {
    Vector3f point = Vector3f::ZERO;
    for (int j = 0; j < hairs.size(); j++) {
      point += hairs[i].getState()[j] * weights[i];
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
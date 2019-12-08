#include "hairsystem.h"
#include "camera.h"
#include "curve.h"
#include "surf.h"
#include "symhair.h"
#include <string>
#include <iostream>

using namespace std;

SymHair::SymHair(Vector3f origin_input,
        vector<HairSystem*> hair_inputs,
        vector<float> weight_inputs) {
  origin = origin_input;
  hairs = hair_inputs;
  weights = weight_inputs;
  hairColor = Vector3f(0.642589, 0.347272, 0.211211);
}

void SymHair::draw(GLProgram& gl, VertexRecorder curveRec, VertexRecorder surfaceRec) {
  gl.disableLighting();
  gl.updateModelMatrix(Matrix4f::identity());
  vector<Vector3f> points;
  points.push_back(origin);

  int hair_len = (hairs[0]->getState()).size() / 2;
  for (int i = 1; i < hair_len; i++) {
    Vector3f point = Vector3f::ZERO;
    for (int j = 0; j < hairs.size(); j++) {
      point += hairs[j]->getState()[2 * i] * weights[j];
    }
    points.push_back(point);
  }

  Curve curve = evalBspline(points, 8);
  recordCurve(curve, &curveRec);
  glLineWidth(1.0f);
  // curveRec.draw(GL_LINES);

  Curve profile = evalCircle(0.01, 6);
  Surface surface = makeGenCyl(profile, curve);
  recordSurface(surface, &surfaceRec, hairColor);

  gl.enableLighting();
  gl.updateMaterial(hairColor);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  surfaceRec.draw(GL_TRIANGLES);
}

void SymHair::setHairColor(float r, float g, float b) {
//  printf("rgb is %f %f %f\n", r, g, b);
  hairColor[0] = r;
  hairColor[1] = g;
  hairColor[2] = b;
}
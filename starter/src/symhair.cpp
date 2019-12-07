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
}

void SymHair::draw(GLProgram& gl, VertexRecorder curveRec) {
//  const Vector3f HAIR_COLOR(0.9f, 0.9f, 0.9f);
//  gl.updateMaterial(HAIR_COLOR);

  gl.disableLighting();
  gl.updateModelMatrix(Matrix4f::identity());
//  VertexRecorder rec;

  vector<Vector3f> points;
//  points.push_back(origin);
//  points.push_back(origin);
//  points.push_back(origin);
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
  recordCurve(curve, &curveRec);

  glLineWidth(1.0f);
  curveRec.draw(GL_LINES);

  Curve profile = evalCircle(0.01, 20);
  Surface surface = makeGenCyl(profile, curve);

  recordSurface(surface, &curveRec);

  // // recordSurface(surface, &surfaceRec);
  // // recordNormals(surface, &surfaceNormals, 0.1f);

  // gl.camera->SetUniforms(gl.program_light);
  gl.updateMaterial(Vector3f {0.6f, 0.3f, 0.0f}, Vector3f {0.6f, 0.3f, 0.0f});
  // gl.updateModelMatrix(Matrix4f::identity());
  // shade interior of polygons
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  // surfaceRec.draw(GL_TRIANGLES);
  curveRec.draw(GL_TRIANGLES);

  gl.enableLighting(); // reset to default lighting model
}
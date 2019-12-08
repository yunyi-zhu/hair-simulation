#include "hairsystem.h"
#include "camera.h"
#include "curve.h"
#include "surf.h"
#include <string>
#include <iostream>

using namespace std;

static Vector3f headCollisionForce(Vector3f point) {
  float length = point.abs();
  if ( length < 1.07) {
    float forceAbs = COLLISION_RES;
    return forceAbs * point.normalized();
  } else {
    return Vector3f::ZERO;
  }
}

HairSystem::HairSystem(Vector3f origin, int length)
{
  H = length;

  for (int i = 0; i < H; i++) {
    Vector3f sub_origin;
    if (i == 0) {
      sub_origin = origin;
    } else {
      sub_origin = origin + Vector3f(pow(-1, i) * HORI_DELTA, i*UNIT_H, -pow(-1, i) * VERTI_DELTA);
    }
    m_vVecState.push_back(sub_origin);
    m_vVecState.push_back(Vector3f::ZERO);
  }

  // core springs
  for (int i = 0; i < H - 1; i++) {
    springs.push_back(Vector4f(i, i+1, CORE_L, CORE_K));
  }

  // support springs
  for (int i = 0; i < H - 2; i++) {
    springs.push_back(Vector4f(i, i+2, SUPPORT_L, SUPPORT_K));
  }

  for (int i = 0; i < H - 3; i++) {
    springs.push_back(Vector4f(i, i+3, SUPPORT_L_3, SUPPORT_K));
  }
  fixedPtIndex.push_back(0);

  // wind
  windBlowing = true;
  windDirection = Vector3f(0,0,1);
  windStrength = 7.0f;

  hairColor = Vector3f(0.642589, 0.347272, 0.211211);
  tempColor = hairColor;
  highlightCore = false;
}

std::vector<Vector3f> HairSystem::evalF(std::vector<Vector3f> state)
{
  std::vector<Vector3f> f;

  // gravity and drag for structural points
  for (int i = 0; i < H; i++) {
    f.push_back(state[2 * i + 1]);
    Vector3f gravity(0.0, -GRAVITY, 0.0);
    Vector3f drag = -K_DRAG * state[2 * i + 1] / M;
    Vector3f collision = headCollisionForce(state[2 * i]);
    Vector3f windForce = Vector3f::ZERO;

    if (windStrength > 0 && i > H / 2) {
      // only blow the lower half of your hair
      // to create a realistic effect
      windForce = windDirection * windStrength;
      if (i > H * 3 / 4) {
        windForce *= 2;
      }
    }

    f.push_back(gravity + drag + collision + windForce);
  }

  // springs
  for (int i = 0; i < springs.size(); i++) {
    float restLen = springs[i][2];
    float stiff = springs[i][3];
    int ind1 = (int) springs[i][0];
    int ind2 = (int) springs[i][1];
    // d and ||d|| in the formula
    Vector3f d = state[2 * ind1] - state[2 * ind2];
    Vector3f spring_force = - stiff * (d.abs() - restLen) * d.normalized();
    f[2 * ind1 + 1] += spring_force / M;
    f[2 * ind2 + 1] -= spring_force / M;
  }

  for (int i = 0; i < fixedPtIndex.size(); i++) {
    f[2 * fixedPtIndex[i]] = Vector3f::ZERO;
    f[2 * fixedPtIndex[i] + 1] = Vector3f::ZERO;
  }
  return f;
}

void HairSystem::draw(GLProgram& gl, VertexRecorder curveRec, VertexRecorder surfaceRec)
{
  gl.disableLighting();
  gl.updateModelMatrix(Matrix4f::identity());

  vector<Vector3f> state = getState();
  vector<Vector3f> points;
  points.push_back(state[0]);
  for (int i = 0; i < H; i++) {
    points.push_back(state[2 * i]);
  }

  Curve curve = evalBspline(points, 8);

  recordCurve(curve, &curveRec);
  // recordCurveFrames(curve, &curveFrames, 0.1f);
  glLineWidth(1.0f);
  // curveRec.draw(GL_LINES);

  Curve profile = evalCircle(0.01, 6);
  Surface surface = makeGenCyl(profile, curve);
  recordSurface(surface, &surfaceRec, hairColor);

  gl.enableLighting();
  gl.updateMaterial(hairColor);
  // shade interior of polygons
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  // makes wrong light: glEnable(GL_CULL_FACE); glCullFace(GL_BACK);
  surfaceRec.draw(GL_TRIANGLES);
}

void HairSystem::setHairCurve(float l_input) {
  for (int i = 2*H - 3; i < 3*H - 6; i++) {
    springs[i][2] = l_input * UNIT_H;
  }
}

void HairSystem::toggleWind() {
  windBlowing = !windBlowing;
}

void HairSystem::toggleHighlight() {
  highlightCore = !highlightCore;
  if (highlightCore) {
    tempColor = hairColor;
    setHairColor(1, 0, 0);
  } else {
    setHairColor(tempColor[0], tempColor[1], tempColor[2]);
  }
}

void HairSystem::setWindStrength(float strength) {
  windStrength = strength;
}

void HairSystem::setWindDirection(float index) {
  float theta_min = 0;
  float theta_max = M_PI;
  float offset = M_PI / 3;
  float theta = index * (theta_max - theta_min) + theta_min + offset;

  windDirection[0] = cos(theta);
  windDirection[2] = sin(theta);
}

void HairSystem::setHairColor(float r, float g, float b) {
  hairColor[0] = r;
  hairColor[1] = g;
  hairColor[2] = b;
}
#include "hairsystem.h"
#include "camera.h"
#include "vertexrecorder.h"
#include <string>
#include <iostream>

// your system should at least contain 8x8 particles.


const float ks = 1.5;
const float unitL = 0.5;
const float rls = unitL;
const float rlshear = 1.414 * unitL;
const float rlf = 2 * unitL;

const float kd = 0.3;
const float m = 0.01;

const float GRAVITY = 9.8f;
const float K_DRAG = 0.03f;
const float M = 0.01f;

const int C = 4; // number of points on a curve
const int H = 8; // number of curves on a hair
const float UNIT_R = 0.1f; // radius of a curve or square
const float UNIT_H = 0.05f;
const float STR_L = 0.1f;
const float STR_K = 50.0f;

using namespace std;

HairSystem::HairSystem()
{
  Vector3f Origin(0, 0, 0);
  for (int i = 0; i < H; i++) {
    Vector3f sub_origin(0, i*UNIT_H, 0);
    m_vVecState.push_back(sub_origin);
    m_vVecState.push_back(Vector3f::ZERO);

    m_vVecState.push_back(sub_origin + Vector3f(UNIT_R, 0, 0));
    m_vVecState.push_back(Vector3f::ZERO);

    m_vVecState.push_back(sub_origin + Vector3f(UNIT_R, 0, UNIT_R));
    m_vVecState.push_back(Vector3f::ZERO);

    m_vVecState.push_back(sub_origin + Vector3f(0, 0, UNIT_R));
    m_vVecState.push_back(Vector3f::ZERO);
  }

  // structural springs
  for (int i = 0; i < H; i++) {
    for (int j = 0; j < C-1; j++) {
      springs.push_back(Vector4f(indexOf(i, j), indexOf(i, j+1), STR_L, STR_K));
    }
    if ( i < H-1 ) {
      springs.push_back(Vector4f(indexOf(i, C-1), indexOf(i+1, 0), UNIT_H, STR_K));
    }
  }

  fixedPtIndex.push_back(0);
}

std::vector<Vector3f> HairSystem::evalF(std::vector<Vector3f> state)
{
  std::vector<Vector3f> f;

  // gravity and drag
  for (int i = 0; i < H; i++) {
    for (int j = 0; j < C; j++) {
      f.push_back(state[2 * indexOf(i, j) + 1]);
      Vector3f gravity(0.0, -GRAVITY, 0.0);
      Vector3f drag = -K_DRAG * state[2 * indexOf(i, j) + 1] / M;
      f.push_back(gravity + drag);
    }
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

void HairSystem::draw(GLProgram& gl)
{
  const Vector3f HAIR_COLOR(0.9f, 0.9f, 0.9f);
  gl.updateMaterial(HAIR_COLOR);
  vector<Vector3f> state = getState();

  drawSphere(1.0f, 2, 2);

  gl.disableLighting();
  gl.updateModelMatrix(Matrix4f::identity());
  VertexRecorder rec;

  // draw springs as lines;
  for (int i = 0; i < springs.size(); i++) {
    Vector4f sp = springs[i];
    rec.record(state[2*sp[0]], HAIR_COLOR);
    rec.record(state[2*sp[1]], HAIR_COLOR);
  }

  glLineWidth(3.0f);
  rec.draw(GL_LINES);
  gl.enableLighting(); // reset to default lighting model

}

int HairSystem::indexOf(int i, int j)
{
  return i * C + j;
}


#include "clothsystem.h"
#include "camera.h"
#include "vertexrecorder.h"
#include <string>
#include <iostream>

 // your system should at least contain 8x8 particles.
const int W = 8;
const int H = 8;

const float ks = 1.5;
const float unitL = 0.5;
const float rls = unitL;
const float rlshear = 1.414 * unitL;
const float rlf = 2 * unitL;

const float kd = 0.3;
const float m = 0.01;

using namespace std;

ClothSystem::ClothSystem()
{
    // TODO 5. Initialize m_vVecState with cloth particles. 
    // You can again use rand_uniform(lo, hi) to make things a bit more interesting
    breeze = false;
    showWireframe = true;
    smooth = false;
    Vector3f O(0.4f, 1, 0);
    for (int j = 0; j < H; j++) {
        for (int i = 0; i < W; i++) {
            m_vVecState.push_back(O + Vector3f(unitL * i, unitL * j, 0));
            m_vVecState.push_back(Vector3f::ZERO);
        }
    }

    // fix upper left and upper right corner
    fixedPtInd.push_back(indexOf(0, H-1));
    fixedPtInd.push_back(indexOf(W-1, H-1));

    // structural springs
    for (int j = 0; j < H; j++) {
        for (int i = 0; i < W; i++) {
            if (j + 1 <= H - 1) {
                springs.push_back(Vector4f(indexOf(i, j), indexOf(i, j + 1), rls, ks));
                // cout << indexOf(i, j) << indexOf(i, j + 1) << endl;
            }
            if (i + 1 <= W - 1) {
                springs.push_back(Vector4f(indexOf(i, j), indexOf(i + 1, j), rls, ks));
                // cout << indexOf(i, j) << indexOf(i + 1, j) << endl;
            }
        }
    }

    // shear spring
    for (int j = 0; j < H - 1; j++) {
        for (int i = 0; i < W; i++) {
            if (i-1 >= 0) {
                springs.push_back(Vector4f(indexOf(i, j), indexOf(i - 1, j + 1), rlshear, ks));
            }
            if (i+1 <= W-1) {
                springs.push_back(Vector4f(indexOf(i, j), indexOf(i + 1, j + 1), rlshear, ks));
            }
        }
    }

    // flex spring
    for (int j = 0; j < H; j++) {
        for (int i = 0; i < W; i++) {
            if (j + 2 <= H - 1) {
                springs.push_back(Vector4f(indexOf(i, j), indexOf(i, j + 2), rlf, ks));
            }
            if (i + 2 <= W - 1) {
                springs.push_back(Vector4f(indexOf(i, j), indexOf(i + 2, j), rlf, ks));
            }
        }
    }
    // cout << springs.size() << endl;
}

int ClothSystem::indexOf(int i, int j)
{
    return j * W + i;
}

void ClothSystem::addBreeze()
{
    breeze = !breeze;
};

void ClothSystem::addWireframe()
{
    showWireframe = !showWireframe;
};

void ClothSystem::addSmooth()
{
    smooth = !smooth;
};

void ClothSystem::moveDown()
{
    vector<Vector3f> state = getState();
    vector<Vector3f> newState;
    for (int i = 0; i < state.size() / 2; i++) {
        newState.push_back(state[2 * i] - Vector3f(0, 0.5, 0));
        newState.push_back(state[2 * i + 1]);
    }
    setState(newState);
};

void ClothSystem::moveUp()
{
    vector<Vector3f> state = getState();
    vector<Vector3f> newState;
    for (int i = 0; i < state.size() / 2; i++) {
        newState.push_back(state[2 * i] + Vector3f(0, 0.5, 0));
        newState.push_back(state[2 * i + 1]);
    }
    setState(newState);
};

void ClothSystem::moveLeft()
{
    vector<Vector3f> state = getState();
    vector<Vector3f> newState;
    for (int i = 0; i < state.size() / 2; i++) {
        newState.push_back(state[2 * i] - Vector3f(0.5, 0, 0));
        newState.push_back(state[2 * i + 1]);
    }
    setState(newState);
};

void ClothSystem::moveRight()
{
    vector<Vector3f> state = getState();
    vector<Vector3f> newState;
    for (int i = 0; i < state.size() / 2; i++) {
        newState.push_back(state[2 * i] + Vector3f(0.5, 0, 0));
        newState.push_back(state[2 * i + 1]);
    }
    setState(newState);
};

std::vector<Vector3f> ClothSystem::evalF(std::vector<Vector3f> state)
{
    std::vector<Vector3f> f;
    // TODO 5. implement evalF
    // - gravity
    // - viscous drag
    // - structural springs
    // - shear springs
    // - flexion springs

    for (int j = 0; j < H; j++) {
        for (int i = 0; i < W; i++) {
            f.push_back(state[2 * indexOf(i, j) + 1]);
            Vector3f gravity(0.0, - 9.8, 0.0);
            Vector3f drag = - kd * state[2 * indexOf(i, j) + 1] / m;
            // f.push_back(Vector3f::ZERO);
            f.push_back(gravity + drag);
        }
    }

    // Spring forces (p1, p2, rest length, stiffness)
    for (int i = 0; i < springs.size(); i++) {
        float restLen = springs[i][2];
        float stiff = springs[i][3];
        int ind1 = (int) springs[i][0];
        int ind2 = (int) springs[i][1];
        // d and ||d|| in the formula
        Vector3f d = state[2 * ind1] - state[2 * ind2];
        Vector3f spring1 = - stiff * (d.abs() - restLen) * d.normalized();
        f[2 * ind1 + 1] += spring1 / m;
        f[2 * ind2 + 1] -= spring1 / m;
    }

    // breeze force
    if (breeze) {
        float windF = rand_uniform(8.0, 11.0);
        for (int i = 0; i < state.size() / 2; i++) {
            // add acceleration in negative z direction
            f[2 * i + 1] += Vector3f(0, 0, - windF);
        }
    }

    // fix fixed points, set v and a to 0
    for (int i = 0; i < fixedPtInd.size(); i++) {
        f[2 * fixedPtInd[i]] = Vector3f::ZERO;
        f[2 * fixedPtInd[i] + 1] = Vector3f::ZERO;
    }
    return f;
}

// smooth shading
void ClothSystem::drawSmooth() {
    vector<Vector3f> state = getState();
    vector<Vector3f> vs;
    vector<Vector3f> perFNormals;
    vector<vector<Vector3f>> perVNormals(state.size() / 2);
    vector<Vector3f> faces;

    for (int i = 0; i < state.size() / 2; i++) {
        vs.push_back(state[2 * i]);
    }

    int ind1, ind2, ind3;
    for (int j = 0; j < H; j++) {
        for (int i = 0; i < W - 1; i++) {
            if (j - 1 >= 0) {
                ind1 = indexOf(i,j);
                ind2 = indexOf(i+1, j-1);
                ind3 = indexOf(i+1, j);
                faces.push_back(Vector3f(ind1, ind2, ind3));
                Vector3f fn = Vector3f::cross(vs[ind1] - vs[ind2], vs[ind2] - vs[ind3]).normalized();
                perFNormals.push_back(fn);

                perVNormals[ind1].push_back(fn);
                perVNormals[ind2].push_back(fn);
                perVNormals[ind3].push_back(fn);
            }
            if (j + 1 <= H - 1) {
                ind1 = indexOf(i, j+1);
                ind2 = indexOf(i,j);
                ind3 = indexOf(i+1, j);
                faces.push_back(Vector3f(ind1, ind2, ind3));
                Vector3f fn = Vector3f::cross(vs[ind1] - vs[ind2], vs[ind2] - vs[ind3]).normalized();
                perFNormals.push_back(fn);

                perVNormals[ind1].push_back(fn);
                perVNormals[ind2].push_back(fn);
                perVNormals[ind3].push_back(fn);
            }
        }
    }

    // average over adjacent surface normals
    vector<Vector3f> vn;
    for (int i = 0; i < vs.size(); i++) {
        Vector3f v = Vector3f::ZERO;
        for (int j = 0; j < perVNormals[i].size(); j++) {
            v += perVNormals[i][j];
        }
        v /= perVNormals[i].size();
        vn.push_back(v.normalized());
    }

    VertexRecorder rec1;
    for (int i = 0; i < faces.size(); i++) {
        rec1.record(vs[faces[i][0]], vn[faces[i][0]]);
        rec1.record(vs[faces[i][1]], vn[faces[i][1]]);
        rec1.record(vs[faces[i][2]], vn[faces[i][2]]);
    }
    rec1.draw();
}

void ClothSystem::draw(GLProgram& gl)
{
    //TODO 5: render the system 
    //         - ie draw the particles as little spheres
    //         - or draw the springs as little lines or cylinders
    //         - or draw wireframe mesh

    const Vector3f CLOTH_COLOR(0.9f, 0.9f, 0.9f);
    gl.updateMaterial(CLOTH_COLOR);
    vector<Vector3f> state = getState();
    if (smooth) {
        gl.updateModelMatrix(Matrix4f::identity());
        drawSmooth();
    } else {
        // EXAMPLE for how to render cloth particles.
        for (int i = 0; i < state.size() - 1; i += 2) {
            Vector3f pos = state[i]; //YOUR PARTICLE POSITION
            gl.updateModelMatrix(Matrix4f::translation(pos));
            drawSphere(0.04f, 8, 8);
        }
    }

    // EXAMPLE: This shows you how to render lines to debug the spring system.
    //
    //          You should replace this code.
    //
    //          Since lines don't have a clearly defined normal, we can't use
    //          a regular lighting model.
    //          GLprogram has a "color only" mode, where illumination
    //          is disabled, and you specify color directly as vertex attribute.
    //          Note: enableLighting/disableLighting invalidates uniforms,
    //          so you'll have to update the transformation/material parameters
    //          after a mode change.

    if (showWireframe) {
        gl.disableLighting();
        gl.updateModelMatrix(Matrix4f::identity()); // update uniforms after mode change
        VertexRecorder rec;

        // draw springs
        for (int i = 0; i < springs.size(); i += 1) {
            Vector4f sp = springs[i];
            rec.record(state[2 * sp[0]], CLOTH_COLOR);
            rec.record(state[2 * sp[1]], CLOTH_COLOR);
        }
        glLineWidth(3.0f);
        rec.draw(GL_LINES);
        gl.enableLighting(); // reset to default lighting model
    }
}


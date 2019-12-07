#include "surf.h"
#include "vertexrecorder.h"
using namespace std;

const float c_pi = 3.14159265358979323846f;

namespace
{
    // We're only implenting swept surfaces where the profile curve is
    // flat on the xy-plane.  This is a check function.
    static bool checkFlat(const Curve &profile)
    {
        for (unsigned i=0; i<profile.size(); i++)
            if (profile[i].V[2] != 0.0 ||
                profile[i].T[2] != 0.0 ||
                profile[i].N[2] != 0.0)
                return false;
    
        return true;
    }
}

void meshTriangle(const unsigned steps, Surface &surface, int l) {
    int nexts;
    // changed to s < steps - 1 to avoid last - first triangle mesh
    for (int s = 0; s < steps - 1; s++) {
        if (s == steps - 1) {
            nexts = 0;
        } else {
            nexts = s + 1;
        }
        for (int i = 0; i < l - 1; i++) {
            surface.VF.push_back(Tup3u(s * l + i, s * l + i + 1, nexts * l + i));
            surface.VF.push_back(Tup3u(s * l + i + 1, nexts * l + i + 1, nexts * l + i));
        }
    }
}

Surface makeSurfRev(const Curve &profile, unsigned steps)
{
    Surface surface;
    int l = profile.size();
    
    // if (!checkFlat(profile))
    // {
    //     cerr << "surfRev profile curve must be flat on xy plane." << endl;
    //     exit(0);
    // }

    for (int s = 0; s < steps; s++) {
        float theta = (float) s*2*c_pi/steps;
        Matrix3f M = Matrix3f::rotateY(theta);
        Matrix3f Mn = M.inverse().transposed();

        for (int i = 0; i < l; i++) {
            CurvePoint p = profile[i];
            Vector3f newV = M * p.V;
            // reverse the normals
            Vector3f newN = - (Mn * p.N).normalized();
            surface.VV.push_back(newV);
            surface.VN.push_back(newN);
        }
    }
    meshTriangle(steps, surface, l);
    return surface;
}

Surface makeGenCyl(const Curve &profile, const Curve &sweep )
{
    Surface surface;
    int l = profile.size();

    // if (!checkFlat(profile))
    // {
    //     cerr << "genCyl profile curve must be flat on xy plane." << endl;
    //     exit(0);
    // }

    for (int s = 0; s < sweep.size(); s++) {
        CurvePoint p = sweep[s];

        Matrix4f M(Vector4f(p.N, 0), Vector4f(p.B, 0), Vector4f(p.T, 0), Vector4f(p.V, 1));
        Matrix4f Mn = M.inverse().transposed();

        for (int i = 0; i < l; i++) {
            CurvePoint cp = profile[i];
            Vector3f newV = (M * Vector4f(cp.V, 1)).xyz();
            Vector3f newN = -(Mn * Vector4f(cp.N, 0)).xyz().normalized();
            surface.VV.push_back(newV);
            surface.VN.push_back(newN);
        }
    }
    meshTriangle(sweep.size(), surface, l);

    return surface;
}

void recordSurface(const Surface &surface, VertexRecorder* recorder, Vector3f FCOLOR) {
	// const Vector3f HAIRCOLOR(0.6f, 0.3f, 0.0f);
    Vector3f COLOR(0.75, 0.52, 0.76);
    for (int i=0; i<(int)surface.VF.size() - 1; i++)
    {
		recorder->record(surface.VV[surface.VF[i][0]], surface.VN[surface.VF[i][0]], COLOR);
		recorder->record(surface.VV[surface.VF[i][1]], surface.VN[surface.VF[i][1]], COLOR);
		recorder->record(surface.VV[surface.VF[i][2]], surface.VN[surface.VF[i][2]], COLOR);
    }
}

void recordNormals(const Surface &surface, VertexRecorder* recorder, float len)
{
	const Vector3f NORMALCOLOR(0, 1, 1);
    for (int i=0; i<(int)surface.VV.size(); i++)
    {
		recorder->record_poscolor(surface.VV[i], NORMALCOLOR);
		recorder->record_poscolor(surface.VV[i] + surface.VN[i] * len, NORMALCOLOR);
    }
}

void outputObjFile(ostream &out, const Surface &surface)
{
    
    for (int i=0; i<(int)surface.VV.size(); i++)
        out << "v  "
            << surface.VV[i][0] << " "
            << surface.VV[i][1] << " "
            << surface.VV[i][2] << endl;

    for (int i=0; i<(int)surface.VN.size(); i++)
        out << "vn "
            << surface.VN[i][0] << " "
            << surface.VN[i][1] << " "
            << surface.VN[i][2] << endl;

    out << "vt  0 0 0" << endl;
    
    for (int i=0; i<(int)surface.VF.size(); i++)
    {
        out << "f  ";
        for (unsigned j=0; j<3; j++)
        {
            unsigned a = surface.VF[i][j]+1;
            out << a << "/" << "1" << "/" << a << " ";
        }
        out << endl;
    }
}

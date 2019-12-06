#include "curve.h"
#include "vertexrecorder.h"
#include <assert.h>
using namespace std;

const float c_pi = 3.14159265358979323846f;

namespace
{
	// Approximately equal to.  We don't want to use == because of
	// precision issues with floating point.
	inline bool approx(const Vector3f& lhs, const Vector3f& rhs)
	{
		const float eps = 1e-8f;
		return (lhs - rhs).absSquared() < eps;
	}
}

Curve evalBezier(const vector< Vector3f >& P, unsigned steps)
{
	// Check
	if (P.size() < 4 || P.size() % 3 != 1)
	{
		cerr << "evalBezier must be called with 3n+1 control points." << endl;
		exit(0);
	}

	// TODO:
	// You should implement this function so that it returns a Curve
	// (e.g., a vector< CurvePoint >).  The variable "steps" tells you
	// the number of points to generate on each piece of the spline.
	// At least, that's how the sample solution is implemented and how
	// the SWP files are written.  But you are free to interpret this
	// variable however you want, so long as you can control the
	// "resolution" of the discretized spline curve with it.

	// Also note that you may assume that all Bezier curves that you
	// receive have G1 continuity.  Otherwise, the TNB will not be
	// be defined at points where this does not hold.

    int num_of_seg = ((int) P.size() - 1) / 3;
	Curve R;

	for (int i = 0; i < num_of_seg; i++)
	{
		Vector3f P0 = P[3*i];
		Vector3f P1 = P[3*i + 1];
		Vector3f P2 = P[3*i + 2];
		Vector3f P3 = P[3*i + 3];

		for (unsigned j = 0; j <= steps; j++) {
			float t = (float) j / steps;

			Vector3f T,V,N,B;
			// curve along position
			V = P0*(1-t)*(1-t)*(1-t) + P1*3*t*(1-t)*(1-t) + P2*3*t*t*(1-t) + P3*t*t*t;

			// Tangent vector is first derivative
			T = (3*(1-t)*(1-t)*(P1 - P0) + 6*(1-t)*t*(P2 - P1) + 3*t*t*(P3 - P2)).normalized();

			// Normal and binormal vectors are recursively calculated
			if (i == 0 && j == 0) {
				// Initialize B0 by selecting an arbitrary B0 to be
				// pointing in the positive z-direction non-parallel to T1
				Vector3f B0 = Vector3f(0.0f, 0.0f, 1.0f);
				Vector3f T1 = (3*(P1 - P0)).normalized();
		        if (approx(Vector3f::cross(B0, T1), Vector3f(0.0f, 0.0f, 0.0f))) {
		        	B0 = Vector3f(0.0f, 1.0f, 0.0f);
		        }
		        B = B0.normalized();
				N = Vector3f::cross(B, T1).normalized();
			} else {
				N = Vector3f::cross(R[R.size() - 1].B, T).normalized();
				B = Vector3f::cross(T, N).normalized();
			}
			CurvePoint p = {V, T, N, B};
			R.push_back(p);
		}
	}
	return R;
}

Curve evalBspline(const vector< Vector3f >& P, unsigned steps)
{
	// Check
	if (P.size() < 4)
	{
		cerr << "evalBspline must be called with 4 or more control points." << endl;
		exit(0);
	}

	// TODO:
	// It is suggested that you implement this function by changing
	// basis from B-spline to Bezier.  That way, you can just call
	// your evalBezier function.

	Matrix4f Bez = Matrix4f(1.0, -3.0, 3.0, -1.0, 
							0.0, 3.0, -6.0, 3.0, 
							0.0, 0.0, 3.0, -3.0, 
							0.0, 0.0, 0.0, 1.0);
	float coeff = 1.0/6;
	Matrix4f Bsp = coeff * Matrix4f(1.0, -3.0, 3.0, -1.0, 
									4.0, 0.0, -6.0, 3.0, 
									1.0, 3.0, 3.0, -3.0, 
									0.0, 0.0, 0.0, 1.0);
	Matrix4f product = Bsp * (Bez.inverse());

    vector<Vector3f> bezPoints;

	for (int i = 0; i < (int)P.size() - 3; ++i) {
	    Vector4f P0(P[i],   0);
	    Vector4f P1(P[i+1], 0);
	    Vector4f P2(P[i+2], 0);
	    Vector4f P3(P[i+3], 0);
	    Matrix4f G = Matrix4f(P0, P1, P2, P3);
	    Matrix4f GBez = G * Bsp * Bez.inverse();
        
        // makes sure total number of points is 4 + 3*n for input into evalBezier
	    if (i == 0) {
	    	for (int j = 0; j < 4; ++j) {
	    		Vector3f bezPoint = GBez.getCol(j).xyz();
	    		bezPoints.push_back(bezPoint);
	    	}
	    } else {
	    	bezPoints.push_back(GBez.getCol(1).xyz());
	    	bezPoints.push_back(GBez.getCol(2).xyz());
	    	bezPoints.push_back(GBez.getCol(3).xyz());
	    }
	}
	return evalBezier(bezPoints, steps);
}


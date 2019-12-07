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
			// cout << N[0] << " " << N[1] << " " << N[2] << endl;
			// cout << B[0] << " " << B[1] << " " << B[2] << endl;

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

	// change of basis from Bezier
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

Curve evalCircle(float radius, unsigned steps)
{
	// Preallocate a curve with steps+1 CurvePoints
	Curve R(steps + 1);

	// Fill it in counterclockwise
	for (unsigned i = 0; i <= steps; ++i)
	{
		// step from 0 to 2pi
		float t = 2.0f * c_pi * float(i) / steps;

		// Initialize position
		// We're pivoting counterclockwise around the y-axis
		R[i].V = radius * Vector3f(cos(t), sin(t), 0);

		// Tangent vector is first derivative
		R[i].T = Vector3f(-sin(t), cos(t), 0);

		// Normal vector is second derivative
		R[i].N = Vector3f(-cos(t), -sin(t), 0);

		// Finally, binormal is facing up.
		R[i].B = Vector3f(0, 0, 1);
	}

	return R;
}

void recordCurve(const Curve& curve, VertexRecorder* recorder)
{
	const Vector3f HAIRCOLOR(0.6, 0.3, 0.0);
	for (int i = 0; i < (int)curve.size() - 1; ++i)
	{
		recorder->record_poscolor(curve[i].V, HAIRCOLOR);
		recorder->record_poscolor(curve[i + 1].V, HAIRCOLOR);
	}
}

void recordCurveFrames(const Curve& curve, VertexRecorder* recorder, float framesize)
{
	Matrix4f T;
	const Vector3f RED(1, 0, 0);
	const Vector3f GREEN(0, 1, 0);
	const Vector3f BLUE(0, 0, 1);
	
	const Vector4f ORGN(0, 0, 0, 1);
	const Vector4f AXISX(framesize, 0, 0, 1);
	const Vector4f AXISY(0, framesize, 0, 1);
	const Vector4f AXISZ(0, 0, framesize, 1);

	for (int i = 0; i < (int)curve.size(); ++i)
	{
		T.setCol(0, Vector4f(curve[i].N, 0));
		T.setCol(1, Vector4f(curve[i].B, 0));
		T.setCol(2, Vector4f(curve[i].T, 0));
		T.setCol(3, Vector4f(curve[i].V, 1));
 
		// Transform orthogonal frames into model space
		Vector4f MORGN  = T * ORGN;
		Vector4f MAXISX = T * AXISX;
		Vector4f MAXISY = T * AXISY;
		Vector4f MAXISZ = T * AXISZ;

		// Record in model space
		recorder->record_poscolor(MORGN.xyz(), RED);
		recorder->record_poscolor(MAXISX.xyz(), RED);

		recorder->record_poscolor(MORGN.xyz(), GREEN);
		recorder->record_poscolor(MAXISY.xyz(), GREEN);

		recorder->record_poscolor(MORGN.xyz(), BLUE);
		recorder->record_poscolor(MAXISZ.xyz(), BLUE);
	}
}


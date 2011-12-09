/*  Project nGenEx
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         Geometry.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Geometric Utilities
*/

#include "MemDebug.h"
#include "Geometry.h"

// +--------------------------------------------------------------------+

void Rect::Inflate(int dx, int dy)
{
	x -= dx;
	w += dx*2;
	y -= dy;
	h += dy*2;
}

void Rect::Deflate(int dx, int dy)
{
	x += dx;
	w -= dx*2;
	y += dy;
	h -= dy*2;
}

void Rect::Inset(int l, int r, int t, int b)
{
	x += l;
	y += t;
	w -= l + r;
	h -= t + b;
}

int Rect::Contains(int ax, int ay) const
{
	if (ax < x)    return 0;
	if (ax > x+w)  return 0;
	if (ay < y)    return 0;
	if (ay > y+h)  return 0;

	return 1;
}

// +--------------------------------------------------------------------+

double
Point::Normalize()
{
	double scale = 1.0;
	double len   = length();

	if (len)
	scale /= len;

	x *= scale;
	y *= scale;
	z *= scale;

	return len;
}

// +--------------------------------------------------------------------+

void
Point::SetElement(int i, double v)
{
	switch (i) {
	case 0:  x = v; break;
	case 1:  y = v; break;
	case 2:  z = v; break;
	default:        break;
	}
}

// +--------------------------------------------------------------------+

Point
Point::operator*(const Matrix& m) const
{
	Point result;

	result.x = (m.elem[0][0] * x) + (m.elem[1][0] * y) + (m.elem[2][0] * z);
	result.y = (m.elem[0][1] * x) + (m.elem[1][1] * y) + (m.elem[2][1] * z);
	result.z = (m.elem[0][2] * x) + (m.elem[1][2] * y) + (m.elem[2][2] * z);

	return result;
}

// +--------------------------------------------------------------------+

double ClosestApproachTime(const Point& loc1, const Point& vel1,
const Point& loc2, const Point& vel2)
{
	double t = 0;

	Point D  = loc1-loc2;
	Point Dv = vel1-vel2;

	if (Dv.x || Dv.y || Dv.z)
	t = -1 * (Dv*D) / (Dv*Dv);

	return t;
}

// +--------------------------------------------------------------------+

float
Vec2::Normalize()
{
	float  scale = 1.0f;
	float  len   = length();

	if (len)
	scale /= len;

	x *= scale;
	y *= scale;

	return len;
}

// +--------------------------------------------------------------------+

float
Vec3::Normalize()
{
	float  scale = 1.0f;
	float  len   = length();

	if (len)
	scale /= len;

	x *= scale;
	y *= scale;
	z *= scale;

	return len;
}

// +--------------------------------------------------------------------+

Vec3
Vec3::operator*(const Matrix& m) const
{
	Vec3 result;

	result.x = (float) ((m.elem[0][0] * x) + (m.elem[1][0] * y) + (m.elem[2][0] * z));
	result.y = (float) ((m.elem[0][1] * x) + (m.elem[1][1] * y) + (m.elem[2][1] * z));
	result.z = (float) ((m.elem[0][2] * x) + (m.elem[1][2] * y) + (m.elem[2][2] * z));

	return result;
}

// +--------------------------------------------------------------------+

double ClosestApproachTime(const Vec3& loc1, const Vec3& vel1,
const Vec3& loc2, const Vec3& vel2)
{
	double t = 0;

	Point D  = loc1-loc2;
	Point Dv = vel1-vel2;

	if (Dv.x || Dv.y || Dv.z)
	t = -1 * (Dv*D) / (Dv*Dv);

	return t;
}

// +--------------------------------------------------------------------+

double
Quaternion::Normalize()
{
	double scale = 1.0;
	double len   = length();

	if (len)
	scale /= len;

	x *= scale;
	y *= scale;
	z *= scale;
	w *= scale;

	return len;
}

// +--------------------------------------------------------------------+

Matrix::Matrix()
{
	Identity();
}

Matrix::Matrix(const Matrix& m)
{
	CopyMemory(elem, m.elem, sizeof(elem));
}

Matrix::Matrix(const Point& vrt, const Point& vup, const Point& vpn)
{
	elem[0][0] = vrt.x;
	elem[0][1] = vrt.y;
	elem[0][2] = vrt.z;

	elem[1][0] = vup.x;
	elem[1][1] = vup.y;
	elem[1][2] = vup.z;

	elem[2][0] = vpn.x;
	elem[2][1] = vpn.y;
	elem[2][2] = vpn.z;
}

// +--------------------------------------------------------------------+

Matrix&
Matrix::operator =(const Matrix& m)
{
	CopyMemory(elem, m.elem, sizeof(elem));

	return *this;
}

// +--------------------------------------------------------------------+

Matrix&
Matrix::operator*=(const Matrix& m)
{
	return *this = *this * m;
}

// +--------------------------------------------------------------------+

void
Matrix::Identity()
{
	elem[0][0] = 1;
	elem[0][1] = 0;
	elem[0][2] = 0;

	elem[1][0] = 0;
	elem[1][1] = 1;
	elem[1][2] = 0;

	elem[2][0] = 0;
	elem[2][1] = 0;
	elem[2][2] = 1;
}

// +--------------------------------------------------------------------+

inline void swap_elem(double& a, double& b) { double t=a; a=b; b=t; }

void
Matrix::Transpose()
{
	swap_elem(elem[0][1], elem[1][0]);
	swap_elem(elem[0][2], elem[2][0]);
	swap_elem(elem[1][2], elem[2][1]);
}

// +--------------------------------------------------------------------+

void
Matrix::Rotate(double roll, double pitch, double yaw)
{
	double e[3][3];
	CopyMemory(e, elem, sizeof(elem));

	double sr = sin(roll);
	double cr = cos(roll);
	double sp = sin(pitch);
	double cp = cos(pitch);
	double sy = sin(yaw);
	double cy = cos(yaw);

	double a,b,c;

	a = cy*cr;
	b = cy*sr;
	c = -sy;

	elem[0][0] = a*e[0][0] + b*e[1][0] + c*e[2][0];
	elem[0][1] = a*e[0][1] + b*e[1][1] + c*e[2][1];
	elem[0][2] = a*e[0][2] + b*e[1][2] + c*e[2][2];

	a = cp*-sr + sp*sy*cr;
	b = cp* cr + sp*sy*sr;
	c = sp*cy;

	elem[1][0] = a*e[0][0] + b*e[1][0] + c*e[2][0];
	elem[1][1] = a*e[0][1] + b*e[1][1] + c*e[2][1];
	elem[1][2] = a*e[0][2] + b*e[1][2] + c*e[2][2];

	a = -sp*-sr + cp*sy*cr;
	b = -sp* cr + cp*sy*sr;
	c = cp*cy;

	elem[2][0] = a*e[0][0] + b*e[1][0] + c*e[2][0];
	elem[2][1] = a*e[0][1] + b*e[1][1] + c*e[2][1];
	elem[2][2] = a*e[0][2] + b*e[1][2] + c*e[2][2];
}

// +--------------------------------------------------------------------+

void
Matrix::Roll(double roll)
{
	double s = sin(roll);
	double c = cos(roll);

	double e00 = elem[0][0];
	double e01 = elem[0][1];
	double e02 = elem[0][2];
	double e10 = elem[1][0];
	double e11 = elem[1][1];
	double e12 = elem[1][2];

	elem[0][0] =  c*e00 + s*e10;
	elem[0][1] =  c*e01 + s*e11;
	elem[0][2] =  c*e02 + s*e12;

	elem[1][0] = -s*e00 + c*e10;
	elem[1][1] = -s*e01 + c*e11;
	elem[1][2] = -s*e02 + c*e12;
}

// +--------------------------------------------------------------------+

void
Matrix::Pitch(double pitch)
{
	double s = sin(pitch);
	double c = cos(pitch);

	double e10 = elem[1][0];
	double e11 = elem[1][1];
	double e12 = elem[1][2];
	double e20 = elem[2][0];
	double e21 = elem[2][1];
	double e22 = elem[2][2];

	elem[1][0] =  c*e10 + s*e20;
	elem[1][1] =  c*e11 + s*e21;
	elem[1][2] =  c*e12 + s*e22;

	elem[2][0] = -s*e10 + c*e20;
	elem[2][1] = -s*e11 + c*e21;
	elem[2][2] = -s*e12 + c*e22;
}

// +--------------------------------------------------------------------+

void
Matrix::Yaw(double yaw)
{
	double s = sin(yaw);
	double c = cos(yaw);

	double e00 = elem[0][0];
	double e01 = elem[0][1];
	double e02 = elem[0][2];
	double e20 = elem[2][0];
	double e21 = elem[2][1];
	double e22 = elem[2][2];

	elem[0][0] = c*e00 - s*e20;
	elem[0][1] = c*e01 - s*e21;
	elem[0][2] = c*e02 - s*e22;

	elem[2][0] = s*e00 + c*e20;
	elem[2][1] = s*e01 + c*e21;
	elem[2][2] = s*e02 + c*e22;
}

// +--------------------------------------------------------------------+

inline int sign(double d) { return (d >= 0); }

void
Matrix::ComputeEulerAngles(double& roll, double& pitch, double& yaw) const
{
	double cy;

	yaw   = asin(-elem[0][2]);
	cy    = cos(yaw);
	roll  = asin(elem[0][1] / cy);
	pitch = asin(elem[1][2] / cy);

	if (sign(cos(roll)*cy) != sign(elem[0][0]))
	roll = PI - roll;

	if (sign(cos(pitch)*cy) != sign(elem[2][2]))
	pitch = PI - pitch;   
}

// +--------------------------------------------------------------------+

Matrix
Matrix::operator*(const Matrix& m) const
{
	Matrix r;

	r.elem[0][0] = elem[0][0]*m.elem[0][0] + elem[0][1]*m.elem[1][0] + elem[0][2]*m.elem[2][0];
	r.elem[0][1] = elem[0][0]*m.elem[0][1] + elem[0][1]*m.elem[1][1] + elem[0][2]*m.elem[2][1];
	r.elem[0][2] = elem[0][0]*m.elem[0][2] + elem[0][1]*m.elem[1][2] + elem[0][2]*m.elem[2][2];

	r.elem[1][0] = elem[1][0]*m.elem[0][0] + elem[1][1]*m.elem[1][0] + elem[1][2]*m.elem[2][0];
	r.elem[1][1] = elem[1][0]*m.elem[0][1] + elem[1][1]*m.elem[1][1] + elem[1][2]*m.elem[2][1];
	r.elem[1][2] = elem[1][0]*m.elem[0][2] + elem[1][1]*m.elem[1][2] + elem[1][2]*m.elem[2][2];

	r.elem[2][0] = elem[2][0]*m.elem[0][0] + elem[2][1]*m.elem[1][0] + elem[2][2]*m.elem[2][0];
	r.elem[2][1] = elem[2][0]*m.elem[0][1] + elem[2][1]*m.elem[1][1] + elem[2][2]*m.elem[2][1];
	r.elem[2][2] = elem[2][0]*m.elem[0][2] + elem[2][1]*m.elem[1][2] + elem[2][2]*m.elem[2][2];

	return r;
}

// +--------------------------------------------------------------------+

Point
Matrix::operator*(const Point& p) const
{
	Point result;

	result.x = (elem[0][0] * p.x) + (elem[0][1] * p.y) + (elem[0][2] * p.z);
	result.y = (elem[1][0] * p.x) + (elem[1][1] * p.y) + (elem[1][2] * p.z);
	result.z = (elem[2][0] * p.x) + (elem[2][1] * p.y) + (elem[2][2] * p.z);

	return result;
}

// +--------------------------------------------------------------------+

Vec3
Matrix::operator*(const Vec3& v) const
{
	Vec3 result;

	result.x = (float) ((elem[0][0] * v.x) + (elem[0][1] * v.y) + (elem[0][2] * v.z));
	result.y = (float) ((elem[1][0] * v.x) + (elem[1][1] * v.y) + (elem[1][2] * v.z));
	result.z = (float) ((elem[2][0] * v.x) + (elem[2][1] * v.y) + (elem[2][2] * v.z));

	return result;
}

// +--------------------------------------------------------------------+

double 
Matrix::Cofactor(int i, int j) const
{
	int i1=0;
	int i2=2;
	int j1=0;
	int j2=2;

	if (i==0) i1=1; else if (i==2) i2=1;
	if (j==0) j1=1; else if (j==2) j2=1;

	double factor = elem[i1][j1]*elem[i2][j2] - elem[i1][j2]*elem[i2][j1];

	if ((i+j) & 1)
	factor *= -1;

	return factor;      
}

// +--------------------------------------------------------------------+

void 
Matrix::Invert()
{
	double f[3][3];
	int    i, j;

	for (i = 0; i < 3; i++)
	for (j = 0; j < 3; j++)
	f[i][j] = Cofactor(j,i);

	double det = elem[0][0] * f[0][0] +
	elem[0][1] * f[1][0] +
	elem[0][2] * f[2][0];

	if (det != 0) {
		double inv = 1/det;

		for (i = 0; i < 3; i++)
		for (j = 0; j < 3; j++)
		elem[i][j] = f[i][j] * inv;
	}
}

// +--------------------------------------------------------------------+
// +--------------------------------------------------------------------+
// +--------------------------------------------------------------------+

Plane::Plane()
: distance(0.0f)
{ }

Plane::Plane(const Point& p0, const Point& p1, const Point& p2)
{
	Point d1 = p1 - p0;
	Point d2 = p2 - p0;

	normal = (Vec3) d1.cross(d2);
	normal.Normalize();

	distance = (float) (normal * p0);
}

Plane::Plane(const Vec3& v0, const Vec3& v1, const Vec3& v2)
{
	Vec3 d1 = v1 - v0;
	Vec3 d2 = v2 - v0;

	normal = d1.cross(d2);
	normal.Normalize();

	distance = normal * v0;
}

void Plane::Rotate(const Vec3& v0, const Matrix& m)
{
	normal   = normal * m;
	distance = normal * v0;
}

void Plane::Translate(const Vec3& v0)
{
	distance = normal * v0;
}

// +--------------------------------------------------------------------+
// 3-D dot product.

double DotProduct(const Point& a, const Point& b)
{
	return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

// +--------------------------------------------------------------------+
// 3-D cross product.

void CrossProduct(const Point& a, const Point& b, Point& out)
{
	out.x = (a.y * b.z) - (a.z * b.y);
	out.y = (a.z * b.x) - (a.x * b.z);
	out.z = (a.x * b.y) - (a.y * b.x);
}

// +--------------------------------------------------------------------+
// Concatenate two 3x3 matrices.

void MConcat(double in1[3][3], double in2[3][3], double out[3][3])
{
	int     i, j;

	for (i=0 ; i<3 ; i++) {
		for (j=0 ; j<3 ; j++) {
			out[i][j] = in1[i][0] * in2[0][j] +
			in1[i][1] * in2[1][j] +
			in1[i][2] * in2[2][j];
		}
	}
}

/* GRAPHICS GEMS II ----------------------------------------------------
*
* lines_intersect:  AUTHOR: Mukesh Prasad
*
*   This function computes whether two line segments,
*   respectively joining the input points (x1,y1) -- (x2,y2)
*   and the input points (x3,y3) -- (x4,y4) intersect.
*   If the lines intersect, the output variables x, y are
*   set to coordinates of the point of intersection.
*
*   All values are in integers.  The returned value is rounded
*   to the nearest integer point.
*
*   If non-integral grid points are relevant, the function
*   can easily be transformed by substituting floating point
*   calculations instead of integer calculations.
*
*   Entry
*        x1, y1,  x2, y2   Coordinates of endpoints of one segment.
*        x3, y3,  x4, y4   Coordinates of endpoints of other segment.
*
*   Exit
*        x, y              Coordinates of intersection point.
*
*   The value returned by the function is one of:
*
*        DONT_INTERSECT    0
*        DO_INTERSECT      1
*        COLLINEAR         2
*
* Error conditions:
*
*     Depending upon the possible ranges, and particularly on 16-bit
*     computers, care should be taken to protect from overflow.
*
*     In the following code, 'long' values have been used for this
*     purpose, instead of 'int'.
*
*/

#define DONT_INTERSECT    0
#define DO_INTERSECT      1
#define COLLINEAR         2

inline int SAME_SIGNS(double a, double b)
{
	return ((a>=0 && b>=0) || (a<0 && b<0));
}

int
lines_intersect(
/* 1st line segment */ double x1, double y1, double x2, double y2,
/* 2nd line segment */ double x3, double y3, double x4, double y4,
/* pt of intersect  */ double& ix, double& iy)
{
	double a1, a2, b1, b2, c1, c2; /* Coefficients of line eqns. */
	double r1, r2, r3, r4;         /* 'Sign' values */
	double denom, offset, num;     /* Intermediate values */

	/* Compute a1, b1, c1, where line joining points 1 and 2
	* is "a1 x  +  b1 y  +  c1  =  0".  */

	a1 = y2 - y1;
	b1 = x1 - x2;
	c1 = x2 * y1 - x1 * y2;

	/* Compute r3 and r4.  */

	r3 = a1 * x3 + b1 * y3 + c1;
	r4 = a1 * x4 + b1 * y4 + c1;

	/* Check signs of r3 and r4.  If both point 3 and point 4 lie on
	* same side of line 1, the line segments do not intersect.  */

	if ( r3 != 0 &&
			r4 != 0 &&
			SAME_SIGNS( r3, r4 ))
	return ( DONT_INTERSECT );

	/* Compute a2, b2, c2 */

	a2 = y4 - y3;
	b2 = x3 - x4;
	c2 = x4 * y3 - x3 * y4;

	/* Compute r1 and r2 */

	r1 = a2 * x1 + b2 * y1 + c2;
	r2 = a2 * x2 + b2 * y2 + c2;

	/* Check signs of r1 and r2.  If both point 1 and point 2 lie
	* on same side of second line segment, the line segments do
	* not intersect.  */

	if ( r1 != 0 &&
			r2 != 0 &&
			SAME_SIGNS( r1, r2 ))
	return ( DONT_INTERSECT );

	/* Line segments intersect: compute intersection point.  */

	denom = a1 * b2 - a2 * b1;
	if ( denom == 0 )
	return ( DONT_INTERSECT );
	offset = denom < 0 ? - denom / 2 : denom / 2;

	/* The denom/2 is to get rounding instead of truncating.  It
	* is added or subtracted to the numerator, depending upon the
	* sign of the numerator.  */

	num = b1 * c2 - b2 * c1;
	ix = ( num < 0 ? num - offset : num + offset ) / denom;

	num = a2 * c1 - a1 * c2;
	iy = ( num < 0 ? num - offset : num + offset ) / denom;

	return ( DO_INTERSECT );
}


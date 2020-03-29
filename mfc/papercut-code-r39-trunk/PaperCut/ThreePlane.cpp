/*!

	@file	 ThreePlane.cpp
	
	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id$

  Class representing a plane in 3-space. Requires definition of three points.

*/

#include "stdafx.h"
#include ".\threepoint.h"
#include ".\threeplane.h"

#include "Face.h"

// Common initialization called by all constructors
void ThreePlane::CommonInit()
{
	calcValid = false;
	isRotated = false;

	int n;
	for (n = 0; n < sizeof(s)/sizeof(s[0]); n++)
	{
		s[n] = 0.0;
	}

	for (n = 0; n < sizeof(this->a)/sizeof(this->a[0]); n++)
	{
		a[n] = 0.0;
	}

	rot1 = 0.0;
	rot2 = 0.0;
	rot3 = 0.0;
	rot1Sign = 0.0;
	rot2Sign = 0.0;
	rot3Sign = 0.0;
	offsetZ = 0.0;

	// Default
	numVertices = 3;
}

ThreePlane::ThreePlane(void)
{
	CommonInit();
}

ThreePlane::ThreePlane( double x1, double y1, double z1, double x2, double y2, double z2, double x3, double y3, double z3 )
{
	Reset( x1, y1, z1, x2, y2, z2, x3, y3, z3 );
}

void ThreePlane::Reset(  double x1, double y1, double z1, double x2, double y2, double z2, double x3, double y3, double z3 )
{
	CommonInit();
	v[0].Set( x1, y1, z1 );
	v[1].Set( x2, y2, z2 );
	v[2].Set( x3, y3, z3 );
	numVertices = 3;
	Calc();
}

void ThreePlane::Reset()
{
	CommonInit();
}

ThreePlane::~ThreePlane(void)
{
}

// Calculate segments and angles
void ThreePlane::Calc()
{
	// Determine segment lengths
	int n, nNext;
	for (n = 0; n < numVertices; n++)
	{
		nNext = this->GetNextComponentIndex( n );
		s[n] = v[nNext] - v[n];
	}

	// Derive angles
	CalcAngles();

	calcValid = true;
}

// Scale segments by specified amount. Points become invalid
void ThreePlane::Scale( double scaleFactor )
{
	int n;
	for (n = 0; n < numVertices; n++)
	{
		s[n] *= scaleFactor;
	}
}

void ThreePlane::CalcAngles()
{
	/**
	// Calculate vertex angles using law of cosines
	// Given known lengths for all three sides,
	// derive using law of cosines.
	// Given a triangle with sides a, b and c,
	// and an angle A facing side a, B facing b,
	// and C facing c, we know that
	// b^2 = a^2 + c^2 - 2ac * cos( B )
	// 0 = a^2 - b^2 + c^2 - 2ac * cos(B)
	// a^2 - b^2 + c^2 = 2ac * cos(B)
	// (a^2 - b^2 + c^2) / 2ac = cos(B)
	// and therefore B = acos( (a^2 - b^2 + c^2) / 2ac )
	// See http://mathworld.wolfram.com/LawofCosines.html

	// We can also use the law of sines once we have
	// one angle:
	// sin(A) / a == sin(B) / b == sin(C) / c
	// See http://mathworld.wolfram.com/LawofSines.html

**/
	// Use identities to simplify references to law of cosines / law of sines
	long double sa = s[GetOppositeSegmentIndex(0)]; // Opposite angle A
	long double sb = s[GetOppositeSegmentIndex(1)]; // Opposite angle B
	long double sc = s[GetOppositeSegmentIndex(2)]; // Opposite angle C

	double* aA = &a[0];
	double* aB = &a[1];
	double* aC = &a[2];

	long double sb13 = 0.0;
	// If there are four sides, we need to calculate sb as the diagonal connecting s[0] and s[2]
	// Also the rules for determining the opposite segment don't apply... sc is s[0] not s[3]
	if (this->numVertices > 3)
	{
		sb13 = v[2] - v[0];
		sb = sb13;
		sc = s[0];
	}

	// Get value to find arccos
	long double dCos = (sa * sa - sb * sb + sc * sc) / (2 * sa * sc);

	// Solve for coefficient of right side, which is cos(B)
	*aB = acos( dCos );

	// Now get ratio to apply law of sines. This is also 1/2R where R
	// is the radius of the circumcircle (see http://mathworld.wolfram.com/Circumcircle.html)
	//long double dRatio = sin( *aB ) / sb;

	// Solve other angles
	// FIXME Use vector analysis - we had BAD LOGIC which used the law of sines, which does NOT work
	// with certain oblique triangles!!!
	// sin(A) / a == dRatio, therefore
	// sin(A) == a * dRatio, therefore
	// A = asin( a * dRatio )
	//*aA = asin( sa * dRatio );
	dCos = (sb * sb - sa * sa + sc * sc) / (2 * sb * sc);
	*aA = acos( dCos );
	//CDbg::Out( "ThreePlane::CalcAngles():\n\tratio=%.12f, aA=%.12f, a2=%.12f\n", dRatio, *aA, a2 );
	//*aA = a2;
	dCos = (sa * sa - sc * sc + sb * sb) / (2 * sa * sb);
	double c2 = acos( dCos );
	*aC = CFace::m_pi - *aA - *aB;
	CDbg::Out( "ThreePlane::CalcAngles():\n\taA=%.12f, aB=%.12f\n\taC=%.12f, c2=%.12f\n", 
		*aA, *aB, *aC, c2 );

	// If there are only three vertices, we're done
	if (this->numVertices == 3)
	{
		return;
	}

	// We only have one angle, v2, and all four side lengths.
	// We also have the length of the diagonal opposite v2 connecting v1-v3.
	// The law of cosines gives us b^2 = a^2 + c^2 - 2ac * cos( B )
	// Therefore B = acos( (a^2 - b^2 + c^2) / 2ac )
	dCos = (s[3] * s[3] - sb13 * sb13 + s[2] * s[2]) / (2 * s[3] * s[2]);
	a[3] = acos( dCos );

	// Use the law of cosines to calculate the angle at v[0]
	//long double sb2 = v[3] - v[1];
	//dCos = (s[0] * s[0] - sb2 * sb2 + s[3] * s[3]) / (2 * s[0] * s[3]);
	//a[0] = acos( dCos );

	// Calculate the other angle which complements A1 at v[0]
	dCos = (sb13 * sb13 - s[2] * s[2] + s[3] * s[3]) / (2 * sb13 * s[3]);
	double aC2 = acos( dCos );
	// Add to A1 to determine a[0]
	a[0] += aC2;

	// Make alternate calculation of A2
	dCos = (sb13 * sb13 - s[3] * s[3] + s[2] * s[2]) / (2 * sb13 * s[2]);
	double aA2 = acos( dCos );
	double a2Old = a[2];

	// The complements of the angles of a polygon add up to 2*pi.
	// The angles themselves add up to 2*pi for a quadrilateral.
	// Get the remaining angle by subtraction.
	a[2] = CFace::m_pi_x_2 - a[0] - a[1] - a[3];

	CDbg::Out( "C2=%.12f, A2=%.12f\na[2]=%.12f, a[2]alt=%.12f\n", aC2, aA2, a[2], aA2+a2Old );
	/**
	// Get the ratio for the second triangle which shares the original (putative) side 3
	dRatio = sin( *aB ) / sb;
	// By the law of sines, sin(a[3])/sb = sin(alpha)/s[2] = sin(beta)/s[3]
	// Calculate angle alpha which is opposite the actual side 3
	*aC = asin( CFace::ModuloPi( dRatio * sc ) );
	*aA = CFace::m_pi - *aC - *aB;

	// The actual measure of a[0] is for the triangle - add alpha
	this->a[0] += alpha;
	this->a[2] += beta;
	**/
}

// Rotate to make z=0 true. v1, v2 and v3 are left unchanged, with the rotated
// values in rv1, rv2, and rv3
void ThreePlane::Rotate()
{
	// Calculate rotation angles rotX, rotY, and rotZ (in radians)
	isRotated = true;

	// v1 is the pivot point for all rotations
	// Pivot axis 1 is parallel to the y axis, combining all y with v1.x, v1.z
	// This rotation will leave v1 and v3 with z=v1.z
	// This rotation will be saved as rot1 with direction applied to z in the direction rot1Sign and 
	// affects x and z for the rotated point.
	this->pivot1xz = this->v[0];
	pivot1xz.y = v[2].y;
	// Calculate hypotenuse of right triangle for pivot where 
	// r3 will be the angle and the opposite leg will be offsetZ
	double hypPivot1v3 = pivot1xz - v[2];
	// Calculate first rotation
	if (hypPivot1v3 == 0.0 || v[2].z == pivot1xz.z)
	{
		this->rot1 = 0.0;
		this->rot1Sign = 0.0;
	}
	else
	{
		this->rot1 = asin( abs( v[2].z - pivot1xz.z ) / hypPivot1v3 );
		this->rot1Sign = (v[2].z < pivot1xz.z) ? 1.0 : -1.0;
	}

	// Rotate second point
	// FIXME handle quadrants!!!
	ThreePoint rv2( v[1] );
	pivot1xz.y = v[1].y;
	double hypPivot1v2 = pivot1xz - v[1];
	if (hypPivot1v2 != 0.0 && v[1].z != pivot1xz.z)
	{
		// Calculate angle from pivot-v2 (Pivot Origin) to x intercept for v1
		double aPOz = asin( v[1].z / hypPivot1v2 );
		// Calculate angle from new pivot-rv2 (Pivot Rotated) to x intercept
		double aPRz = this->rot1 - aPOz;
		// With angle and hypotenuse, get opposite and adjacent sides
		rv2.z = v[0].z + hypPivot1v2 * sin( aPRz );
		rv2.x = v[0].x + hypPivot1v2 * cos( aPRz );
	}

	// Calculate second rotation
	// Pivot axis 2 is parallel to the z axis, combining all z with v1.x, v1.y
	// This rotation will leave v1 and v3 with

	// Pivot axis 2 is parallel to the x axis, combining all x with v1.y, v1.z
	// This rotation will be saved as rot2 with direction applied to z in the direction rot2Sign
	// and affects y and z for the rotated point.
	this->pivot2yz = this->v[0];
	/******** Construction in progress
	pivot2yz.x = this->v2.x;
	if (hypPivot1v2 == 0.0 || v2.z == pivot1xz.z)
	{
		this->ro;
	}
	ThreePoint pivot12( this->v1 );
	pivot12.y = v2.y;

	// Calculate third rotation
	// Pivot axis 3 is parallel to the z axis, combining all z with v1.x, v1.y
	// Third rotation is not needed
	****************** end construction zone **************/

	// Apply rotation to [copies of] points
	this->rv1 = this->v[0];
	Translate( rv1 );
	this->rv2 = this->v[1];
	Translate( rv2 );
	this->rv3 = this->v[2];
	Translate( rv3 );

}

// Apply rotation and translate another point
void ThreePlane::Translate( ThreePoint& p )
{
	if (!isRotated)
	{
		Rotate();
	}
}

// Remove translation from a point
void ThreePlane::UnTranslate( ThreePoint& p )
{
	if (!isRotated)
	{
		Rotate();
	}
}

// Add vertex. Currently only 4 are supported
ThreePoint& ThreePlane::AddVertex(ThreePoint const& v) //throws const char*
{
	if (numVertices >= ThreePlane::MAX_VERTICES)
	{
		throw("Limit exceeded, only 4 vertices allowed");
	}
	// FIXME This would be a good place to ensure that the new vertex is coplanar
	this->v[numVertices] = v;
	numVertices++;
	return this->v[numVertices-1];
}

// Vertex accessors, index origin:0. Throws exception if vertex has not been created.
// Use AddVertex() to add points beyond 3.
ThreePoint& ThreePlane::vtx(int index) // throws const char*
{
	if (index < 0 || index >= numVertices)
	{
		throw("Invalid vertex index");
	}
	return this->v[index];
}

// Get vertex count
int ThreePlane::GetNumVertices() const
{
	return this->numVertices;
}

// Get angle corresponding to vertex
double ThreePlane::GetAngle(int n) const
{
	if (n < 0 || n >= this->numVertices) throw( "Invalid angle index" );
	return this->a[n];
}

// Get angle in degrees
double ThreePlane::GetAngleDeg(int n) const
{
	return CFace::RadansToDeg( GetAngle(n) );
}

// Segment accessor (origin:0). Segments follow vertices
double ThreePlane::GetSeg( int n ) const
{
	if (n < 0 || n >= this->numVertices) throw( "Invalid segment index" );
	return this->s[n];
}

// Get mean segment length
double ThreePlane::GetMeanSeg() const
{
	if (this->numVertices == 0)
	{
		return 0.0;
	}
	return GetSumOfSegs() / this->numVertices;
}

// Get sum of segment lengths
double ThreePlane::GetSumOfSegs() const
{
	double d = 0.0;
	int n;
	for (n = 0; n < this->numVertices; n++)
	{
		d += this->s[n];
	}
	return d;
}

// Get index of segment opposite angle (only applies to triangles)
int ThreePlane::GetOppositeSegmentIndex( int angleIndex ) const
{
	// The adjacent segment in a triangle always has the same index,
	// therefore the next segment will be the opposite (only for triangles)
	// We do not handle polygons with n sides where n > 4, and for n==4
	// there is no opposite segment.
	return GetNextComponentIndex( angleIndex );
}

// Get index of next angle or segment
int ThreePlane::GetNextComponentIndex( int index ) const
{
	return (index + 1) % this->numVertices;
}

// Get index of previous angle or segment
int ThreePlane::GetPrevComponentIndex( int index ) const
{
	return (index + this->numVertices - 1) % this->numVertices;
}


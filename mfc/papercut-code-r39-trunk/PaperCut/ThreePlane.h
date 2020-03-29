/*!

	@file	 ThreePlane.h
	
	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id$

  Class representing a plane in 3-space. Requires definition of three points.

*/

#pragma once

class ThreePoint;

class ThreePlane
{
public:
	ThreePlane(void);
	ThreePlane( double x1, double y1, double z1, double x2, double y2, double z2, double x3, double y3, double z3 );

	~ThreePlane(void);

	// Rotate to make z=0 true. v1, v2 and v3 are left unchanged, with the rotated
	// values in rv1, rv2, and rv3
	void Rotate();

	// Apply rotation and translate another point
	void Translate( ThreePoint& p );

	// Remove translation from a point
	void UnTranslate( ThreePoint& p );

	// Calculate segments and angles
	void Calc();

	// Scale segments by specified amount. Points become invalid
	void Scale( double scaleFactor );

	// Add vertex. Currently only 4 are supported
	ThreePoint& AddVertex(ThreePoint const& v); //throws const char*

	// Vertex accessors, index origin:0. Throws exception if vertex has not been created.
	// Use AddVertex() to add points beyond 3.
	ThreePoint& vtx(int index); // throws const char*

	// Rotated vertex accessors
	ThreePoint const& GetRotatedv1() const;
	ThreePoint const& GetRotatedv2() const;
	ThreePoint const& GetRotatedv3() const;

	// Segment accessor (origin:0). Segments follow vertices
	double GetSeg( int n ) const;

	// Get mean segment length
	double GetMeanSeg() const;

	// Get sum of segment lengths
	double GetSumOfSegs() const;

	// Get index of segment opposite angle (only applies to triangles)
	int GetOppositeSegmentIndex( int angleIndex ) const;

	// Get index of next angle or segment
	int GetNextComponentIndex( int index ) const;

	// Get index of previous angle or segment
	int GetPrevComponentIndex( int index ) const;

	// Get angle corresponding to vertex
	double GetAngle(int n) const;

	// Get angle in degrees
	double GetAngleDeg(int n) const;

	// Get vertex count
	int GetNumVertices() const;

	// Reset to three vertices - values must be set
	void Reset();

	// Reset to specified three vertices
	void Reset( double x1, double y1, double z1, double x2, double y2, double z2, double x3, double y3, double z3 );

	// Flip orientation to opposite side of the page (clockwise -> counter-clockwise and vice versa)
	// Recalculates all angles
	void Flip();

	// Maximum vertices allowed
	enum {
		MAX_VERTICES = 4
	};
protected:
	// Calculate angles
	void CalcAngles();

	// Common initialization called by all constructors
	void CommonInit();

	// Number of vertices (default = 3)
	int numVertices;

	// Vertices defining the triangle / plane
	ThreePoint v[4];

	// true if rotation has been done
	bool isRotated;

	// true if segments and angles are valid
	bool calcValid;

	// Perform a single translation on point relative to pivot
	// Segments derived from vertices
	double s[4];

	// Angles in radians corresponding to vertices
	double a[4];

	// Rotation in radians on three axes to satisfy z=0
	double rot1, rot2, rot3;

	// Sign of rotations relative to z
	double rot1Sign, rot2Sign, rot3Sign;

	// Pivot points for rotation operations
	ThreePoint pivot1xz; // x, z significant
	ThreePoint pivot2yz; // y, z significant
	ThreePoint pivot3xy; // x, y significant

	// Translation offset for z
	double offsetZ;

	// Rotated and translated vertices
	ThreePoint rv1;
	ThreePoint rv2;
	ThreePoint rv3;

};

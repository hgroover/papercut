/*!

	@file	 ThreePoint.cpp
	
	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id$

  Class representing a single point in 3-space. Supports operations such as distance between two points.

*/

#include "stdafx.h"
#include ".\threepoint.h"

ThreePoint::ThreePoint(void)
{
	x = 0.0;
	y = 0.0;
	z = 0.0;
}

ThreePoint::~ThreePoint(void)
{
}

ThreePoint::ThreePoint( double xa, double ya, double za )
{
	x = xa;
	y = ya;
	z = za;
}

ThreePoint::ThreePoint( ThreePoint const& source )
{
	*this = source;
}

// Set position
void ThreePoint::Set( double xa, double ya, double za )
{
	x = xa;
	y = ya;
	z = za;
}

// Find distance between two points
double ThreePoint::operator-(ThreePoint const& subtrahend)
{
/**
	Given any two points x1,y1,z1 and x2,y2,z2 with the single-axle distances represented as
dx, dy, and dz
dx = abs(x1-x2)
dy = abs(y1-y2)
dz = abs(z1-z2)

the distance between the points is sqrt(dx*dx + dy*dy + dz*dz)
**/
	long double dx = this->x - subtrahend.x;
	long double dy = this->y - subtrahend.y;
	long double dz = this->z - subtrahend.z;

	// Sign makes no difference since we are squaring all the terms
	long double res = sqrt( dx*dx + dy*dy + dz*dz );
	return (double)res;
}

// Copy point
ThreePoint& ThreePoint::operator=(ThreePoint const& source)
{
	Set( source.x, source.y, source.z );
	return *this;
}


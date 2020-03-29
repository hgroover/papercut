/*!

	@file	 ThreePoint.h
	
	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id$

  Class representing a single point in 3-space. Supports operations such as distance between two points.

*/

#pragma once

class ThreePoint;

class ThreePoint
{
public:
	ThreePoint(void);
	ThreePoint( double xa, double ya, double za );
	ThreePoint( ThreePoint const& source );
	~ThreePoint(void);

	// Set position
	void Set( double xa, double ya, double za );

	double x, y, z;

	// Find distance between two points
	double operator-(ThreePoint const& subtrahend);

	// Copy point
	ThreePoint& operator=(ThreePoint const& source);
};

/*!

	@file	 TrapezoidMidpoint.cpp

	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id$

	This is a barebones class for wrapping the calculation of derived angles and lengths
	from a trapezoid midpoint. It makes use of Varignon parallelograms, the SSS theorem,
	and the SAS theorem. 

	Terms used reference the diagram TrapezoidMidpoint.gif

	For further details, see:
	http://mathworld.wolfram.com/VarignonParallelogram.html
	http://mathworld.wolfram.com/SASTheorem.html
	http://mathworld.wolfram.com/LawofCosines.html
	http://mathworld.wolfram.com/SSSTheorem.html

*/

#include "stdafx.h"
#include "TrapezoidMidpoint.h"
#include "Face.h"	///< Use this to get pi and degrees-to-radans

#include <math.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TrapezoidMidpointCalc::TrapezoidMidpointCalc()
{
	// Clear all inputs
	ResetInputs();

	// Clear output
	I = 0.0;
	J = 0.0;
	K = 0.0;
	L = 0.0;
}

TrapezoidMidpointCalc::~TrapezoidMidpointCalc()
{
}

void TrapezoidMidpointCalc::ResetInputs()
{
	a = b = c = d = 0.0;
	alpha = gamma = delta = theta = 0.0;
}

bool TrapezoidMidpointCalc::HasInputs()
{
	return (a > 0.0 && b > 0.0 && c > 0.0 && d > 0.0
			&& alpha > 0.0 && gamma > 0.0 && delta > 0.0 && theta > 0.0);
}

bool TrapezoidMidpointCalc::Calculate()
{
	if (!HasInputs())
	{
		return false;
	}
	/*!
	// To construct a pyramid, we need a center. Take the intersection
	// of the lines joining opposite midpoints as the center.
	// We can take advantage of the Varignon Parallelogram http://mathworld.wolfram.com/VarignonParallelogram.html
	// Inscribing a quadrilateral which joins the midpoints of adjacent sides of a quadrilateral
	// always forms a parallelogram. Also see http://mathworld.wolfram.com/VarignonsTheorem.html for
	// other interesting properties, such as the area of the Varignon parallelogram for a convex quadrilateral
	// is half that of the quadrilateral itself.
	// See trapezoid-midpoint-calc.gif for labels.
	*/

	alpha_r = CFace::DegToRadans(alpha);
	gamma_r = CFace::DegToRadans(gamma);
	delta_r = CFace::DegToRadans(delta);
	theta_r = CFace::DegToRadans(theta);

	/*!
	The base edge is c in this diagram.
	We need the lengths of I, J, K and L.
	Segment x connects the midpoints of b and d, and segment y connects the midpoints of a and c.
	The Varignon parallelogram connecting the midpoints of a, b, c, and d has legs of lengths g and h.
	Angles alpha, gamma, delta, and theta are all known. The half-lengths of a, b, c and d are
	m, n, p, and q respectively. The half-lengths of x and y are z and o, respectively.
	*/
	this->m = a/2.0;
	this->n = this->b / 2.0;
	p = this->c / 2.0;
	q = d / 2.0;

	/*!

	The following derivations make use of the SAS theorem and law of cosines, described at
	http://mathworld.wolfram.com/SASTheorem.html and http://mathworld.wolfram.com/LawofCosines.html
	g = sqrt( n*n + p*p - 2 * n * p * cos( alpha ) );
	h = sqrt( p*p + q*q - 2 * p * q * cos( gamma ) );

	*/
	g = sqrt( n*n + p*p - 2 * n * p * cos( alpha_r ) );
	h = sqrt( p*p + q*q - 2 * p * q * cos( gamma_r ) );

	/*!
	We derive these angles using the SSS theorem and the law of cosines, described at
	http://mathworld.wolfram.com/SSSTheorem.html
	phi = acos( ( g*g + p*p - n*n ) / (2 * g * p) );
	mu = acos( ( h*h + p*p - q*q ) / (2 * h * p) );
	rho = acos( (m*m + g*g - q*q) / (2 * m * g) );
	tau = acos( (m*m + h*h - n*n) / (2 * m * h) );

	*/
	this->phi = CFace::RadansToDeg( acos( (g*g + p*p - n*n) / (2*g*p) ) );
	mu = CFace::RadansToDeg( acos( (h*h + p*p - q*q) / (2*h*p) ) );
	rho = CFace::RadansToDeg( acos( (m*m + g*g - q*q) / (2*m*g) ) );
	tau = CFace::RadansToDeg( acos( (m*m + h*h - n*n) / (2*m*h) ) );

	/*!
	We can derive angle W as
	W = 180 - mu - phi
	and derive x using the law of cosines:
	x = sqrt( h*h + g*g - 2*h*g * cos( W ) );
	z = x/2;
	*/
	this->W = 180 - mu - phi;
	this->x = sqrt( h*h + g*g - 2*h*g * cos( CFace::DegToRadans( W ) ) );
	this->z = x/2.0;

	/*!
	More angles we need are
	chi = 180 - alpha - phi
	xi = 180 - theta - tau
	*/
	this->chi = 180.0 - alpha - phi;
	this->xi = 180.0 - theta - tau;

	/*!
	And we can derive
	eta = 180 - xi - chi
	and can derive y using the law of cosines:
	y = sqrt( h*h + g*g - 2*h*g * cos( eta ) );
	o = y/2;

	*/
	this->eta = 180.0 - xi - chi;
	y = sqrt( h*h + g*g - 2*h*g * cos( CFace::DegToRadans( eta ) ) );
	o = y/2.0;

	/*!
	Now we can derive the angles on either side of the bimeridian y using SSS and the law of cosines:
	T = phi + acos( (o*o + g*g - z*z) / (2 * o * g) );
	R = 180 - T;
	E = rho + acos( (o*o + g*g - z*z) / (2 * o * g) );
	F = 180 - E;

	*/
	this->T = phi + CFace::RadansToDeg( acos( (o*o + g*g - z*z) / (2*o*g) ) );
	R = 180.0 - T;
	E = rho + CFace::RadansToDeg( acos( (o*o + g*g - z*z) / (2*o*g) ) );
	F = 180.0 - E;

	/*!
	And we can use SAS to derive our final results:
	I = sqrt( o*o + p*p - 2*o*p * cos( R ) );
	J = sqrt( o*o + p*p - 2*o*p * cos( T ) );
	K = sqrt( o*o + m*m - 2*o*m * cos( E ) );
	L = sqrt( o*o + m*m - 2*o*m * cos( F ) );

	*/
	I = sqrt( o*o + p*p - 2*o*p * cos( CFace::DegToRadans( R ) ) );
	J = sqrt( o*o + p*p - 2*o*p * cos( CFace::DegToRadans( T ) ) );
	K = sqrt( o*o + m*m - 2*o*m * cos( CFace::DegToRadans( E ) ) );
	L = sqrt( o*o + m*m - 2*o*m * cos( CFace::DegToRadans( F ) ) );

	return true;
}

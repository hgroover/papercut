/*!

	@file	 TrapezoidMidpoint.h
	
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

#if !defined(TRAPEZOID_MIDPOINT_H_INCLUDED)
#define TRAPEZOID_MIDPOINT_H_INCLUDED

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class TrapezoidMidpointCalc
{
public:
	TrapezoidMidpointCalc();
	~TrapezoidMidpointCalc();

	// Input variables
	/// Outer legs of trapezoid
	double a, b, c, d;
	/// Inside angles of trapezoid, in degrees, starting with angle to left of leg a and going clockwise
	double delta, theta, alpha, gamma;

	/// Check to see whether inputs are valid
	bool HasInputs();

	/// Reset inputs
	void ResetInputs();

	/// Calculate derived values. Return true if successful
	bool Calculate();

	// Derived values
	/// Half of a, b, c, d respectively
	double m, n, p, q;
	/// Legs of Varignon parallelogram
	double g, h;
	/// Bimedians (legs connecting opposite midpoints)
	/// Joins midpoint(a) with midpoint(c)
	double y;
	/// joins midpoint(b) with midpoint(d)
	double x;
	/// Half of x and y respectively
	double z, o;

	/// Derived angles (in degrees) of angles formed by Varignon parallelogram legs, midpoints, and trapezoid legs
	double mu, phi, xi, chi, rho, tau, eta;
	/// Derived angles (in degrees) of other angles
	double E, F, T, R, W;

	/// Legs joining delta, theta, alpha and gamma respectively to center - this is what we're solving for
	double K, L, J, I;

protected:
	/// Inside angles in radans
	double alpha_r, gamma_r, delta_r, theta_r;
};

#endif

/*!

	@file	 LogRect.cpp
	
	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: LogRect.cpp 9 2006-03-08 14:41:10Z henry_groover $

  The logical rectangle object uses floating point dimensions and is used to describe
  available areas for layout operations.

*/

#include "stdafx.h"
#include "PaperCut.h"
#include "LogRect.h"

#include <math.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLogRect::CLogRect() :
	m_Max(),
	m_Min(),
	m_Org()
{
	m_dOrientation = 0;
}

CLogRect::CLogRect( double minx, double miny, double maxx, double maxy,
				   double orgx /*= 0*/, double orgy /*= 0*/, double orientation /*=0*/ ) :
	m_Max( maxx, maxy ),
	m_Min( minx, miny ),
	m_Org( orgx, orgy )
{
	m_dOrientation = orientation;
} // explicit value constructor

CLogRect::CLogRect( CLogPoint const& pMin, CLogPoint const& pMax ) :
	m_Min( pMin ),
	m_Max( pMax )
{
	m_dOrientation = 0;
}

CLogRect::CLogRect( CLogPoint const& pMin, CLogPoint const& pMax, CLogPoint const& Org ) :
	m_Min( pMin ),
	m_Max( pMax ),
	m_Org( Org )
{
	m_dOrientation = 0;
}

CLogRect::CLogRect( CLogRect const& r )
{
	(*this) = r;
} // copy constructor

CLogRect const&
CLogRect::operator = ( CLogRect const& r )
{
	m_Max = r.m_Max;
	m_Min = r.m_Min;
	m_Org = r.m_Org;
	m_dOrientation = r.m_dOrientation;
	return (*this);
} // operator =

CLogRect::~CLogRect()
{

}

CLogRect
CLogRect::operator + ( CLogRect const& r )
{
	// FIXME This will only work if both have the same
	// orientation!

	// Always add to right
	CLogRect sum( *this );
	sum.m_Max += r.m_Max;
	sum.m_Org = m_Org;
	return sum;
} // operator +

CLogRect
CLogRect::operator - ( CLogRect const& r )
{
	// FIXME This will only work if both have the same
	// orientation!

	// Always subtract from right
	CLogRect sum( *this );
	sum.m_Max -= r.m_Max;
	sum.m_Org = m_Org;
	// We should at least have zero dimensions
	sum.m_Max.m_dX = __max( sum.m_Max.m_dX, sum.m_Min.m_dX );
	sum.m_Max.m_dY = __max( sum.m_Max.m_dY, sum.m_Max.m_dY );
	return sum;
} // operator -

double
CLogRect::GetWidthToHeight() const
{
	// Avoid divide by zero
	double dHeight = GetHeight();
	if (dHeight == 0.0)
	{
		return 0;
	}
	double dWidth = GetWidth();
	return dWidth / dHeight;
} // CLogRect::GetWidthToHeight()

static void
Swap( double& d1, double& d2 )
{
	double d = d1;
	d1 = d2;
	d2 = d;
} // Swap()

static void
Swap( CLogPoint& p1, CLogPoint& p2 )
{
	CLogPoint p = p1;
	p1 = p2;
	p2 = p;
}

// Rotate orientation 90 degrees
void
CLogRect::Rotate90()
{
	// Normalize first
	Normalize();
	this->m_dOrientation = fmod( m_dOrientation + 90, 360 );
	double dW = GetWidth();
	double dH = GetHeight();
	// Reposition max with coordinates swapped
	m_Max = m_Min + CLogPoint( dH, dW );
} // CLogRect::Rotate90()

// Adjust origin, min and max so that origin is at 0, 0
void CLogRect::Normalize()
{
	CLogPoint Adjust( m_Org );
	m_Org -= Adjust;
	m_Min -= Adjust;
	m_Max -= Adjust;
}

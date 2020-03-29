/*!

	@file	 LogPoint.cpp

	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: LogPoint.cpp 9 2006-03-08 14:41:10Z henry_groover $

	Simply a point with floating point coordinates in an arbitrary system.

*/

#include "stdafx.h"
#include "PaperCut.h"
#include "LogPoint.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLogPoint::CLogPoint()
{
	m_dX = 0;
	m_dY = 0;
}

CLogPoint::CLogPoint( double x, double y )
{
	m_dX = x;
	m_dY = y;
}

CLogPoint::CLogPoint( CLogPoint const& src )
{
	(*this) = src;
}

const CLogPoint&
CLogPoint::operator = (CLogPoint const& r )
{
	m_dX = r.m_dX;
	m_dY = r.m_dY;
	return (*this);
}

CLogPoint::~CLogPoint()
{

}

const CLogPoint&
CLogPoint::operator +=(CLogPoint const& r )
{
	m_dX += r.m_dX;
	m_dY += r.m_dY;
	return (*this);
}

const CLogPoint&
CLogPoint::operator +=(double d )
{
	m_dX += d;
	m_dY += d;
	return (*this);
}

const CLogPoint&
CLogPoint::operator -=(CLogPoint const& r )
{
	m_dX -= r.m_dX;
	m_dY -= r.m_dY;
	return (*this);
}

const CLogPoint&
CLogPoint::operator -=(double d )
{
	m_dX -= d;
	m_dY -= d;
	return (*this);
}

CLogPoint
CLogPoint::operator + (CLogPoint const& r ) const
{
	CLogPoint s( *this );
	s += r;
	return s;
}

CLogPoint
CLogPoint::operator - (CLogPoint const& r ) const
{
	CLogPoint s( *this );
	s -= r;
	return s;
}

// Set to lesser of dimensions
bool
CLogPoint::SetToLesser( CLogPoint const& r )
{
	bool bReturn = false;
	if (r.m_dY < m_dY)
	{
		bReturn = true;
		m_dY = r.m_dY;
	}
	if (r.m_dX < m_dX)
	{
		bReturn = true;
		m_dX = r.m_dX;
	}
	return bReturn;
} // CLogPoint::SetToLesser()

// Set to greater of dimensions
bool
CLogPoint::SetToGreater( CLogPoint const& r )
{
	bool bReturn = false;
	if (r.m_dY > m_dY)
	{
		bReturn = true;
		m_dY = r.m_dY;
	}
	if (r.m_dX > m_dX)
	{
		bReturn = true;
		m_dX = r.m_dX;
	}
	return bReturn;
} // CLogPoint::SetToGreater()

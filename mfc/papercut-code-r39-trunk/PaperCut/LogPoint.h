/*!

	@file	 LogPoint.h
	
	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: LogPoint.h 9 2006-03-08 14:41:10Z henry_groover $

	Simply a point with floating point coordinates in an arbitrary system.

*/

#if !defined(AFX_LOGPOINT_H__9827ACE1_48F6_11D6_A858_0040F4309CCE__INCLUDED_)
#define AFX_LOGPOINT_H__9827ACE1_48F6_11D6_A858_0040F4309CCE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CLogRect;

class CLogPoint  
{
	friend class CLogRect;
public:
	CLogPoint();
	CLogPoint( CLogPoint const& src );
	CLogPoint( double x, double y );
	virtual ~CLogPoint();

	// Basic operations
	CLogPoint operator + (CLogPoint const& r ) const;
	CLogPoint operator - (CLogPoint const& r ) const;

	const CLogPoint& operator = (CLogPoint const& r );
	const CLogPoint& operator +=(CLogPoint const& r );
	const CLogPoint& operator +=(double d );
	const CLogPoint& operator -=(CLogPoint const& r );
	const CLogPoint& operator -=(double d );


	// Comparisons.  These hold true iff BOTH components pass the test

	// Is my position less than r in both dimensions?
	bool operator <  ( CLogPoint const& r ) const { return (m_dX <  r.m_dX) && (m_dY <  r.m_dY); }
	// etc...
	bool operator <= ( CLogPoint const& r ) const { return (m_dX <= r.m_dX) && (m_dY <= r.m_dY); }
	bool operator == ( CLogPoint const& r ) const { return (m_dX == r.m_dX) && (m_dY == r.m_dY); }
	bool operator >= ( CLogPoint const& r ) const { return (m_dX >= r.m_dX) && (m_dY >= r.m_dY); }
	bool operator >  ( CLogPoint const& r ) const { return (m_dX >  r.m_dX) && (m_dY >  r.m_dY); }

	void Set( double x, double y ) { m_dX = x; m_dY = y; }
	void SetX( double x ) { m_dX = x; }
	void SetY( double y ) { m_dY = y; }

	// Set to lesser of dimensions.  Returns true if changed
	bool SetToLesser( CLogPoint const& r );

	// Set to greater of dimensions.  Returns true if changed
	bool SetToGreater( CLogPoint const& r );

	// Actual x and y components
	double m_dY;
	double m_dX;
};

#endif // !defined(AFX_LOGPOINT_H__9827ACE1_48F6_11D6_A858_0040F4309CCE__INCLUDED_)

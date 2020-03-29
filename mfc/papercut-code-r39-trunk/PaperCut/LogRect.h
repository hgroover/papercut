/*!

	@file	 LogRect.h
	
	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: LogRect.h 9 2006-03-08 14:41:10Z henry_groover $

  The logical rectangle object uses floating point dimensions and is used to describe
  available areas for layout operations.

*/

#if !defined(AFX_LOGRECT_H__7E443201_4471_11D6_A858_0040F4309CCE__INCLUDED_)
#define AFX_LOGRECT_H__7E443201_4471_11D6_A858_0040F4309CCE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "LogPoint.h"

class CLogRect;
class CPageGroup;

class CLogRect  
{
	// Declare ourselves as a friend
	friend class CLogRect;

	// CPageGroup is one class that manages CLogRect collections
	friend class CPageGroup;
public:
	CLogRect();
	CLogRect( double minx, double miny, double maxx, double maxy, double orgx = 0, double orgy = 0, double orientation = 0 );
	CLogRect( CLogPoint const& pMin, CLogPoint const& pMax );
	CLogRect( CLogPoint const& pMin, CLogPoint const& pMax, CLogPoint const& Org );
	CLogRect( CLogRect const& r );

	virtual ~CLogRect();

	// Comparisons

	// Will I fit inside r with room to spare?
	bool operator <  ( CLogRect& r ) const { return (GetWidth() <  r.GetWidth()) && (GetHeight() <  r.GetHeight()); }

	// Will I fit inside r?
	bool operator <= ( CLogRect& r ) const { return (GetWidth() <= r.GetWidth()) && (GetHeight() <= r.GetHeight()); }

	// Is r exactly of the same dimensions?
	bool operator == ( CLogRect& r ) const { return (GetWidth() == r.GetWidth()) && (GetHeight() == r.GetHeight()); }

	// Do I exceed or match r in both dimensions?
	bool operator >= ( CLogRect& r ) const { return (GetWidth() >= r.GetWidth()) && (GetHeight() >= r.GetHeight()); }

	// Do I exceed r in both dimensions?
	bool operator >  ( CLogRect& r ) const { return (GetWidth() >  r.GetWidth()) && (GetHeight() >  r.GetHeight()); }

	// Add other's dimensions to left and bottom
	CLogRect operator + ( CLogRect const& r );

	// Subtract other's dimensions from left and bottom
	CLogRect operator - ( CLogRect const& r );

	// Assignment
	CLogRect const& operator = ( CLogRect const& r );

	// Position change

	// Accessor - mutator

	virtual double GetWidth() const { return m_Max.m_dX - m_Min.m_dX; }
	virtual double GetHeight() const { return m_Max.m_dY - m_Min.m_dY; }

	virtual double GetMinX() const { return m_Min.m_dX; }
	virtual double GetMinY() const { return m_Min.m_dY; }
	virtual double GetMaxX() const { return m_Max.m_dX; }
	virtual double GetMaxY() const { return m_Max.m_dY; }

	virtual void GetMin( CLogPoint& P ) const { P = m_Min; }
	virtual CLogPoint const& GetMin( void ) const { return m_Min; }
	virtual void GetMax( CLogPoint& P ) const { P = m_Max; }
	virtual CLogPoint const& GetMax( void ) const { return m_Max; }

	virtual double GetOrgX() const { return m_Org.m_dX; }
	virtual double GetOrgY() const { return m_Org.m_dY; }

	virtual void GetOrg( CLogPoint& P ) const { P = m_Org; }
	virtual CLogPoint const& GetOrg( void ) const { return m_Org; }

	virtual double GetOrientation() const { return m_dOrientation; }

	virtual double GetWidthToHeight() const;

	virtual void SetOrigin( double x, double y ) { m_Org.m_dX = x; m_Org.m_dY = y; }
	virtual void SetOrigin( CLogPoint const& P ) { m_Org = P; }

	// Rotate orientation 90 degrees
	virtual void Rotate90();

	// Adjust origin, min and max so that origin is at 0, 0
	void Normalize();
protected:
	CLogPoint m_Org;
	CLogPoint m_Min;
	CLogPoint m_Max;
	double m_dOrientation;

};

#endif // !defined(AFX_LOGRECT_H__7E443201_4471_11D6_A858_0040F4309CCE__INCLUDED_)

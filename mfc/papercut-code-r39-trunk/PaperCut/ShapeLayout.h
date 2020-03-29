/*!

	@file	 ShapeLayout.h
	
	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: ShapeLayout.h 9 2006-03-08 14:41:10Z henry_groover $

  ShapeLayout.h: interface for the CShapeLayout class.

*/

#if !defined(AFX_SHAPELAYOUT_H__DC3032A1_38C4_11D6_A858_0040F4309CCE__INCLUDED_)
#define AFX_SHAPELAYOUT_H__DC3032A1_38C4_11D6_A858_0040F4309CCE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Vertex.h"
#include "Edge.h"
#include "Face.h"
#include "Shape.h"
#include "LogPoint.h"

#include <afxtempl.h>

class CPageGroup;

class CShapeLayout  
{
public:
	void GetBoundingMaxRectangleFor( int nFaceGroup, double& dWidth, double& dHeight, double& dMinX, double& dMinY );
	void GetBoundingRectangleForAllPages( double& dWidth, double& dHeight, double& dMinX, double& dMinY );
	void Reset( CShape* pShape );
	int CalculateAllBoundingRectangles();
	int GetFaceGroupCount();
	CShapeLayout( CShape* pShape );
	virtual ~CShapeLayout();

	// Return true if not in the middle of drawing
	bool IsValid();

	// Get bounding rectangle enclosing entire laid out shape
	// for face group n (origin:0)
	void GetBoundingRectangleFor( int nFaceGroup, double& dWidth, double& dHeight, double& dMinX, double& dMinY );

	// Do a single join.  Return 1 if successful, 0 if not found, -1 if error
	int Join( const char *pszSourceFaceEdge );

	// Automatically join laid out faces with specified number of
	// target pages.  Returns actual number, which may be less.
	int AutoJoin( int nMaxPerPage );

	// Get minimum x and y as well as width
	void GetMinMaxWidth( CArray<CLogPoint,CLogPoint const&>& a,
				 double& dWidth, double& dHeight, CLogPoint& MinPoint );

	// Access a face in m_aDiscreteFaces by ordinal
	CFace* GetBaseFace( int nIndex );

	// Map of popped joins
	CMap<CString,const char*,CString,const char*> m_mapPopped;

	// Map of possible orphan faces.  Facename maps to number of removals.
	CMap<CString,const char*,int,int> m_mapOrphanCandidates;

	CShape* m_pShape;
	CFace* m_pBaseOrientationFace;
	CArray<CEdge*,CEdge*> m_pCutEdges;

	// Get number of pages
	int GetPageCount() const { return m_aPages.GetSize(); }

	// Get specified page group
	CPageGroup* GetPage( int nPage ) const;

	// Remove and free all page groups
	void ResetPageGroups();

	// Save layout properties in symbolic form
	CString SaveSymbolic();

	// Restore layout properties
	static void RestoreSymbolic( LPCTSTR lp, double& dPageWidthInLogicalUnits, double& dPageWidthOverHeight );

	// Page width in logical units (from view)
	double m_dPageWidthInLogicalUnits;

	// Page width / page height (0.762 for 8.5X11)
	double m_dPageWidthOverHeight;

protected:
	// Array of discrete faces which are not recursively
	// joined to others.  Created by AutoLayout(), used by
	// GetAllBoundingRectangles() and GetBaseFace()
	CArray<CFace*,CFace*> m_aDiscreteFaces;

	// Page groups
	CArray<CPageGroup*,CPageGroup*> m_aPages;

	// Map of face pointers to m_aDiscreteFaces indices
	// Presence in this map indicates a face has already
	// been joined.  Created by AutoLayout()
	CMap<CFace*,CFace*,int,int> m_mapFaceGroup;

public:
	// Faces in shape are rotated to fit the planar orientation
	// of the net layout.  Each face has an orientation with respect
	// to a "north pole" vertex of the polyhedron.  If the face
	// forms the base of a regular pyramid with the north pole vertex
	// at the apex, the orientation is arbitrary.
	// Note that these layout face rotations are ADDED to the intrinsic
	// orientation of each face.
	CMap<CFace*,CFace*,float,float> m_mapFaceRotations;

	// A simple vector containing degrees added to multivertices
	CArray<double,double> m_adMultiVertexAddedAngles;

	// Collection of all faces not yet laid out (by name)
	CMap<CString,const char*,CFace*,CFace*> m_mapFacePool;

	// Final collection of faces with tweaked edge joins
	CArray<CFace*,CFace*> m_apFaceLayout;

	double m_dScale;
	double m_dOrientation;

	// Orientation at which bounding values calculated
	double m_dBoundingCalcOrientation;

	// Set of convex hull logical points at bounding calc orientation
	CArray<CLogPoint,CLogPoint const&> m_aConvexHull;

protected:

	CLogPoint m_LogSummaryMin;
	double m_dLogSummaryHeight;
	double m_dLogSummaryWidth;
	bool m_bValid;

	// These correspond to m_aDiscreteFaces
	CArray<CLogPoint,CLogPoint const&> m_aLogOrigin;
	CArray<CLogPoint,CLogPoint const&> m_aLogMax;
	CArray<CLogPoint,CLogPoint const&> m_aLogMin;
	CArray<double,double> m_adLogHeight;
	CArray<double,double> m_adLogWidth;
public:
	double GetMinimalJoinAngle() const;
	double GetTabShoulderAngle() const;
	static double m_dDefaultMinimalJoinAngle;
	static double m_dDefaultTabShoulderAngle;
	CArray<double,double> m_adOptimalOrientation;
protected:
	// Free all page groups
	void FreeAllPages();

	// Join a face and all connected faces
	int RecursiveJoin( CFace* p, bool bDepthFirst, int& nMaxFaces );
};

#endif // !defined(AFX_SHAPELAYOUT_H__DC3032A1_38C4_11D6_A858_0040F4309CCE__INCLUDED_)

/*!

	@file	 Edge.h

	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: Edge.h 16 2006-04-15 06:39:12Z henry_groover $

	Edge object. Edges are attached to vertices and belong to faces.

*/

#if !defined(AFX_EDGE_H__9837ABD9_3851_11D6_A858_0040F4309CCE__INCLUDED_)
#define AFX_EDGE_H__9837ABD9_3851_11D6_A858_0040F4309CCE__INCLUDED_

#include "Vertex.h"	// Added by ClassView

#include <afxtempl.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CFace;
class CEdge;
class CVertex;

class CEdge  
{
public:
	CEdge( CFace* pOwner );
	CEdge( CFace* pOwner, const char *pszName );
	virtual ~CEdge();

protected:
	// These members should ONLY be called from CFace!!!
	friend class CFace;
	CEdge( CEdge const& src );
	CEdge& operator =(const CEdge &src);
public:
	bool m_bConcave;

	// Get relationship with vertex.  Return values:
	// -1 Vertex precedes edge
	//  0 No relation
	//  1 Vertex follows edge
	int GetVertexRelation( CVertex* pv );

	// Join outside of an edge with another edge.  If bSetReference,
	// also set m_szOutwardConnectedEdge
	void JoinOutward( CEdge* pOtherEdge, bool bSetReference = false );

	enum { NUMSYMBOLIC_ELEMENTS = 7 };

	// Create symbolic representation
	// "edge", "{", "name x", "owner x", "length x", "orient x", "concave 1|0", "forced-split"
	// "outward face:edge", "start vertexname", "end vertexname"
	// Returns comma-delimited list for debugging
	CString SaveSymbolic( CArray<CString,const char*>& a ) const;

	// Values returned by ParseElement
	enum {
		PE_ERROR = -1, PE_EMPTY = 0, PE_UNKNOWN,
		PE_ATTRIBUTE,
		PE_OPEN,
		PE_CLOSE,
		// Object types which may belong to this object
		PE_OBJECT_UNKNOWN
	};

	// Parse a single element.  If bAllowExternalJoins, hook up
	// outward connections (should be false on pass 1)
	int ParseElement( LPCTSTR lpElement, bool bAllowExternalJoins = true );

	// Check return code: true if an object
	static bool IsObject( int nPEReturn ) { return false; }

	// Is this edge the base edge of its face?
	bool IsBaseEdge() const;

	// Restore from symbolic representation
	int CreateFromSymbolic1( CArray<CString,const char*> const& a );

	// Fixup symbolic names
	void FixupSymbolic( CMap<CString,const char*,CVertex*,CVertex*> const& mapVertex );

	// Return name
	CString const& GetName() const { return m_szName; }

	// Return fully qualified name (facename:edgename)
	CString GetFQName() const;

	// Get fully qualified name of outward join if connected
	// Return false if not
	bool GetOuterJoinFQName( CString& szName ) const;

	// Detach from outward join.  Return 1 if previously attached,
	// 0 if not.  If bSaveSymbolic, save for later use by FixupSymbolic
	int Detach( bool bSaveSymbolic = false );

	// Get next edge going clockwise
	CEdge* GetClockwiseNext() const;

	// Get next edge going counterclockwise
	CEdge* GetClockwisePrev() const;

	// Get forced-split attribute. If true, we will force this edge to not be joined
	// with another face's edge in the same page group
	bool GetForcedSplit() const { return m_forcedSplit; }

	// Set forced-split to specified value
	void SetForcedSplit( bool b = true ) { m_forcedSplit = b; }

	CFace* m_pFace;
	CString m_szName;
	CString m_szTabText;
	double m_dLength;
	// "North" is 0 degrees, "East" 90, "South" 180, "West" 270
	double m_dOrientation;

	// Pointer to outward connected edge
	CEdge* m_pOutwardConnectedEdge;

	// Face:edge name reference to outward connected edge
	CString m_szOutwardConnectedEdge;
	CString m_szSavedOutwardEdge; // Always saved in Detach()

	// Edge end points always go in clockwise order
	CVertex* m_pEndPoints[2];

	// Name references to end points
	CString m_aszEndPoints[2];

	// Set edge text
	void SetEdgeText( LPCTSTR text ) { m_szEdgeText = text; }

	// Get edge text
	CString GetEdgeText() { return m_szEdgeText; }

protected:
	// Text to be displayed just inside edge
	CString m_szEdgeText;

	// If true, force this edge not to be connected to another
	bool m_forcedSplit;
};

#endif // !defined(AFX_EDGE_H__9837ABD9_3851_11D6_A858_0040F4309CCE__INCLUDED_)

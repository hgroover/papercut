/*!

	@file	 Vertex.h
	
	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: Vertex.h 9 2006-03-08 14:41:10Z henry_groover $
  Vertex.h: interface for the CVertex class.
*/

#if !defined(AFX_VERTEX_H__9837ABDA_3851_11D6_A858_0040F4309CCE__INCLUDED_)
#define AFX_VERTEX_H__9837ABDA_3851_11D6_A858_0040F4309CCE__INCLUDED_

#include "Edge.h"	// Added by ClassView
#include "Face.h"

#include <afxtempl.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CVertex;
class CEdge;
class CFace;
class CLogPoint;

class CVertex  
{
public:
	CVertex( CFace* pOwner );
	CVertex( CFace* pOwner, const char *pszName );
	virtual ~CVertex();

protected:
	// These members should ONLY be called from CFace!!!
	friend class CFace;
	CVertex( CVertex const& src );
	CVertex& operator =(const CVertex &src);
	// Order of vertex within face (origin:0
	int m_nOrder;
public:

	enum { NUMSYMBOLIC_ELEMENTS = 6 };

	// Return name
	CString const& GetName() const { return m_szVertexName; }

	// Create symbolic representation
	// "vertex", "{", "name x", "owner facename", "pivot 0", "inside x",
	// "leg0 edgename", "leg1 edgename"
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

	// Restore from symbolic representation
	int CreateFromSymbolic1( CArray<CString,const char*> const& a );

	// Fixup symbolic names
	void FixupSymbolic( CMap<CString,const char*,CEdge*,CEdge*> const& mapEdge );

	// Get origin:0 order vertex appears within face
	int GetOrder() const { return m_nOrder; }

	CFace* m_pFace;
	CString m_szVertexName;
	bool m_bPivot;
	double m_dInsideAngle;

	// Side[0], vertex, side[1] go clockwise, therefore side[0] is counterclockwise
	// and side[1] is clockwise.
	CEdge* m_pSides[2];

	// Symbolic names for edges
	CString m_aszEdge[2];

	// Logical point used for determining layout
	CLogPoint *m_loc;
	bool m_locSet;
};

#endif // !defined(AFX_VERTEX_H__9837ABDA_3851_11D6_A858_0040F4309CCE__INCLUDED_)

/*!

	@file	 Face.h

	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: Face.h 16 2006-04-15 06:39:12Z henry_groover $

	Face of a polyhedron. Part of a shape object.

*/

#if !defined(AFX_FACE_H__9837ABD8_3851_11D6_A858_0040F4309CCE__INCLUDED_)
#define AFX_FACE_H__9837ABD8_3851_11D6_A858_0040F4309CCE__INCLUDED_

#include "Edge.h"	// Added by ClassView
#include "Vertex.h"
#include "GlobalPreferences.h"
#include "EdgeDef.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxtempl.h>

class CShape;
class CShapeLayout;
class CFace;
class CEdge;
class CVertex;
class CLogPoint;
class CFaceContent;
class CPageGroup;

class CFace : public CPreferenceSet
{
public:
	int GetMinMaxEdge( double& dMin, double& dMax );
	CFace( CShape* pOwner );
	CFace( CShape* pOwner, const char *pszName );
	CFace( CShape* pOwner, CFace const& src );
	virtual ~CFace();

	CFace& operator =( CFace const& src);

	CString m_szFaceName;

	// Upward link to owner
	CShape* m_pOwner;

	// Get edge shared with another face or NULL
	CEdge* GetCommonEdge( CFace* pOtherFace );

	// Find congruent vertex IFF its ordinal is the antecedent of the two vertices
	// on the specified edge.  Returns NULL if not found.  pOtherEdge==NULL
	// guarantees a NULL return value.
	CVertex* GetLeastCongruentVertex( CVertex* pReference, CEdge* pOtherEdge ) const;

	// Get number of outward joins for this face.  If 0, it is isolated.
	int GetJoinCount();

	// Return true if vertex a is the antecedent of vertex b.
	// Normally ordinal(a) < ordinal(b) unless ((ordinal(a)+1)%numvertices) == ordinal(b),
	// which also proves true
	bool IsAntecedent( CVertex* pA, CVertex* pB ) const;

	// Create closed polygonal face. If angles are zero they will be calculated, otherwise angles are for
	// vertices at the end of each edge, going clockwise.
	int CreatePolygon( const char **ppszEdgeNames, double* pdLengths, double* pdAngles, int nSides );

	// Get edge definition set used in CreatePolygon()
	EDGEDEF * GetEdgeDef() const { return m_pEdgeDef; }

	// Get edge definition set used in CreatePolygon() or create one
	EDGEDEF * GetSafeEdgeDef();

	// Re-initialize GetOrder() for vertices
	void SetVertexOrder();

	// Join named edge from this face with edge from another face
	int JoinFaceEdge( const char *pszMyEdgeName, CFace* pOtherFace, const char *pszOtherEdgeName );

	// Create edge of specified length with a symbolic name.
	// name MUST be unique within face
	CEdge* AddEdge( const char *pszEdgeName, double dLength );

	// Join two edges, creating a vertex
	void JoinEdge( const char *pszVertexName, CEdge *pEdge1, CEdge *pEdge2, bool bIsPivot = false );

	// Get the base edge for this face (i.e. the base against which orientation
	// is calculated).
	CEdge* GetBaseEdge() const;

	// Get base edge name (if set)
	CString GetBaseEdgeName() const { return m_szBaseEdgeName; }

	// Set base edge name
	void SetBaseEdgeName( LPCTSTR lp ) { m_szBaseEdgeName = lp; }

	// Get lexically first edge in face
	CEdge* GetFirstEdge() const;

	// Get lexically last edge in face
	CEdge* GetLastEdge() const;

	// Get lexically first vertex in face
	CVertex* GetFirstVertex() const;

	// Get a vertex by name or NULL if not found
	CVertex* GetVertex( LPCTSTR lpVertexName );

	// Get edge by name or NULL if not found
	CEdge* GetEdge( LPCTSTR lpEdgeName ) const;

	// Get page group. May not exist if layout has not been done
	CPageGroup* GetPageGroup() const { return m_PageGroup; }

	// Set page group and return previous setting
	CPageGroup* SetPageGroup(CPageGroup* pGroup, BOOL bRecursive = TRUE );

	// Create symbolic representation of face
	// "face", "{", "name x", "orient x", edges, vertices, "}"
	// Returns comma-delimited list for debugging
	CString SaveSymbolic( CArray<CString,const char*>& a ) const;

	// Values returned by ParseElement
	enum {
		PE_ERROR = -1, PE_EMPTY = 0, PE_UNKNOWN,
		PE_ATTRIBUTE,
		PE_OPEN,
		PE_CLOSE,
		PE_CONTENT,
		// Object types which may belong to this object
		PE_OBJECT_EDGE,
		PE_OBJECT_VERTEX,
		PE_OBJECT_UNKNOWN
	};

	// Parse a single element.  If bAllowExternalJoins, hook up
	// outward connections (should be false on pass 1)
	int ParseElement( LPCTSTR lpElement, bool bAllowExternalJoins = true );

	// Check return code: true if an object
	static bool IsObject( int nPEReturn ) { return (nPEReturn == PE_OBJECT_EDGE || nPEReturn == PE_OBJECT_VERTEX); }

	// Restore from symbolic representation, stage 1
	// name, orientation, numedges, numvertices
	// This is where we allocate all the edge and vertex objects.
	int CreateFromSymbolic1( CArray<CString,const char*>& a );

	// Restore from symbolic representation, stage 2
	// This is where we recurse down and initialize all the
	// edge and vertex objects.
	int FixupSymbolic();

	// Cut connection with other faces.  Returns number
	// of previous connections.  If bSaveSymbolic, save
	// references for restoral by FixupSymbolic()
	int DetachEdges( bool bSaveSymbolic = false );

	// Get all face:edge joins in a pair of arrays
	// Return number of joined pairs
	int GetJoinList( CArray<CString,const char*>& aSource,
		CArray<CString,const char*>& aDest ) const;

	// Get vertex coordinates recursively. Also sets m_loc in each face
	int GetVertexCoordinates( CFace* pOrigin, int nStartVertex, CLogPoint const& StartPoint,
		double dOrientation, CArray<CLogPoint,CLogPoint const&>& a );

	// Render face recursively
	void Render( CDC *pDC, CFace* pOrigin, int nStartVertex, CLogPoint const& StartPoint,
		LPCTSTR szDivision,
		double dOrientation, double dScale = 1.0, double dPrintScale = 1.0 );

	// Recursively find face containing point at specified scale and rotation
	CFace* FindContainingFace( CPoint const& pt, CFace* pOrigin, int nStartVertex, CLogPoint const& StartPoint,
		CEdge*& pEdge, double dOrientation, double dScale = 1.0 );

	// Check for polygon intersections in currently joined set of faces.
	// Return number of collisions found. GetVertexCoordinates() should have been called first to set up m_loc
	int CollisionCount( CArray<CFace*,CFace*>& targetGroup );

	// Check for this face colliding with another. We check for any intersection of the interiors
	// of the two polygons. m_loc must be set in vertices (via GetVertexCoordinates)
	bool CollidesWith( CFace const& dest ) const;

	// Helper function for render: vector move.
	// vector 0-360 in degrees
	static void VectorMove( double& dX, double& dY, double dVectorAngle, double dDistance );

	// Write text at specified coordinates in rotated font
	// with bounding box of specified width and height.
	static void RotatedTextOut( CDC* pDC, double dX, double dY, double dOrientation,
		int nWidth, int nHeight, CString& szText, bool bScaleToFit = true );

	// Calculate inside angles if any are zero.  Always calculate if bForceCalc
	int CalcInsideAngles( bool bForceCalc = false );

	// Change body
	void SetBody( LPCTSTR lpBody ) { m_strBody = lpBody; }

	// Get body after evaluating $ escapes
	CString GetBody( CString& szDefault ) const;

	// Get page number if available
	int GetPageNumber() const;

	// Construct a recursive join list for this face
	CString GetTextJoinList() const;

	// Recursively get joined faces
	void GetJoinedFaces( CMap<CFace const*,CFace const*,int,int>& mapFaceCount ) const;

	// Is face a part of named group?
	bool IsPartOfGroup( LPCTSTR lpGroupName ) const;

	// Add to group
	void AddToGroup( LPCTSTR lpGroupName );

	// Does face have any group memberships?
	bool HasGroups() const { return !m_szGroupNames.IsEmpty(); }

	// Get list of groups
	int GetGroups( CArray<CString,LPCTSTR>& aGroups ) const;

	// Clear group membership
	void ResetGroups() { m_szGroupNames.Empty(); }

	// Expose raw group text
	LPCTSTR GetGroupText() const { return this->m_szGroupNames; }

	// Copy group list from another face
	void CopyGroups( CFace* pSource ) { m_szGroupNames = pSource->GetGroupText(); }

	// Set group text
	void SetGroupText( LPCTSTR lpGroups ) { m_szGroupNames = lpGroups; }

	// Set layout division. "" == default. Returns index of new division
	int SetLayoutDivision( LPCTSTR lpNew );

	// Get layout division.
	CString GetLayoutDivision() { return m_szLayoutDivision; }

	// Do depth-first recursion to clear concave attribute from all outward edges
	void ClearOutwardEdgeConcavity( CFace* pRoot, CFace* pFrom = NULL );

	// Recursively build a map of connected faces
	void BuildConnectedFaceMap( CMap<CFace*,CFace*,int,int>& map, BOOL bForceRecursion = TRUE );

	// Utility functions for symbolic dumping
	static CString m_strNull;
	static CString m_strSpace;
#define MAKESYMBOLIC(name,member) \
	szReturn += member; \
	szReturn += ','; \
	a.Add( CString( name ) + CFace::m_strSpace + member )

#define MAKESYMBOLIC_CVT(name,member) \
	szCvt = CFace::ToStr( member ); \
	MAKESYMBOLIC( name, szCvt )

#define MAKESYMBOLIC_NOTNULL(symname,member,name) \
	pstr = &CFace::m_strNull; \
	if (member) pstr = &(member->name); \
	MAKESYMBOLIC( symname, *pstr )

	// Find fully qualified edge
	static CEdge* FindFQEdge( const char* pszFQEdgeName, CMap<CString,const char*,CFace*,CFace*>& mapFace );

	// Convert radans to degrees
	static long double m_pi;
	static long double m_pi_over_180;
	static long double m_pi_x_2;
	static long double m_pi_over_2;
	static double RadansToDeg( double dr );

	// Convert degrees to radans
	static double DegToRadans( double dDeg );

	// Reduce a value modulo pi
	static double ModuloPi( double angle );

	// Reduce a value modulo 360
	static double Modulo360( double angle );

	// Convert various data types to string
	static CString ToStr( bool b ) { return b ? "1" : "0"; }
	static CString ToStr( int n ) { CString sz; sz.Format( "%d", n ); return sz; }
	static CString ToStr( unsigned long u ) { CString sz; sz.Format( "%lu", u ); return sz; }
	static CString ToStr( float f ) { CString sz; sz.Format( "%f", f ); return sz; }
	static CString ToStr( double d ) { CString sz; sz.Format( "%.16f", d ); return sz; }

	// Create unique name using specified polygon identifier
	static CString UniquePolyName( char cType );

	// Reset unique name for specified polygon
	static void ResetPolyName( char cType );

	// Face orientation in degrees (FIXME need to define this precisely)
	float m_fOrientation;

	// Bitmap and other contents to render
	CFaceContent* m_pContent;

	// This is where actual edge and vertex objects are stored
	CMap<CString,const char*,CEdge*,CEdge*> m_pEdges;
	CMap<CString,const char*,CVertex*,CVertex*> m_pVertices;
protected:
	// FIXME move this to m_pContent
	friend class CPaperCutView;
	CString m_strBody;

	// Groups this face belongs to, beginning with and separated by colon
	CString m_szGroupNames;

	// Unique identifiers for polygons
	static CMap<char,char,CString,CString> m_mapPolyNameExtension;

	// Page group pointer - not valid unless layout done
	CPageGroup *m_PageGroup;

	// Base edge name (optional)
	CString m_szBaseEdgeName;

	// Layout division this face belongs to (default == "")
	CString m_szLayoutDivision;

	// Recursively connect page groups until we come back to ourself
	void ConnectPageGroups( CFace* pEnd, CPageGroup* pGroup, BOOL bClockwise );

	void ClearAllVertices();
	void ClearAllEdges();

	// Last edge def from CreatePolygon() (may also be created by GetSafeEdgeDef())
	EDGEDEF *m_pEdgeDef;

	// Free edge def
	void FreeEdgeDef();

	// Save edge def
	void SaveEdgeDef( const char **ppszEdgeNames, double* pdLengths, int nSides, double* pdAngles = NULL );
	void SaveEdgeDef( EDGEDEF *pEdgeDef ) { if (pEdgeDef!=NULL) SaveEdgeDef( pEdgeDef->aszName, pEdgeDef->adLength, pEdgeDef->nEdges, pEdgeDef->adEndAngles ); }

};

#endif // !defined(AFX_FACE_H__9837ABD8_3851_11D6_A858_0040F4309CCE__INCLUDED_)

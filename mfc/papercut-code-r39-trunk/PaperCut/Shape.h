/*!

	@file	 Shape.h
	
	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: Shape.h 16 2006-04-15 06:39:12Z henry_groover $

  Definition of CShape, the object representing a complete shape

*/

#if !defined(AFX_SHAPE_H__9837ABD7_3851_11D6_A858_0040F4309CCE__INCLUDED_)
#define AFX_SHAPE_H__9837ABD7_3851_11D6_A858_0040F4309CCE__INCLUDED_

#include <afxtempl.h>

#include "Face.h"	// Added by ClassView
#include "Multivertex.h"
#include "GlobalPreferences.h"
#include "EdgeDef.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CVertex;
class CShapeLayout;

class CShape : public CPreferenceSet
{
public:
	CShape( const char *pszBasicType = NULL );
	CShape( CShape const& src );

	virtual ~CShape();

	void RebuildVertexMaps();
	int m_nMaxSides;
	bool IsTriangular();
	bool IsRegular();
	// Get minimum, maximum sides per face and min/max edge lengths.  Returns number of faces.
	int GetStats( int& MinSides, int& MaxSides, double& dMinLength, double& dMaxLength );
	int Geodesic( int Breakdown, bool bStellateCenter );
	CString MakeUniqueFaceName( LPCTSTR lpBase, LPCTSTR lpHint );
	void SetValid( bool bValid = true );

	CShape& operator=( const CShape& src );

	// Copy dictionary from another instance
	void CopyDict( const CShape& src );

	// Values returned by ParseElement
	enum {
		PE_ERROR = -1, PE_EMPTY = 0, PE_UNKNOWN,
		PE_ATTRIBUTE,
		PE_OPEN,
		PE_CLOSE,
		// Object types which may belong to this object
		PE_OBJECT_FACE,
		PE_DICTENTRY,
		PE_LAYOUT, // This is to be ignored
		PE_OBJECT_UNKNOWN = 0x1000
	};

	// Parse a single element.  If bAllowExternalJoins, hook up
	// outward connections (should be false on pass 1)
	int ParseElement( LPCTSTR lpElement, bool bAllowExternalJoins = true );

	// Check return code: true if an object
	static bool IsObject( int nPEReturn ) { return (nPEReturn == PE_OBJECT_FACE); }

	// Set up predefined shape by name ("tetra", "icosa", etc.)
	void SetupShape( const char *pszShapeName );

	// Set up shape as a single polygon: 'T' == triangle, 'S' == square, ..., 'O' == octagon
	void SetupPolygon( char cPolygonType );

	// Deallocate faces
	void ClearAllFaces();

	// Add a single face. After adding all faces call Finalize()
	CFace* AddFace( LPCTSTR faceName, int nEdges, 
		double adEdgeLengths[], double adAngles[], const char *aszEdgeNames[], 
		const char *baseEdgeName, double fOrientation = 0.0 );

	// Finalize shape after adding individual faces. Caller should already have completed edge joins
	void Finalize();

	// Join two faces by name
	int JoinFaceEdge( const char *pszFaceName, const char *pszEdgeName, const char *pszFace2Name, const char *pszEdge2Name );

	// Set up m_mapUnifiedVertices and m_aMultiVertices;
	int MapVertices();

	// Create symbolic representation of shape
	// "shape", "{", face, ..., "}"
	CString SaveSymbolic( CArray<CString,const char*>& a ) const;

	// Restore from symbolic representation, stage 1
	// "shape", "{", face, ..., "}"
	// This is where we allocate all the face, edge and vertex objects.
	int CreateFromSymbolic1( CArray<CString,const char*>& a );

	// Restore from symbolic representation, stage 2
	// This is where we recurse down and fix up all the
	// references in objects created in CreateFromSymbolic1()
	int FixupSymbolic();

	// Find a face by name
	CFace* FindFace( const char *pszName );

	// Return true if shape is valid
	bool IsValid() const;

	// Clear face:edge -> face:edge map
	void ClearFaceEdgeMap() { m_mapFaceEdge.RemoveAll(); }

	// Push a face:vertex join onto map of face:edge => face:edge joins
	// It is not a push but a replacement/insert.  However the map
	// is used as a named entry unordered queue.
	void PushFaceEdgeJoin( const char *pszFaceEdge, const char *pszDestFaceEdge );

	// Variant with face:edge values already parsed
	void PushFaceEdgeJoin( const char *pszFace, const char *pszEdge, const char *pszDestFace, const char *pszDestEdge );

	// Remove face from map of face:edge -> face:edge joins wherever
	// it appears as a target (on the right hand side).  Returns number
	// of entries removed.
	// If bRemoveAsKey == true, left entry will also be removed wherever
	// it appears as a key.
	// If pRightPopMap != NULL, the left side face name of any right-triggered
	// removals will be added.
	int RemoveFaceAsTarget( CFace* pFace, bool bRemoveAsKey = false, CMap<CString,const char*,int,int>* pRightPopMap = NULL );

	// Separate all faces from each other.
	// If bSaveSymbolic, save references for restore by FixupSymbolic()
	void CutAllFaceJoins( bool bSaveSymbolic = false );

	// Pop a face:vertex from the map and if found, join with its
	// destination.  Returns true if found
	bool PopFaceEdgeJoin( const char *pszFaceEdge, CMap<CString,const char*,CString,const char*>& mapPopped, CShapeLayout* pLayout );

	// Parsed variant
	bool PopFaceEdgeJoin( const char *pszFace, const char *pszEdge, CMap<CString,const char*,CString,const char*>& mapPopped, CShapeLayout* pLayout );

	// Get combined join list
	// Return number of entries
	int BuildJoinList() { return BuildJoinList( m_mapFaceEdge ); }

	// Get combined join list into any map
	// Return number of entries
	int BuildJoinList( CMap<CString,const char*,CString,const char*>& map ) const;

	// Get shape name
	LPCTSTR GetName() const { return m_szName; }

	// Set shape name, e.g. "My Tesselated dodecarhombicosahedron"
	void SetName( LPCTSTR lpName ) { m_szName = lpName; }

	// Strip elements from one array to another, up to and including
	// ending brace ("}").  Returns total removed from source or -1 if error.
	// Current position must be {
	static int MoveObject( CArray<CString,LPCTSTR>& aDest, CArray<CString,LPCTSTR>& aSrc,
		int nStartSource, bool bEmptyFromSource = true );

	// Find fully qualified edge
	CEdge* FindFQEdge( const char* pszFQEdgeName ) { return CFace::FindFQEdge( pszFQEdgeName, m_mapFaces ); }

	// Get number of faces
	int GetNumFaces() const { return m_mapFaces.GetCount(); }

	// Get first face
	CFace* GetFirstFace() const;

	// Rename a face
	void RenameFace( CFace* pFace, LPCTSTR NewName );

	// Stellate (pop out) faces by making each face into a pyramid.
	// The edges of the pyramid are equal to the base edges * ratio.
	// Returns number of faces ADDED.
	int Stellate( double dRatio, BOOL bKepler = FALSE );

	// Stellate a single face.  Return number of new faces added.
	int StellateFace( CFace* pFace, double dRatio );

	// Stellate shape a la Kepler's Stella Octangula. Return number of new faces added.
	int KStellate( double dRatio );

	// Outdent a single quadrangular face. Negative height ratio indents. Return number of new faces added.
	int OutdentFace( CFace* pFace, double dHeightRatio, double dSideRatio );

	// true if minimum stellation ratio has been exceeded
	bool GetMinimumStellationRatio( double& dMinimum );

	// Reset minimum stellation ratio
	void ResetMinimumRatio() { this->m_bMinStellationRatioExceeded = false; this->m_dMinStellationRatio = 0.0; }

	// Remove a face, unlink from all connections, and delete associated object
	void DeleteFace( LPCTSTR lpFaceName, bool bRedoMaps = true );

	// Delete an entire group of faces
	void DeleteFaceGroup( LPCTSTR lpFaceGroupName );

	// Get list of faces sorted in lexical order.  Return number in list
	int GetSortedFaces( CArray<CFace*,CFace*>& aFaces ) const;

	// Set defaults for layout
	void SetLayoutDefaults( double dPageWidth, double dPageWidthOverHeight );

	// Accessors for layout defaults
	double GetLayoutDefaultWidth() const { return m_dLayoutDefaultPageWidth; }
	double GetLayoutDefaultWidthOverHeight() const { return m_dLayoutDefaultPageWidthOverHeight; }
	double GetDefaultSizeRatio() const { return m_dDefaultSizeRatio; }

	// Load transform for pathnames (if file is not specified by absolute path, use m_szBaseDir
	// to make it an absolute path)
	CString LoadTransformPath( LPCTSTR lpInputRelativePath );

	// Save transform for pathnames (convert to relative path if matching m_szBaseDir)
	CString SaveTransformPath( LPCTSTR lpInputAbsolutePath );

	// Set base directory.  Input may be a file pathname or a directory
	// path ending in backslash.
	void SetTransformBaseDir( LPCTSTR lpPath );

	// Substitute $varname$ escapes in text. Returns total number of substitutions
	int Substitute( CString& sz, unsigned int data1 = 0, unsigned int data2 = 0 );

	// Scan for matching paren (handling nested parens). Return offset from lp[0] or -1 if not found. lp[0]=='('
	int ScanMatchingParen( LPCTSTR lp );

	// Extract immediate text, escaping '$$' --> '$'. Return index to first non-text
	int ScanEscapedText( CString& szText, LPCTSTR lp );

	// Get division count
	int GetDivisionCount() const { return m_aDivisions.GetSize(); }

	// Get division
	CString GetDivisionName( int Index ) const { if (Index >= m_aDivisions.GetSize()) return ""; return m_aDivisions[Index]; }

	// Get active division. Default is ""
	CString GetActiveDivision() const { return m_aDivisions[m_nActiveDivision]; }

	// Get active division index.
	int GetActiveDivisionIndex() const { return m_nActiveDivision; }

	// Set active division. New index returned if valid, -1 if division doesn't exist and bAddNew==false
	int SetActiveDivision( LPCTSTR szNew, bool bAddNew = false );

	// Get division index
	int GetDivisionIndex( LPCTSTR sz, bool bAddNew = false );

	// Set active division by index. New index returned if valid, -1 if out of range
	int SetActiveDivision( int New );

	// Find division by name. Return index or -1 if not found
	int FindDivision( LPCTSTR sz );

	// Join from Face:edge-Face:edge list
	void JoinFromList( LPCTSTR aszJoinList[] );
	void JoinFromList( CArray<CString,LPCTSTR>& aJoinList );

	// Set layout defaults
	void SetDefaultWidth( double dPageWidth = 2.9, double dWidthOverHeight = 0.762 ) { 
		this->m_dLayoutDefaultPageWidth = dPageWidth; 
		this->m_dLayoutDefaultPageWidthOverHeight = dWidthOverHeight;
	}

	// Process single entry from JoinFromList() - false if error
	bool JoinFromSingleEntry(CString& szErrors, LPCTSTR Entry);

	// Write as definition. true if successful.
	bool WriteAsDefinition(CString const& szPathName) const;

	// Create a definition name appropriate to a polygon with specified number of faces.
	static CString GetPolygonDefName( int nSides );

	// Compare edge defs
	static bool AreEdgeDefsEqual( EDGEDEF const *p1, EDGEDEF const *p2 );

	// Last saved path. May be empty if new shape and not yet saved.
	CString m_SavedPath;

	// Faces in this shape
	CMap<CString,const char*,CFace*,CFace*> m_mapFaces;

	// Map of face:edge to face:edge joins
	// Used by ClearFaceEdgeMap(), PushFaceEdgeJoin(), RemoveFaceAsTarget(),
	// and PopFaceEdgeJoin()
	CMap<CString,const char*,CString,const char*> m_mapFaceEdge;

	// Map of face:vertex names to multivertex objects
	CMap<CString,const char*,CMultiVertex*,CMultiVertex*> m_mapUnifiedVertices;

	// Actual multivertex objects
	CArray<CMultiVertex*,CMultiVertex*> m_aMultiVertices;

	// String dictionary for $varname$ substitution
	CMap<CString,const char*,CString,const char*> m_mapVariableDictionary;

	// Message formats
	static LPCTSTR szMIN_STELLATION_RATIO_WARNING_fsf;

protected:
	double m_dLayoutDefaultPageWidth;
	double m_dLayoutDefaultPageWidthOverHeight;
	double m_dDefaultSizeRatio;

	double m_dMinStellationRatio;
	bool m_bMinStellationRatioExceeded;

	int m_nMinSides;
	bool m_bIsTriangular;
	int m_IsRegular;
	bool m_bValid;
	CString m_szName;

	// Valid divisions. Default is to have 1 division named "" (default)
	CArray<CString,LPCTSTR> m_aDivisions;

	// Index of active division
	int m_nActiveDivision;

	// Basename used by LoadTransformPath() and SaveTransformPath()
	CString m_szBaseDir;

	// Recursive parsing used by Substitute - parse text (see Substitute implementation)
	int SubText( CString& sz, unsigned int data1, unsigned int data2 );

	// Recursive parsing used by Substitute - parse expression (see Substitute implementation)
	int SubExpr( CString& sz, CString szParens, unsigned int data1, unsigned int data2 );

	// Called by SetupShape
	void CreateShapeFromParms( LPCTSTR szName, int nEdges, double adEdgeLengths[], double adAngles[], const char *aszEdgeNames[], 
			int nFaces, const char *aszFaceNames[], const char *aszJoinList[] );

	// Also called by SetupShape - aFaces[] ends with szType==NULL, aszJoins[] ends with NULL
	void CreateShapePrimitive( LPCTSTR szName, EDGEDEFDICT& dictEdgeDefs, FACEDEF *aFaces, LPCTSTR aszJoins[] );

	// Free multi-vertex vector
	void FreeMultiVertices();

};

#endif // !defined(AFX_SHAPE_H__9837ABD7_3851_11D6_A858_0040F4309CCE__INCLUDED_)

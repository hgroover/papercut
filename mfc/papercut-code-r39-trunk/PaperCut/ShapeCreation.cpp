/*!

	@file	 ShapeCreation.cpp
	
	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: ShapeCreation.cpp 16 2006-04-15 06:39:12Z henry_groover $

  Implementation of primitive creation functions for CShape

*/

#include "stdafx.h"
#include "PaperCut.h"
#include "Shape.h"

#include <math.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// Golden rhombus - a golden rhombus has the following angles (see http://mathworld.wolfram.com/GoldenRhombus.html for details)
// leg a to leg b - 116.56505117707798935157219372045 degrees
// leg b to leg c -  63.43494882292201064842780627955 degrees
// When adding any rhombus as a face, we either add it as "a" (meaning that the leg preceding the greater
// of the two angles will be joined at the attachment point) or "b" (meaning that the leg following the greater
// of the two angles will be joined at the attachment point).

/**
Notes on Escher's solid

Escher's solid is based on a rhombic dodecahedron. See escher-pyramid.gif and http://mathworld.wolfram.com/RhombicDodecahedron.html
Consider the base, which is a face of a rhombic dodecahedron. r=2*sqrt(2), q=2, and s=sqrt(3)
Expressed in units of s, q=2/sqrt(3) and r=2*sqrt(2)/sqrt(3)
The pyramid has two longer vertical edges named v and two shorter vertical edges named t. The height is h.
v = r / sqrt(2)
h = sqrt( v^2 - r^2 / 4 )
t = sqrt( q^2 / 4 + h^2 )
Solving for r in units of s gives us
r = 2 * sqrt(2) / sqrt(3) = 2 * 1.4142135623730950488016887242097 / 1.7320508075688772935274463415059 = 1.6329931618554520654648560498039
v = r / sqrt(2) = 1.6329931618554520654648560498039 / 1.4142135623730950488016887242097 = 1.1547005383792515290182975610045
h = 2/3
**/

// Join a single entry
bool CShape::JoinFromSingleEntry(CString& szErrors, LPCTSTR Entry)
{
	CString szTemp;
	char szFace1[64], szEdge1[64], szFace2[64], szEdge2[64];
	if (sscanf( Entry, "%[^:]:%[^-= ]%*[-= ]%[^:]:%s", szFace1, szEdge1, szFace2, szEdge2 ) == 4)
	{
		JoinFaceEdge( szFace1, szEdge1, szFace2, szEdge2 );
	}
	else
	{
		szTemp.Format( "Join: unable to parse %s\r\n", Entry );
		if (szErrors.IsEmpty()) szErrors = "The following internal errors occurred setting up this shape:\r\n";
		szErrors += szTemp;
		return false;
	}
	return true;
}

// Join from Face:edge-Face:edge list
void CShape::JoinFromList( LPCTSTR aszJoinList[] )
{
	CString szErrors;
	// Join faces by edge.  Note that for any two
	// faces, there is only one orientation for an
	// edge join without flipping.  This is constrained
	// by both faces having had their sides defined
	// in the same edge order.  We always go clockwise.
	for (int n = 0; aszJoinList[n] != NULL; n++)
	{
		JoinFromSingleEntry( szErrors, aszJoinList[n] );
	}
	if (!szErrors.IsEmpty())
	{
		AfxMessageBox( szErrors );
	}
}

void CShape::JoinFromList( CArray<CString,LPCTSTR>& aJoinList )
{
	CString szErrors;
	// Join faces by edge.
	for (int n = 0; n < aJoinList.GetSize(); n++)
	{
		JoinFromSingleEntry( szErrors, aJoinList[n] );
	}
	if (!szErrors.IsEmpty())
	{
		AfxMessageBox( szErrors );
	}
}

// Also called by SetupShape - aFaces[] ends with szType==NULL, aszJoins[] ends with NULL
void CShape::CreateShapePrimitive( LPCTSTR szName, EDGEDEFDICT& dictEdgeDefs, FACEDEF *aFaces, LPCTSTR aszJoins[] )
{
	ClearAllFaces();
	m_szName = szName;

	// Create faces
	int nFace;
	for (nFace = 0; aFaces[nFace].szType != NULL; nFace++)
	{
		CFace* pf = new CFace( this, aFaces[nFace].szName );
		EDGEDEF * pedef = dictEdgeDefs[aFaces[nFace].szType];
		pf->CreatePolygon( pedef->aszName, pedef->adLength, pedef->adEndAngles, pedef->nEdges );
		pf->SetBaseEdgeName( aFaces[nFace].szBaseEdge );
		pf->m_fOrientation = aFaces[nFace].fOrientation;
		m_mapFaces.SetAt( aFaces[nFace].szName, pf );
	} // for all faces

	// Join edges
	JoinFromList( aszJoins );

	// Create map of unified vertices
	MapVertices();

	// Set default page width
	m_dLayoutDefaultPageWidth = 2.9;
	m_dLayoutDefaultPageWidthOverHeight = 0.762;

}

// Add a single face. After adding all faces call Finalize()
// Edges and angles must be added in clockwise order from the perspective of outside the shape.
// All faces must be treated consistently.
CFace* CShape::AddFace( LPCTSTR faceName, 
					 int nEdges, double adEdgeLengths[], double adAngles[], const char *aszEdgeNames[], 
					 const char *baseEdgeName, double fOrientation /*= 0.0*/ )
{
	CFace* pf = new CFace( this, faceName );
	pf->CreatePolygon( aszEdgeNames, adEdgeLengths, adAngles, nEdges );
	pf->SetBaseEdgeName( baseEdgeName );
	pf->m_fOrientation = fOrientation;
	m_mapFaces.SetAt( faceName, pf );
	return pf;
}

// Finalize shape after adding individual faces. Caller should already have completed edge joins
void CShape::Finalize()
{
	// Create map of unified vertices
	MapVertices();

	// Set default page width
	m_dLayoutDefaultPageWidth = 2.9;
	m_dLayoutDefaultPageWidthOverHeight = 0.762;
}

// Called by SetupShape
void CShape::CreateShapeFromParms( LPCTSTR szName, int nEdges, double adEdgeLengths[], double adAngles[], const char *aszEdgeNames[], 
									int nFaces, const char *aszFaceNames[], const char *aszJoinList[] )
{
	ClearAllFaces();

	m_szName = szName;

	int n;
	for (n = 0; n < nFaces; n++) {
		CFace* pf = new CFace( this, aszFaceNames[ n ] );
		// This creates a vertex after each side with the same name
		pf->CreatePolygon( aszEdgeNames, adEdgeLengths, adAngles, nEdges );
		m_mapFaces.SetAt( aszFaceNames[n], pf );
	}

	JoinFromList( aszJoinList );

	// Create map of unified vertices
	MapVertices();

}

// Set up shape as a single polygon: 'T' == triangle, 'S' == square, ..., 'D' == decagon
void CShape::SetupPolygon( char cPolygonType )
{
	int nSides;
	switch (cPolygonType)
	{
	case 'T':
		nSides = 3;
		break;
	case 'S':
		nSides = 4;
		break;
	case 'P':
		nSides = 5;
		break;
	case 'H':
		nSides = 6;
		break;
	case 'O':
		nSides = 8;
		break;
	case 'D':
		nSides = 10;
		break;
	default:
		::AfxMessageBox( "Unknown polygon type" );
		return;
	}

	// Complements of angles add up to 360
	double dAngle = 180.0 - (360.0 / nSides);
	// Reset name to "A" so we'll start with "TA", "SA", "HA", etc.
	CFace::ResetPolyName( cPolygonType );
	CFace* pFace = new CFace( this, CFace::UniquePolyName( cPolygonType ) );
	// Create polygon for face
	const char *aszEdgeNames[] = {	"a", "b", "c", "d",
									"e", "f", "g", "h",	
									"i", "j", "k", "l" };
	double adEdgeLengths[] = {		1.0, 1.0, 1.0, 1.0,
									1.0, 1.0, 1.0, 1.0,
									1.0, 1.0, 1.0, 1.0 };
	double adAngles[] = { dAngle, dAngle, dAngle, dAngle,
						  dAngle, dAngle, dAngle, dAngle,
						  dAngle, dAngle, dAngle, dAngle };
	pFace->CreatePolygon( aszEdgeNames, adEdgeLengths, adAngles, nSides );
	// Set base edge to a
	pFace->SetBaseEdgeName( aszEdgeNames[0] );
	// Add to face map
	this->m_mapFaces.SetAt( pFace->m_szFaceName, pFace );
	// Create map of unified vertices
	MapVertices();
}

void CShape::SetupShape(const char *pszShapeName)
{
	// Basic types supported are "tetra", "icosa", "geo2x"
	// Shape is an unfolded net representation of a 3D polyhedron
	// (net as in fisherman's net)
	// The shape consists of multiple faces.
	// Faces are joined on edges.  Edges may be unjoined.
	// Laying out a shape involves splitting edges
	// Splits may be minimal or container optimal.
	// A sheet of paper is a container, and for a given size
	// there will be an optimal set of faces which best fit the container.

	m_IsRegular = -1; // Indeterminate

	// Pick off shapes defined in papercut.shapedefs
	if (MYAPP()->InitShape( *this, pszShapeName ))
	{
		return;
	}

	if (!stricmp( pszShapeName, "tetra" ) || !stricmp( pszShapeName, "tetrahedron" ))
	{
		EDGEDEFDICT dictDefs;
		FACEDEF aFaces[] = {
			{ "T", "A", "c", 90.0 },
			{ "T", "B", "c", 90.0 },
			{ "T", "C", "c", 90.0 },
			{ "T", "D", "a", 90.0 },
			{ NULL, NULL }
		};
		const char *aszEdgeNames[] = { "a", "b", "c" };
		double adEdgeLengths1[] = { 1.0, 1.0, 1.0 };
		double adAngles[] = { 60.0, 60.0, 60.0 };
		EDGEDEF aTEdges = { 3, aszEdgeNames, adEdgeLengths1, adAngles };
		dictDefs.SetAt( "T", &aTEdges );
		LPCTSTR aszJoins[] = {
			"A:a-C:b",
			"A:b-B:a",
			"A:c-D:a",

			"B:b-C:a",
			"B:c-D:b",

			"C:c-D:c",

			NULL
		};
		CreateShapePrimitive( pszShapeName, dictDefs, aFaces, aszJoins );
		// 4 faces, 6 edges, minimum 3 splits

		// Set default page width
		m_dLayoutDefaultPageWidth = 1.9;
		m_dLayoutDefaultPageWidthOverHeight = 0.762;

	} // Tetrahedron

	if (!stricmp( pszShapeName, "cube" ))
	{
		EDGEDEFDICT dictDefs;
		FACEDEF aFaces[] = {
			{ "S", "A", "c", 90.0 },
			{ "S", "B", "c", 90.0 },
			{ "S", "C", "c", 90.0 },
			{ "S", "D", "c", 90.0 },
			{ "S", "E", "a", 90.0 },
			{ "S", "F", "a", 90.0 },
			{ NULL, NULL }
		};
		const char *aszEdgeNames[] = { "a", "b", "c", "d" };
		double adEdgeLengths1[] = { 1.0, 1.0, 1.0, 1.0 };
		double adAngles[] = { 90.0, 90.0, 90.0, 90.0 };
		EDGEDEF aSEdges = { 4, aszEdgeNames, adEdgeLengths1, adAngles };
		LPCTSTR aszJoins[] = {
			// Go strictly in lexical order
			"A:a-F:c",
			"A:b-D:a",
			"A:c-C:a",
			"A:d-B:a",

			"B:b-C:d",
			"B:c-E:d",
			"B:d-F:d",

			"C:b-D:d",
			"C:c-E:a",

			"D:b-F:b",
			"D:c-E:b",

			"E:c-F:a",

			NULL
		};
		dictDefs.SetAt( "S", &aSEdges );
		CreateShapePrimitive( pszShapeName, dictDefs, aFaces, aszJoins );

		// Default to scaling by 96% of width
		this->SetScaleMethod( CPreferenceSet::SM_BYBASE );
		this->SetBaseRatio( 0.96 );

		// Set default page width
		m_dLayoutDefaultPageWidth = 2.2;
		m_dLayoutDefaultPageWidthOverHeight = 0.762;

	} // Cube

	if (!stricmp( pszShapeName, "dodeca" ) || !stricmp( pszShapeName, "dodecahedron" ))
	{
		// 12 faces
		EDGEDEFDICT dictDefs;
		FACEDEF aFaces[] = {
			{ "P", "A", "e", 90.0 },
			{ "P", "B", "d", 90.0 },
			{ "P", "C", "d", 90.0 },
			{ "P", "D", "e", 90.0 },
			{ "P", "E", "d", 90.0 },
			{ "P", "F", "d", 90.0 },
			{ "P", "G", "a", 90.0 },
			{ "P", "H", "a", 90.0 },
			{ "P", "I", "a", 90.0 },
			{ "P", "J", "a", 90.0 },
			{ "P", "K", "a", 90.0 },
			{ "P", "L", "c", 90.0 },
			{ NULL, NULL }
		};
		static const char *apszNames[] = { "a", "b", "c", "d", "e" };
		static double adLengths[] = { 1.0, 1.0, 1.0, 1.0, 1.0 };
		static double adAngles[] = { 0.0, 0.0, 0.0, 0.0, 0.0 };
		static EDGEDEF aPEdges = { 5, apszNames, adLengths, adAngles };
		dictDefs.SetAt( "P", &aPEdges );
		static const char *aszJoins[] = {
			"A:a-F:c",
			"A:b-B:d",
			"A:c-C:e",
			"A:d-D:a",
			"A:e-E:b",

			"B:a-I:e",
			"B:b-G:e",
			"B:c-C:a",
			"B:e-F:b",

			"C:b-G:d",
			"C:c-L:d",
			"C:d-D:b",

			"D:c-L:c",
			"D:d-K:b",
			"D:e-E:c",

			"E:d-K:a",
			"E:e-J:a",
			"E:a-F:d",

			"F:e-J:e",
			"F:a-I:a",

			"G:a-I:d",
			"G:b-H:d",
			"G:c-L:e",

			"H:e-I:c",
			"H:a-J:c",
			"H:b-K:d",
			"H:c-L:a",

			"I:b-J:d",

			"J:b-K:e",

			"K:c-L:b",

			//L already connected
			NULL
		};

		CreateShapePrimitive( pszShapeName, dictDefs, aFaces, aszJoins );

		// Default to scaling by 96% of width - which works for unrotated pentagonal faces
		this->SetScaleMethod( CPreferenceSet::SM_BYBASE );
		this->SetBaseRatio( 0.96 );
		// Set default page width
		m_dLayoutDefaultPageWidth = 2.6;
		m_dLayoutDefaultPageWidthOverHeight = 0.762;
	} // Dodecahedron

	if (!stricmp( pszShapeName, "cuboctahedron" ))
	{
		EDGEDEFDICT dictDefs;
		FACEDEF aFaces[] = {
			{ "S", "SA", "d", 90.0 },
			{ "S", "SB", "d", 90.0 },
			{ "S", "SC", "d", 90.0 },
			{ "S", "SD", "d", 90.0 },
			{ "S", "SE", "d", 90.0 },
			{ "S", "SF", "d", 90.0 },
			{ "T", "TA", "c", 90.0 },
			{ "T", "TB", "c", 90.0 },
			{ "T", "TC", "c", 90.0 },
			{ "T", "TD", "c", 90.0 },
			{ "T", "TE", "c", 90.0 },
			{ "T", "TF", "c", 90.0 },
			{ "T", "TG", "c", 90.0 },
			{ "T", "TH", "c", 90.0 },
			{ NULL, NULL }
		};
		const char *aszEdgeNames[] = { "a", "b", "c", "d" };
		double adEdgeLengths1[] = { 1.0, 1.0, 1.0, 1.0 };
		static double adAngles[] = { 0.0, 0.0, 0.0, 0.0, 0.0 };
		EDGEDEF aTEdges = { 3, aszEdgeNames, adEdgeLengths1, adAngles };
		EDGEDEF aSEdges = { 4, aszEdgeNames, adEdgeLengths1, adAngles };
		LPCTSTR aszJoins[] = {
			// Go strictly in lexical order, which means SA < SB < ... SF < TA < TB ... < TH
			"SA:a-TG:a",
			"SA:b-TB:c",
			"SA:c-TC:a",
			"SA:d-TA:b",
			"SB:a-TA:c",
			"SB:b-TC:c",
			"SB:c-TD:c",
			"SB:d-TH:a",
			"SC:a-TC:b",
			"SC:b-TB:b",
			"SC:c-TE:c",
			"SC:d-TD:a",
			"SD:a-TE:b",
			"SD:b-TF:c",
			"SD:c-TH:b",
			"SD:d-TD:b",
			"SE:a-TB:a",
			"SE:b-TG:c",
			"SE:c-TF:a",
			"SE:d-TE:a",
			"SF:a-TA:a",
			"SF:b-TH:c",
			"SF:c-TF:b",
			"SF:d-TG:b",
			NULL
		};
		dictDefs.SetAt( "S", &aSEdges );
		dictDefs.SetAt( "T", &aTEdges );
		CreateShapePrimitive( pszShapeName, dictDefs, aFaces, aszJoins );
		// Default to scaling by 96% of width - which works for square faces
		this->SetScaleMethod( CPreferenceSet::SM_BYBASE );
		this->SetBaseRatio( 0.96 );
	} // Cuboctahedron

	if (!stricmp( pszShapeName, "truncated octahedron" ))
	{
		EDGEDEFDICT dictDefs;
		FACEDEF aFaces[] = {
			{ "H", "HA", "f", 90.0 },
			{ "H", "HB", "f", 90.0 },
			{ "H", "HC", "f", 90.0 },
			{ "H", "HD", "f", 90.0 },
			{ "H", "HE", "f", 90.0 },
			{ "H", "HF", "f", 90.0 },
			{ "H", "HG", "f", 90.0 },
			{ "H", "HH", "f", 90.0 },
			{ "S", "SA", "d", 90.0 },
			{ "S", "SB", "d", 90.0 },
			{ "S", "SC", "d", 90.0 },
			{ "S", "SD", "d", 90.0 },
			{ "S", "SE", "d", 90.0 },
			{ "S", "SF", "d", 90.0 },
			{ NULL, NULL }
		};
		const char *aszEdgeNames[] = { "a", "b", "c", "d", "e", "f" };
		double adEdgeLengths1[] = { 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 };
		static double adAngles[] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
		EDGEDEF aHEdges = { 6, aszEdgeNames, adEdgeLengths1, adAngles };
		EDGEDEF aSEdges = { 4, aszEdgeNames, adEdgeLengths1, adAngles };
		LPCTSTR aszJoins[] = {
			// Go strictly in lexical order, which means HA < HB < ... HH < SA < SB ... < SF
			"HA:a-SA:c",
			"HA:b-HB:e",
			"HA:c-SB:d",
			"HA:d-HD:e",
			"HA:e-SF:b",

			"HA:f-HH:c",
			"HB:a-HF:f",
			"HB:b-SC:d",
			"HB:c-HC:f",
			"HB:d-SB:a",

			"HB:f-SA:b",
			"HC:a-SC:c",
			"HC:b-HE:e",
			"HC:c-SD:d",
			"HC:d-HD:a",

			"HC:e-SB:b",
			"HD:b-SD:c",
			"HD:c-HG:d",
			"HD:d-SF:c",
			"HD:f-SB:c",

			"HE:a-HF:d",
			"HE:b-SE:d",
			"HE:c-HG:f",
			"HE:d-SD:a",
			"HE:f-SC:b",

			"HF:a-SA:a",
			"HF:b-HH:a",
			"HF:c-SE:a",
			"HF:e-SC:a",
			"HG:a-SE:c",

			"HG:b-HH:e",
			"HG:c-SF:d",
			"HG:e-SD:b",
			"HH:b-SA:d",
			"HH:d-SF:a",
			"HH:f-SE:b",
			NULL
		};
		dictDefs.SetAt( "H", &aHEdges );
		dictDefs.SetAt( "S", &aSEdges );
		CreateShapePrimitive( pszShapeName, dictDefs, aFaces, aszJoins );
		// Default to scaling by 96% of width - which works for square faces
		this->SetScaleMethod( CPreferenceSet::SM_BYBASE );
		this->SetBaseRatio( 0.96 );
	} // Truncated Octahedron

	if (!stricmp( pszShapeName, "snub cube" ))
	{
		EDGEDEFDICT dictDefs;
		FACEDEF aFaces[] = {
			{ "S", "SA", "c", 90.0 },
			{ "S", "SB", "b", 75.0 },
			{ "S", "SC", "c", 75.0 },
			{ "S", "SD", "c", 90.0 },
			{ "S", "SE", "a", 75.0 },
			{ "S", "SF", "d", 75.0 },
			{ "T", "TA", "c", 100.0 },
			{ "T", "TB", "b", -90.0 },
			{ "T", "TC", "b", -90.0 },
			{ "T", "TD", "a", -90.0 },
			{ "T", "TE", "a", 100.0 },
			{ "T", "TF", "a", 75.0 },
			{ "T", "TG", "c", -90.0 },
			{ "T", "TH", "a", 100.0 },
			{ "T", "TI", "a", 96.0 },
			{ "T", "TJ", "b", 65.0 },
			{ "T", "TK", "b", 60.0 },
			{ "T", "TL", "a", 90.0 },
			{ "T", "TM", "a", 120.0 },
			{ "T", "TN", "a", 75.0 },
			{ "T", "TO", "b", 55.0 },
			{ "T", "TP", "b", 75.0 },
			{ "T", "TQ", "b", 120.0 },
			{ "T", "TR", "b", 95.0 },
			{ "T", "TS", "b", 90.0 },
			{ "T", "TT", "b", 90.0 },
			{ "T", "TU", "c", 95.0 },
			{ "T", "TV", "c", 120.0 },
			{ "T", "TW", "b", 60.0 },
			{ "T", "TX", "b", 75.0 },
			{ "T", "TY", "c", 60.0 },
			{ "T", "TZ", "b", 90.0 },
			{ "T", "T1", "c", 90.0 },
			{ "T", "T2", "c", 60.0 },
			{ "T", "T3", "c", 70.0 },
			{ "T", "T4", "c", 100.0 },
			{ "T", "T5", "c", 80.0 },
			{ "T", "T6", "b", 100.0 },
			{ NULL, NULL }
		};
		const char *aszEdgeNames[] = { "a", "b", "c", "d" };
		double adEdgeLengths1[] = { 1.0, 1.0, 1.0, 1.0 };
		static double adAngles[] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
		EDGEDEF aTEdges = { 3, aszEdgeNames, adEdgeLengths1, adAngles };
		EDGEDEF aSEdges = { 4, aszEdgeNames, adEdgeLengths1, adAngles };
		LPCTSTR aszJoins[] = {
			// Go strictly in lexical order, which means SA < SB < ... SF < T1 ... < T6 < TA < TB ... < TZ
			"SA:a-TC:b",
			"SA:b-TG:c",
			"SA:c-TD:a",
			"SA:d-TB:a",
			"SB:a-TI:b",
			"SB:b-TM:c",
			"SB:c-TJ:a",
			"SB:d-TF:a",
			"SC:a-TP:b",
			"SC:b-TR:c",
			"SC:c-TQ:a",
			"SC:d-TO:a",
			"SD:a-TS:b",
			"SD:b-T1:c",
			"SD:c-TT:a",
			"SD:d-TL:a",
			"SE:a-TV:b",
			"SE:b-TW:c",
			"SE:c-TX:a",
			"SE:d-TU:a",
			"SF:a-T3:b",
			"SF:b-T5:c",
			"SF:c-T4:a",
			"SF:d-TZ:a",
			"T1:a-TZ:b",
			"T1:b-T2:c",
			"T2:a-T4:c",
			"T3:a-TA:c",
			"T3:c-TR:a",
			"T4:b-TW:a",
			"T5:a-TB:b",
			"T5:b-T6:c",
			"T6:a-TD:c",
			"T6:b-TW:b",
			"TA:a-TP:a",
			"TA:b-TB:c",
			"TC:a-TE:c",
			"TC:c-TP:c",
			"TD:b-TX:a",
			"TE:a-TO:c",
			"TE:b-TF:c",
			"TF:b-TG:a",
			"TG:b-TH:c",
			"TH:a-TJ:c",
			"TI:a-TK:c",
			"TI:c-TO:b",
			"TJ:b-TU:b",
			"TK:a-TQ:c",
			"TK:b-TL:c",
			"TL:b-TM:a",
			"TM:b-TN:c",
			"TN:a-TT:c",
			"TN:b-TU:c",
			//TO
			//TP
			"TQ:b-TS:c",
			"TR:b-TY:a",
			"TS:a-TY:c",
			"TT:b-TV:c",
			//TU
			//TV
			//TW
			//TX
			"TY:b-TZ:c",
			//TZ
			NULL
		};
		dictDefs.SetAt( "S", &aSEdges );
		dictDefs.SetAt( "T", &aTEdges );
		CreateShapePrimitive( pszShapeName, dictDefs, aFaces, aszJoins );
		// Default to scaling by 96% of width - which works for square faces
		this->SetScaleMethod( CPreferenceSet::SM_BYBASE );
		this->SetBaseRatio( 0.96 );
	}

	if (!stricmp( pszShapeName, "buckyball" ))
	{
		EDGEDEFDICT dictDefs;
		FACEDEF aFaces[] = {
			{ "H", "HA", "d", 90.0 },
			{ "H", "HB", "d", 90.0 },
			{ "H", "HC", "d", 90.0 },
			{ "H", "HD", "d", 90.0 },
			{ "H", "HE", "d", 90.0 },
			{ "H", "HF", "d", 90.0 },
			{ "H", "HG", "d", 90.0 },
			{ "H", "HH", "d", 90.0 },
			{ "H", "HI", "d", 90.0 },
			{ "H", "HJ", "d", 90.0 },
			{ "H", "HK", "d", 90.0 },
			{ "H", "HL", "d", 90.0 },
			{ "H", "HM", "d", 90.0 },
			{ "H", "HN", "d", 90.0 },
			{ "H", "HO", "d", 90.0 },
			{ "H", "HP", "d", 90.0 },
			{ "H", "HQ", "d", 90.0 },
			{ "H", "HR", "d", 90.0 },
			{ "H", "HS", "d", 90.0 },
			{ "H", "HT", "d", 90.0 },
			{ "P", "PA", "d", 75.0 },
			{ "P", "PB", "c", 90.0 },
			{ "P", "PC", "d", 75.0 },
			{ "P", "PD", "c", 90.0 },
			{ "P", "PE", "d", 75.0 },
			{ "P", "PF", "c", 90.0 },
			{ "P", "PG", "d", 75.0 },
			{ "P", "PH", "c", 90.0 },
			{ "P", "PI", "d", 75.0 },
			{ "P", "PJ", "c", 90.0 },
			{ "P", "PK", "d", 75.0 },
			{ "P", "PL", "c", 90.0 },
			{ NULL, NULL }
		};
		const char *aszEdgeNames[] = { "a", "b", "c", "d", "e", "f" };
		double adEdgeLengths1[] = { 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 };
		static double adAngles[] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
		EDGEDEF aHEdges = { 6, aszEdgeNames, adEdgeLengths1, adAngles };
		EDGEDEF aPEdges = { 5, aszEdgeNames, adEdgeLengths1, adAngles };
		LPCTSTR aszJoins[] = {
			// Go strictly in lexical order, which means HA < HB < ... HT < PA < PB ... < PL
			"HA:a-PD:d",
			"HA:b-HE:f",
			"HA:c-PB:e",
			"HA:d-HB:a",
			"HA:e-PL:a",
			"HA:f-HQ:b",

			"HB:b-PB:d",
			"HB:c-HC:f",
			"HB:d-PA:a",
			"HB:e-HS:b",
			"HB:f-PL:b",

			"HC:a-PB:c",
			"HC:b-HF:e",
			"HC:c-PE:e",
			"HC:d-HD:a",
			"HC:e-PA:b",

			"HD:b-PE:d",
			"HD:c-HH:e",
			"HD:d-PC:a",
			"HD:e-HT:c",
			"HD:f-PA:c",

			"HE:a-PD:c",
			"HE:b-HI:f",
			"HE:c-PF:e",
			"HE:d-HF:a",
			"HE:e-PB:a",

			"HF:b-PF:d",
			"HF:c-HG:f",
			"HF:d-PE:a",
			"HF:f-PB:b",

			"HG:a-PF:c",
			"HG:b-HJ:e",
			"HG:c-PG:e",
			"HG:d-HH:a",
			"HG:e-PE:b",

			"HH:b-PG:d",
			"HH:c-HL:e",
			"HH:d-PC:b",
			"HH:f-PE:c",

			"HI:a-PD:b",
			"HI:b-HM:f",
			"HI:c-PH:e",
			"HI:d-HJ:a",
			"HI:e-PF:a",

			"HJ:b-PH:d",
			"HJ:c-HK:f",
			"HJ:d-PG:a",
			"HJ:f-PF:b",

			"HK:a-PH:c",
			"HK:b-HN:e",
			"HK:c-PI:e",
			"HK:d-HL:a",
			"HK:e-PG:b",

			"HL:b-PI:d",
			"HL:c-HP:e",
			"HL:d-PC:c",
			"HL:f-PG:c",

			"HM:a-PD:a",
			"HM:b-HQ:f",
			"HM:c-PJ:e",
			"HM:d-HN:a",
			"HM:e-PH:a",

			"HN:b-PJ:d",
			"HN:c-HO:f",
			"HN:d-PI:a",
			"HN:f-PH:b",

			"HO:a-PJ:c",
			"HO:b-HR:e",
			"HO:c-PK:e",
			"HO:d-HP:a",
			"HO:e-PI:b",

			"HP:b-PK:d",
			"HP:c-HT:e",
			"HP:d-PC:d",
			"HP:f-PI:c",

			"HQ:a-PD:e",
			"HQ:c-PL:e",
			"HQ:d-HR:a",
			"HQ:e-PJ:a",

			"HR:b-PL:d",
			"HR:c-HS:f",
			"HR:d-PK:a",
			"HR:f-PJ:b",

			"HS:a-PL:c",
			"HS:c-PA:e",
			"HS:d-HT:a",
			"HS:e-PK:b",

			"HT:b-PA:d",
			"HT:d-PC:e",
			"HT:f-PK:c",

			NULL
		};
		dictDefs.SetAt( "H", &aHEdges );
		dictDefs.SetAt( "P", &aPEdges );
		CreateShapePrimitive( pszShapeName, dictDefs, aFaces, aszJoins );
		// Default to scaling by 96% of width - which works for pentagonal and hexagonal faces
		this->SetScaleMethod( CPreferenceSet::SM_BYBASE );
		this->SetBaseRatio( 0.96 );
		return;
	}

	int nHexa = !stricmp( pszShapeName, "hexa" ) || !stricmp( pszShapeName, "hexahedron" );
	int nOcta = !stricmp( pszShapeName, "octa" ) || !stricmp( pszShapeName, "octahedron" );
	if (nHexa || nOcta)
	{
		// 6 faces, 8 edges
		// or 8 faces, 10 edges
		int nFaces;
		if (nOcta)
		{
			nFaces = 8;
			m_szName = "Octahedron";
		}
		else
		{
			nFaces = 6;
			m_szName = "Hexahedron";
		}
		ClearAllFaces();
		static const char *apszNames[] = { "a", "b", "c" };
		static double adLengths[] = { 1.0, 1.0, 1.0 };
		static double adAngles[] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
		static const char *apszFaceNames[] = { "A", "B", "C", "D", "E", "F", "G", "H" };

		// Create faces A, B, C, D, E and F
		int n;
		for (n = 0; n < nFaces; n++) {
			CFace* pf = new CFace( this, apszFaceNames[ n ] );
			// This creates a vertex after each side with the same name
			pf->CreatePolygon( apszNames, adLengths, adAngles, 3 );
			m_mapFaces.SetAt( apszFaceNames[n], pf );
		}

		// Join faces by edge.
		// For hexahedra:
		// Every face A-C is joined to its corresponding D-F on edge c
		// Every face within the modulus group A-C is joined to the
		// next face in rotation with b connected to the next face's a
		// Every face within the modulus group D-F is similarly
		// joined to the next face in rotation, but with its a connected
		// to the next face's b.
		// The same applies to octahedra but using A-D and E-H
		char szA[2] = "";
		char szB[2] = "";
		int nGroupModulus = nFaces / 2;
		for (n=0; n < nGroupModulus; n++)
		{
			szA[0] = 'A' + n;
			szB[0] = 'A' + nGroupModulus + n;
			JoinFaceEdge( szA, "c", szB, "c" );
			szB[0] = 'A' + ((n+1)%nGroupModulus);
			JoinFaceEdge( szA, "b", szB, "a" );
			szA[0] = 'A' + nGroupModulus + n;
			szB[0] = 'A' + nGroupModulus + ((n+1)%nGroupModulus);
			JoinFaceEdge( szA, "a", szB, "b" );
		}
		// Create map of unified vertices
		MapVertices();

		// Set default page width
		m_dLayoutDefaultPageWidth = 2.1;
		m_dLayoutDefaultPageWidthOverHeight = 0.762;

	} // Hexahedron

	if (!stricmp( pszShapeName, "icosa" ) || !stricmp( pszShapeName, "icosahedron" ))
	{
		EDGEDEFDICT dictDefs;
		FACEDEF aFaces[] = {
			{ "T", "A", "c", 90.0 },
			{ "T", "B", "c", 90.0 },
			{ "T", "C", "c", 90.0 },
			{ "T", "D", "c", 90.0 },
			{ "T", "E", "c", 90.0 },

			{ "T", "F", "c", 90.0 },
			{ "T", "G", "c", 90.0 },
			{ "T", "H", "c", 90.0 },
			{ "T", "I", "c", 90.0 },
			{ "T", "J", "a", 90.0 },

			{ "T", "K", "b", 90.0 },
			{ "T", "L", "b", 90.0 },
			{ "T", "M", "b", 90.0 },
			{ "T", "N", "a", 90.0 },
			{ "T", "O", "b", 90.0 },

			{ "T", "P", "c", 90.0 },
			{ "T", "Q", "b", 90.0 },
			{ "T", "R", "a", 90.0 },
			{ "T", "S", "b", 90.0 },
			{ "T", "T", "c", 90.0 },

			{ NULL, NULL }
		};
		const char *aszEdgeNames[] = { "a", "b", "c" };
		double adEdgeLengths1[] = { 1.0, 1.0, 1.0 };
		static double adAngles[] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
		EDGEDEF aTEdges = { 3, aszEdgeNames, adEdgeLengths1, adAngles };
		dictDefs.SetAt( "T", &aTEdges );
		LPCTSTR aszJoins[] = {
			"A:a-G:b",
			"A:b-B:a",
			"A:c-E:a",
			"B:b-S:c",
			"B:c-C:a",
			"C:b-D:a",
			"C:c-F:b",
			"D:b-T:c",
			"D:c-L:b",
			"E:b-F:a",
			"E:c-I:b",
			"F:c-K:b",
			"G:a-O:a",
			"G:c-H:a",
			"H:b-I:a",
			"H:c-M:a",
			"I:c-J:a",
			"J:b-K:a",
			"J:c-N:b",
			"K:c-L:a",
			"L:c-R:c",
			"M:b-N:a",
			"M:c-P:b",
			"N:c-R:b",
			"O:b-P:a",
			"O:c-S:a",
			"P:c-Q:a",
			"Q:b-R:a",
			"Q:c-T:b",
			"S:b-T:a",
			NULL
		};
		CreateShapePrimitive( pszShapeName, dictDefs, aFaces, aszJoins );

		// Set default page width
		m_dLayoutDefaultPageWidth = 2.9;
		m_dLayoutDefaultPageWidthOverHeight = 0.762;

	} // Icosahedron

}


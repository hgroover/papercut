/*!

	@file	 Face.cpp

	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: Face.cpp 16 2006-04-15 06:39:12Z henry_groover $

	Face of a polyhedron. Part of a shape object.

*/

#include "stdafx.h"
#include "PaperCut.h"
#include "Face.h"
#include "MultiVertex.h"
#include "Shape.h"
#include "ShapeLayout.h"
#include "LogPoint.h"
#include "FaceContent.h"
#include "PageGroup.h"

#include <math.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
// Useful math link: http://www.geocities.com/SiliconValley/2548/BigCalculator.html
// Now it's here: http://www.sun-microsystems.org/BigCalculator/BigCalculator.shtml
// Arbitrary precision calculator: http://rascal.sourceforge.net/cgi-bin/a?b=variables
//3.1415926535897932384626433832795028841971693993751
////0582097494459230781640628620899862803482534211706
////7982148086513282306647093844609550582231725359408
////1284811174502841027019385211055596446229489549303
////8196
// pi / 180 = 0.01745329251994329576923690768488612713442871888541725456097191440171009114603449

long double CFace::m_pi = 3.14159265358979323846264338327950288419716939937510582097494459230781640628620899862803482534211706;
long double CFace::m_pi_over_180 = 0.01745329251994329576923690768488612713442871888541725456097191440171009114603449;
long double CFace::m_pi_x_2 = 6.28318530717958647692528676655900576839433879875021164194988918461563281257241800;
long double CFace::m_pi_over_2 = 1.57079632679489661923132169163975144209858469968755291048747229615390820314310450;
CString CFace::m_strNull( "" );
CString CFace::m_strSpace( " " );

CFace::CFace( CShape* pOwner )
	:	m_szFaceName(),
		m_pEdges(),
		m_pVertices(),
		m_strBody(),
		m_szGroupNames()
{
	m_pOwner = pOwner;
	// Set preference inheritance
	Reset();
	SetParent( pOwner );
	m_fOrientation = 0.0;
	m_pContent = NULL;
	m_PageGroup = NULL;
	m_pEdgeDef = NULL;
}

CFace::CFace( CShape* pOwner, const char *pszName )
	:	m_szFaceName( pszName ),
		m_pEdges(),
		m_pVertices(),
		m_strBody(),
		m_szGroupNames()
{
	m_pOwner = pOwner;
	// Set preference inheritance
	Reset();
	SetParent( pOwner );
	m_fOrientation = 0.0;
	m_pContent = NULL;
	m_PageGroup = NULL;
	m_pEdgeDef = NULL;
}

CFace::CFace( CShape* pOwner, CFace const& src )
{
	CDbg::Out( 2, "  CFace::CFace(CShape*,CFace&): creating face %08x from %08x->%08x\n",
			(void*)this,
			(void*)src.m_pOwner,
			(void*)&src );
	// Set preference inheritance
	Reset();
	SetParent( pOwner );
	m_pEdgeDef = NULL;
	(*this) = src;
	m_pOwner = pOwner;
	m_PageGroup = NULL;
}

CFace::~CFace()
{
	// Delete all entries
	ClearAllEdges();
	ClearAllVertices();
	FreeEdgeDef();
	if (m_pContent)
	{
		//m_pDIB->DeleteObject();
		if (0 == m_pContent->RemoveReference())
		{
			delete m_pContent;
		} // We were the last one to free a reference
		m_pContent = NULL;
	}
}

CFace& CFace::operator =(CFace const& src)
{
	POSITION pos;
	CString szName;
	// Copy edges and vertices individually.  Note that they
	// will NOT be valid until we've fixed up their internal
	// relationships!
	CEdge* pe;
	pos = src.m_pEdges.GetStartPosition();
	while (pos != NULL)
	{
		src.m_pEdges.GetNextAssoc( pos, szName, pe );
		CEdge* peNew = new CEdge( *pe );
		peNew->m_pFace = this;
		m_pEdges.SetAt( szName, peNew );
	}
	CVertex* pv;
	pos = src.m_pVertices.GetStartPosition();
	while (pos != NULL)
	{
		src.m_pVertices.GetNextAssoc( pos, szName, pv );
		CVertex* pvNew = new CVertex( *pv );
		pvNew->m_pFace = this;
		// Fix up edge relations as we go along
		m_pEdges.Lookup( pv->m_pSides[0]->GetName(), pvNew->m_pSides[0] );
		m_pEdges.Lookup( pv->m_pSides[1]->GetName(), pvNew->m_pSides[1] );
		m_pVertices.SetAt( szName, pvNew );
	}
	// Now that we've set up vertices, we can join edge
	// endpoints.
	pos = src.m_pEdges.GetStartPosition();
	while (pos != NULL)
	{
		src.m_pEdges.GetNextAssoc( pos, szName, pe );
		CEdge* peNew;
		if (!m_pEdges.Lookup( pe->GetName(), peNew ))
		{
			ASSERT( true );
			continue;
		} // Error!!!
		m_pVertices.Lookup( pe->m_pEndPoints[0]->GetName(), peNew->m_pEndPoints[0] );
		m_pVertices.Lookup( pe->m_pEndPoints[1]->GetName(), peNew->m_pEndPoints[1] );
	}
	// Copy owner pointer as well, though it will be overwritten
	// in copy constructor
	m_pOwner = src.m_pOwner;

	// Copy body
	m_strBody = src.m_strBody;

	// Copy content and increment reference
	m_pContent = src.m_pContent;
	if (m_pContent)
	{
		m_pContent->AddReference();
	}

	// Copy group ownership
	m_szGroupNames = src.m_szGroupNames;

	// Copy page group
	m_PageGroup = src.GetPageGroup();

	// Copy base edge
	m_szBaseEdgeName = src.m_szBaseEdgeName;

	// Copy orientation
	m_fOrientation = src.m_fOrientation;

	// Copy layout division
	m_szLayoutDivision = src.m_szLayoutDivision;

	// Copy edge def used in CreatePolygon()
	SaveEdgeDef( src.m_pEdgeDef );

	// All objects are valid now
	m_szFaceName = src.m_szFaceName;

	return *this;
}

// Join named edge from this face with edge from another face
int
CFace::JoinFaceEdge( const char *pszMyEdgeName, CFace* pOtherFace, const char *pszOtherEdgeName )
{
	CEdge* pMyEdge;
	if (!m_pEdges.Lookup( pszMyEdgeName, pMyEdge ))
	{
		return -1;
	} // Failed
	CEdge* pOtherEdge;
	if (!pOtherFace->m_pEdges.Lookup( pszOtherEdgeName, pOtherEdge ))
	{
		return -1;
	} // Failed
	// Join edges
	pMyEdge->JoinOutward( pOtherEdge );
	return 0;
} // CFace::JoinFaceEdge()

// Create closed polygonal face. Edge names, lengths and angles go clockwise from
// the outside of the shape. All faces need to be added from a consistent perspective.
int
CFace::CreatePolygon( const char **ppszEdgeNames, double* pdLengths, double* pdAngles, int nSides )
{
	int n;
	CEdge* peStart = NULL;
	CEdge* peLast;
	if (nSides < 3)
	{
		return -1;
	}
	// Save edge definition
	SaveEdgeDef( ppszEdgeNames, pdLengths, nSides, pdAngles );
	// Create sides. If any angles are specified != 0.0, we won't calculate inside angles.
	bool anyAnglesSpecified = false;
	for (n = 0; n < nSides; n++)
	{
		CEdge* pe = AddEdge( ppszEdgeNames[n], pdLengths[n] );
		if (!peStart)
		{
			peStart = pe;
		}
		else
		{
			// Name vertices according to first edge + second edge
			CString szVertexName;
			szVertexName.Format( "%s.%s", ppszEdgeNames[n-1], ppszEdgeNames[n] );
			JoinEdge( szVertexName, peLast, pe );
		}
		// Angles are specified after the leg name and length in the shapedef file.
		// The angle (specified in degrees) is the measure of the angle following the leg (0 if the angle will
		// be calculated). However, the vertex we attach the angle measure to is not created and joined to the 
		// leg we're adding it to until we've created the next leg. So we keep the previous leg and use the
		// previous leg's end angle measure here.
		if (n > 0 && pdAngles[n] != 0.0)
		{
			anyAnglesSpecified = true;
			peLast->m_pEndPoints[1]->m_dInsideAngle = pdAngles[n-1];
		}
		peLast = pe;
	} // for all sides
	// Close by joining last with first
	CString szVertexName;
	szVertexName.Format( "%s.%s", ppszEdgeNames[nSides-1], ppszEdgeNames[0] );
	JoinEdge( szVertexName, peLast, peStart );
	if (pdAngles[nSides-1] != 0.0)
	{
		anyAnglesSpecified = true;
		peLast->m_pEndPoints[1]->m_dInsideAngle = pdAngles[nSides-1];
	}
	// Calculate inside angles only if none specified
	// FIXME it is possible to specify m angles where m = nsides - 1, and calculate remaining angles
	if (!anyAnglesSpecified)
	{
		CalcInsideAngles( true );
	}
	return 0;
} // CFace::CreatePolygon()

// Join two edges, creating a vertex
void
CFace::JoinEdge( const char *pszVertexName, CEdge *pEdge1, CEdge *pEdge2, bool bIsPivot /*= false*/ )
{
	CVertex* pv = new CVertex( this );
	pv->m_szVertexName = pszVertexName;
	pv->m_bPivot = bIsPivot;
	pv->m_pSides[0] = pEdge1;
	pv->m_pSides[1] = pEdge2;
	pEdge1->m_pEndPoints[1] = pv;
	pEdge2->m_pEndPoints[0] = pv;
	// Add to our map of vertices
	m_pVertices.SetAt( pszVertexName, pv );
} // CFace::JoinEdge()

// Create edge of specified length with a symbolic name.
// name MUST be unique within face
CEdge*
CFace::AddEdge( const char *pszEdgeName, double dLength )
{
	CEdge* pe = new CEdge( this, pszEdgeName );
	pe->m_dLength = dLength;
	m_pEdges.SetAt( pszEdgeName, pe );
	return pe;
} // CFace::AddEdge()


CEdge* CFace::GetCommonEdge(CFace *pOtherFace)
{
	// Weed out test against ourselves
	if (pOtherFace == this)
	{
		return NULL;
	} // Test is irrelevant

	// Go through our edge list
	POSITION pos;
	CString szName;
	CEdge* pe;
	for (pos = m_pEdges.GetStartPosition(); pos != NULL; )
	{
		m_pEdges.GetNextAssoc( pos, szName, pe );
		CEdge* pe2 = pe->m_pOutwardConnectedEdge;
		if (!pe2) continue;
		if (pe2->m_pFace == pOtherFace)
		{
			return pe;
		}
		if (!pe2->m_pFace) continue;
		if (m_szFaceName.Compare( pe2->m_pFace->m_szFaceName ) == 0)
		{
			return pe;
		} // Compare by name succeeded
	} // for all edges

	// No common edge found
	return NULL;
}

double CFace::DegToRadans(double dDeg)
{
	return dDeg * m_pi_over_180;
}

double CFace::RadansToDeg(double dr)
{
	return dr / m_pi_over_180;
}

// Reduce a value modulo pi
double CFace::ModuloPi( double angle )
{
	if (angle > m_pi)
	{
		int reduceBy = (int)(angle / m_pi);
		angle -= reduceBy * m_pi;
	}
	else if (angle < -m_pi)
	{
		int increaseBy = (int)(-angle / m_pi);
		angle += increaseBy * m_pi;
	}
	return angle;
}

// Find fully qualified edge
CEdge*
CFace::FindFQEdge( const char* pszFQEdgeName, CMap<CString,const char*,CFace*,CFace*>& mapFace )
{
	CString sz( pszFQEdgeName );
	CString szFace;
	szFace = sz.SpanExcluding( ":" );
	sz = sz.Mid( szFace.GetLength() + 1 );
	CFace* pFace;
	if (!mapFace.Lookup( szFace, pFace ))
	{
		return NULL;
	} // Could not find face
	CEdge* pEdge;
	if (!pFace->m_pEdges.Lookup( sz, pEdge ))
	{
		return NULL;
	} // COuld not find edge
	return pEdge;
} // CFace::FindFQEdge()

// Create symbolic representation of face
// "face", "{", "name x", "orient x", edges, vertices, "}"
// Returns comma-delimited list for debugging
CString
CFace::SaveSymbolic( CArray<CString,const char*>& a ) const
{
	CString szReturn;
	CString szCvt;
	//CString* pstr;

	a.Add( "object face" );
	a.Add( "{" );

	MAKESYMBOLIC( "name", m_szFaceName );
	if (!m_szLayoutDivision.IsEmpty())
	{
		MAKESYMBOLIC( "division", m_szLayoutDivision );
	}
	if (m_pContent)
	{
		CString sz;
		sz = m_pContent->GetSerialized( m_pOwner );
		if (!sz.IsEmpty())
		{
			MAKESYMBOLIC( "content", sz );
		}
	}
	if (!m_strBody.IsEmpty())
	{
		CString sz;
		sz = m_strBody;
		sz.Replace( "\r\n", "^" );
		sz.Replace( "\n", "^" );
		sz.Replace( "\r", "^" );
		MAKESYMBOLIC( "text", sz );
	}
	MAKESYMBOLIC_CVT( "orient", m_fOrientation );
	MAKESYMBOLIC( "groups", m_szGroupNames );
	MAKESYMBOLIC( "base", m_szBaseEdgeName );

	// Get any properties we may have set
	CString szProperties( GetSymbolicForm() );
	szProperties.TrimRight();
	if (!szProperties.IsEmpty())
	{
		MAKESYMBOLIC( "settings", szProperties );
	}

	//int NumEdges = m_pEdges.GetCount();
	//MAKESYMBOLIC_CVT( NumEdges );
	//int NumVertices = m_pVertices.GetCount();
	//MAKESYMBOLIC_CVT( NumVertices );

	POSITION pos;
	CString szName;
	CEdge* pe;
	for (pos = m_pEdges.GetStartPosition(); pos != NULL; )
	{
		m_pEdges.GetNextAssoc( pos, szName, pe );
		szReturn += '{';
		szReturn += pe->SaveSymbolic( a );
		szReturn += "},";
	} // for all edges

	CVertex* pv;
	for (pos = m_pVertices.GetStartPosition(); pos != NULL; )
	{
		m_pVertices.GetNextAssoc( pos, szName, pv );
		szReturn += '{';
		szReturn += pv->SaveSymbolic( a );
		szReturn += "},";
	} // for all vertices

	// Close face
	a.Add( "}" );

	return szReturn;
} // CFace::SaveSymbolic()

// Parse a single element.  If bAllowExternalJoins, hook up
// outward connections (should be false on pass 1)
int CFace::ParseElement(LPCTSTR lpElement, bool bAllowExternalJoins /*=true*/)
{
	if (!lpElement)
	{
		return PE_ERROR;
	}
	if (!*lpElement)
	{
		return PE_EMPTY;
	}
	CString sz( lpElement );
	// Get first token
	sz = sz.SpanExcluding( " \t" );
	// Skip over first token
	lpElement += sz.GetLength();
	// Skip over intervening whitespace
	lpElement += strspn( lpElement, " \t" );
	if (!sz.CompareNoCase( "name" ))
	{
		this->m_szFaceName = lpElement;
		return PE_ATTRIBUTE;
	}
	else if (!sz.CompareNoCase( "division" ))
	{
		this->m_szLayoutDivision = lpElement;
		return PE_ATTRIBUTE;
	}
	else if (!sz.CompareNoCase( "orient" ))
	{
		m_fOrientation = (float)atof( lpElement );
		return PE_ATTRIBUTE;
	}
	else if (!sz.CompareNoCase( "text" ))
	{
		m_strBody = lpElement;
		m_strBody.Replace( "^", "\r\n" );
		return PE_ATTRIBUTE;
	}
	else if (!sz.CompareNoCase( "content" ))
	{
		if (!m_pContent)
		{
			m_pContent = new CFaceContent();
		}
		// Let content object handle its own serialization
		m_pContent->FromSerialized( lpElement, m_pOwner );
		return PE_CONTENT;
	}
	else if (!sz.CompareNoCase( "groups" ))
	{
		m_szGroupNames = lpElement;
		return PE_ATTRIBUTE;
	}
	else if (!sz.CompareNoCase( "base" ))
	{
		m_szBaseEdgeName = lpElement;
		return PE_ATTRIBUTE;
	}
	else if (!sz.CompareNoCase( "settings" ))
	{
		// Pass to underlying CPreferenceSet object
		ParseSymbolicForm( CString( lpElement ) );
		return PE_ATTRIBUTE;
	}
	else if (!sz.Compare( "{" ))
	{
		return PE_OPEN;
	}
	else if (!sz.Compare( "}" ))
	{
		return PE_CLOSE;
	}
	else if (!sz.CompareNoCase( "object" ))
	{
		// Check for permissible objects
		if (!stricmp( lpElement, "edge" ))
		{
			return PE_OBJECT_EDGE;
		}
		else if (!stricmp( lpElement, "vertex" ))
		{
			return PE_OBJECT_VERTEX;
		}

		CDbg::Out( "Got unexpected object in %s:%u - %s %s\n",
			__FILE__,
			__LINE__,
			(LPCTSTR)sz,
			lpElement );
		return PE_OBJECT_UNKNOWN;
	}

	CDbg::Out( 0, "Got unexpected element in %s:%u - %s %s\n",
			__FILE__,
			__LINE__,
			(LPCTSTR)sz,
			lpElement );
	return PE_UNKNOWN;

}

// Restore from symbolic representation, stage 1
// name, orientation, numedges, numvertices
// This is where we allocate all the edge and vertex objects.
int
CFace::CreateFromSymbolic1( CArray<CString,const char*>& a )
{
	// What we have:
	/****
	object face
	{
		name A
		orient 135.000
		object edge
		{
			name a
			...
		}
		object vertex
		{
			name a.b
			...
		}
		...
	}
	****/
	ClearAllEdges();
	ClearAllVertices();
	int nCurrent;
	int nLimit = a.GetSize();
	CArray<CString,const char*> aObject;
	int PEReturn;
	// Minimum elements for an empty object is 3
	if (nLimit < 3)
	{
		return PE_ERROR;
	}
	// First element should be object <ourselves> which we ignore
	if (ParseElement( a[0], false ) != PE_OBJECT_UNKNOWN)
	{
		return PE_ERROR;
	}
	// Next should be opening brace
	if (ParseElement( a[1], false ) != PE_OPEN)
	{
		return PE_ERROR;
	}
	int nNested;
	for (nCurrent = 2; nCurrent < nLimit; nCurrent++)
	{
		PEReturn = ParseElement( a[nCurrent], false );
		if (PEReturn == PE_ERROR)
		{
			return PEReturn;
		}
		// Treat unknowns as error
		if (PEReturn == PE_UNKNOWN || PEReturn == PE_OBJECT_UNKNOWN)
		{
			return PE_ERROR;
		}
		// Check for unexpected end
		if (PEReturn == PE_CLOSE)
		{
			// If last one, it's ok
			if (nCurrent == nLimit - 1)
			{
				break;
			}
			CDbg::Out( 0, "Warning: unexpected ending brace in %s:%u parsing element %d\n",
				__FILE__,
				__LINE__,
				nCurrent );
			return 0;
		}
		// Ignore all other except objects
		if (!IsObject( PEReturn ))
		{
			continue;
		}
		// Now we have an object.  Make sure there is at least one
		// following element.  MoveToArray() will ensure it is a brace
		// and is matched.
		if (nCurrent >= nLimit-1)
		{
			return PE_ERROR;
		}
		// Move to array but don't delete
		aObject.RemoveAll();
		aObject.Add( a[nCurrent] );
		if (CShape::MoveObject( aObject, a, nCurrent + 1, false ) < 0)
		{
			return PE_ERROR;
		}
		// Dispatch to appropriate object
		switch (PEReturn)
		{
		case PE_OBJECT_EDGE:
			{
			CEdge* pEdge = new CEdge( this );
			nNested = pEdge->CreateFromSymbolic1( aObject );
			if (nNested < 0)
			{
				delete pEdge;
				return PE_ERROR;
			}
			// pEdge should now have its name set properly
			m_pEdges.SetAt( pEdge->GetName(), pEdge );
			}
			// Skip to last entry of face - for ( ; ; ) will skip past it
			nCurrent += (nNested - 1);
			break;
		case PE_OBJECT_VERTEX:
			{
			CVertex* pVertex = new CVertex( this );
			nNested = pVertex->CreateFromSymbolic1( aObject );
			if (nNested < 0)
			{
				delete pVertex;
				return PE_ERROR;
			}
			m_pVertices.SetAt( pVertex->GetName(), pVertex );
			}
			// Skip to last entry of face - for ( ; ; ) will skip past it
			nCurrent += (nNested - 1);
			break;
		default:
			return PE_ERROR;
		}
	}
	// Return number of elements processed
	return nLimit;
} // CFace::CreateFromSymbolic1()

// Restore from symbolic representation, stage 2
// This is where we recurse down and initialize all the
// edge and vertex objects.
int
CFace::FixupSymbolic()
{
	POSITION pos;
	CString szName;
	CEdge* pe;
	for (pos = m_pEdges.GetStartPosition(); pos != NULL; )
	{
		m_pEdges.GetNextAssoc( pos, szName, pe );
		pe->FixupSymbolic( this->m_pVertices );
	} // for all edges

	CVertex* pv;
	for (pos = m_pVertices.GetStartPosition(); pos != NULL; )
	{
		m_pVertices.GetNextAssoc( pos, szName, pv );
		pv->FixupSymbolic( this->m_pEdges );
	} // for all vertices

	// Calculate inside angles
	CalcInsideAngles();

	// Success
	return 0;
} // CFace::FixupSymbolic()


// Cut connection with other faces.  Returns number
// of previous connections.
int
CFace::DetachEdges( bool bSaveSymbolic /*= false*/ )
{
	POSITION pos;
	CString szName;
	CEdge* pe;
	int nReturn = 0;
	for (pos = m_pEdges.GetStartPosition(); pos != NULL; )
	{
		m_pEdges.GetNextAssoc( pos, szName, pe );
		nReturn += pe->Detach( bSaveSymbolic );
	} // for all edges
	return nReturn;
} // CFace::DetachEdges()

// Get all face:edge joins in a pair of arrays
// Return number of joined pairs.  Always return at least 1 with empty destination.
int
CFace::GetJoinList( CArray<CString,const char*>& aSource,
	CArray<CString,const char*>& aDest ) const
{
	POSITION pos;
	CString szName;
	CString szOuterJoin;
	CEdge* pe;
	int nReturn = 0;
	for (pos = m_pEdges.GetStartPosition(); pos != NULL; )
	{
		m_pEdges.GetNextAssoc( pos, szName, pe );
		if (!pe->GetOuterJoinFQName( szOuterJoin ))
		{
			continue;
		} // Not joined
		CDbg::Out( "\t\tCFace::GetJoinList: adding %s(%f) -> %s(%f)\n",
				(LPCTSTR)pe->GetFQName(),
				pe->m_dLength,
				(LPCTSTR)szOuterJoin,
				pe->m_pOutwardConnectedEdge->m_dLength );
		ASSERT( pe->m_dLength == pe->m_pOutwardConnectedEdge->m_dLength );
		// Debug breakpoint assert:
		//ASSERT( pe->GetFQName().Compare( "Ds1s1:b" ) != 0 );
		aSource.Add( pe->GetFQName() );
		aDest.Add( szOuterJoin );
		nReturn++;
	} // for all edges
	if (!nReturn && pe)
	{
		CDbg::Out( "\t\tCFace::GetJoinList: adding %s -> NULL\n",
				(LPCTSTR)pe->GetFQName() );
		aSource.Add( pe->GetFQName() );
		szOuterJoin.Empty();
		aDest.Add( szOuterJoin );
		nReturn++;
	}
	return nReturn;
} // CFace::GetJoinList()

// Get vertex coordinates recursively.  dOrientation is the orientation
// of a ray which bisects the inside angle of the starting vertex.
int
CFace::GetVertexCoordinates( CFace* pOrigin, int nStartVertex, CLogPoint const& StartPoint,
					double dOrientation, CArray<CLogPoint,CLogPoint const&>& a )
{
	// Get my vertex coordinates, starting with 0 at the bottom center.

	CVertex* pv;
	CLogPoint Start( StartPoint );
	// Build array of vertices
	CArray<CVertex*,CVertex*> av;
	int nSize = m_pVertices.GetCount();
	int nOrder;
	pv = GetFirstVertex();
	for (nOrder = 0; pv != NULL && nOrder < nSize; nOrder++)
	{
		av.Add( pv );
		pv = pv->m_pSides[1]->m_pEndPoints[1];
	} // for all vertices
	CDbg::Out( 2, "Start get coords face(%s)\r\n", (LPCTSTR)m_szFaceName );
	int nOffset;
	nSize = av.GetSize();
	// nStartVertex will be non-zero when called from another vertex
	CString szDivision = this->m_pOwner->GetActiveDivision();
	for (nOffset = 0; nOffset < nSize; nOffset++)
	{
		int nIndex = (nStartVertex + nOffset) % nSize;
		pv = av[nIndex];
		CDbg::Out( 2, "vertex %d start %f, %f\r\n", nIndex, Start.m_dX, Start.m_dY );
		a.Add( Start );
		// Also save location in vertex itself for layout collision detection
		*(pv->m_loc) = Start;
		pv->m_locSet = true;
		// Check for congruent vertex in a connected face
		// There are potentially four candidates
		CVertex* pvc;
		pvc = GetLeastCongruentVertex( pv, pv->m_pSides[0]->m_pOutwardConnectedEdge );
		// Note that we must also guard against looping
		if (pvc && pvc->m_pFace != pOrigin && pvc->m_pFace->GetLayoutDivision() == szDivision)
		{
			// Starting orientation is current + pv inside angle / 2 + pvc inside angle / 2
			double dNewOrientation = dOrientation + pv->m_dInsideAngle / 2 + pvc->m_dInsideAngle / 2;
			pvc->m_pFace->GetVertexCoordinates( this, pvc->GetOrder(), Start,
					dNewOrientation, a );
		} // Found one
		pvc = GetLeastCongruentVertex( pv, pv->m_pSides[1]->m_pOutwardConnectedEdge );
		if (pvc && pvc->m_pFace != pOrigin && pvc->m_pFace->GetLayoutDivision() == szDivision)
		{
			// Starting orientation is current - pv inside angle / 2 - pvc inside angle / 2
			double dNewOrientation = dOrientation - pv->m_dInsideAngle / 2 - pvc->m_dInsideAngle / 2;
			pvc->m_pFace->GetVertexCoordinates( this, pvc->GetOrder(), Start,
					dNewOrientation, a );
		} // Found another
		// Now move start to next vertex.
		// Given an orientation, the leg is always orientation - this vertex's inside angle / 2
		// The next orientation will be that leg + 180 - the next vertex's inside angle / 2
		CVertex* pvn = av[(nIndex+1)%nSize];
		CDbg::Out( 2, "joining %s to %s, angles %f & %f, orientation %f\r\n", (LPCTSTR)pv->GetName(),
				(LPCTSTR)pvn->GetName(), pv->m_dInsideAngle, pvn->m_dInsideAngle, dOrientation );
		double dLegOrientation = fmod( dOrientation - pv->m_dInsideAngle / 2 + 360.0, 360.0 );
		dOrientation = fmod( dLegOrientation + 180 - pvn->m_dInsideAngle / 2 + 360.0, 360.0 );
		double dLegLength = pv->m_pSides[1]->m_dLength;

		// Draw tab if no outward connected edge or if outward connected face is in another division
		bool TabRequired = (pv->m_pSides[1]->m_pOutwardConnectedEdge == NULL);
		if (!TabRequired)
		{
			TabRequired = (pv->m_pSides[1]->m_pOutwardConnectedEdge->m_pFace->GetLayoutDivision() != szDivision);
		}
		if (TabRequired)
		{
			double dXt = Start.m_dX;
			double dYt = Start.m_dY;
			// Use 30 degree slope for tab ends
			double dTabAngle = 30.0;
			// Use 1/9 leg length for tab width
			double dTabw = dLegLength / 9;
			// Tab hypotenuse
			double dTabh = dTabw / sin( DegToRadans( dTabAngle ) );
			// Tab base
			double dTabb = dTabw / tan( DegToRadans( dTabAngle ) );
			// Tab edge
			double dTabe = dLegLength - 2 * dTabb;
			VectorMove( dXt, dYt, dLegOrientation - dTabAngle, dTabh );
			a.Add( CLogPoint( dXt, dYt ) );
			VectorMove( dXt, dYt, dLegOrientation, dTabe );
			a.Add( CLogPoint( dXt, dYt ) );
			//VectorMove( dXt, dYt, dLegOrientation + dTabAngle, dTabh );
			// should be the same as next point
		} // outer edge, draw tab

		VectorMove( Start.m_dX, Start.m_dY, dLegOrientation, dLegLength );

		CDbg::Out( 2, "end %f, %f\r\n", Start.m_dX, Start.m_dY );
	} // for all vertices

	// Go through all connected faces
	// Use coordinates of my vertex which is congruent to vertex 0 of each
	// connected face for the next start

	// Indicate success
	return 0;
} // CFace::GetVertexCoordinates()

// Render face recursively. Sets m_loc on all vertices
void
CFace::Render( CDC *pDC, CFace* pOrigin, int nStartVertex, CLogPoint const& StartPoint,
		LPCTSTR szDivision,
		double dOrientation, double dScale /*= 1.0*/, double dPrintScale /* = 1.0 */ )
{
	CVertex* pv;
	CString szName;
	CLogPoint Start( StartPoint );
	// If printing, pens need to be adjusted for resolution. Use 0.25mm (0.0155" which we'll round up to 0.02")
	BOOL bPrinting = pDC->IsPrinting();
	int penWidthEdge = 1;
	int penWidthTab = 1;
	if (bPrinting)
	{
		int mapMode = pDC->GetMapMode();
		int pixPerInch = pDC->GetDeviceCaps( LOGPIXELSX );
		switch (mapMode)
		{
		case MM_HIENGLISH: // 0.001 inch
			penWidthEdge *= 25;
			penWidthTab *= 16;
			break;
		case MM_LOENGLISH: // 0.01 inch
			penWidthEdge *= 2;
			break;
		case MM_HIMETRIC: // 0.01mm
			penWidthEdge *= 40;
			penWidthTab *= 16;
			break;
		case MM_LOMETRIC: // 0.1mm
			penWidthEdge *= 4;
			penWidthTab *= 2;
			break;
		case MM_TWIPS: // 1/1440"
			penWidthEdge *= 36;
			penWidthTab *= 24;
			break;
		default: // MM_TEXT, MM_ISOTROPIC, MM_ANISOTROPIC
			// Select appropriate width in device pixels
			penWidthEdge = pixPerInch * 0.025;
			penWidthTab = pixPerInch * 0.0155;
		}
	}

	// Get default face text if specified
	CString szDefaultFaceText( GetDefaultFaceText() );
	// Create dashed pen for concave edges
	CPen penConcave, penNormal, penTab;
	bool bFaceBgSolid, bTabBgSolid;
	COLORREF clrPen = GetEdgeColor();
	penNormal.CreatePen( PS_SOLID, penWidthEdge, clrPen );
	clrPen = GetInwdEdgeColor();
	penConcave.CreatePen( PS_DASH, penWidthEdge, clrPen );
	clrPen = GetTabColor();
	penTab.CreatePen( PS_SOLID, penWidthTab, clrPen );
	COLORREF clrBrush = GetFaceBgColor();
	bFaceBgSolid = (clrBrush != RGB(255,255,255));
	CBrush brFace, brTab;
	brFace.CreateSolidBrush( clrBrush );
	clrBrush = GetTabBgColor();
	bTabBgSolid = (clrBrush != RGB(255,255,255));
	brTab.CreateSolidBrush( clrBrush );
	CPen* pOldPen = pDC->SelectObject( &penNormal );
	CBrush* pOldBrush = pDC->SelectObject( &brFace );

	// Create default fonts to be used in content caption rendering
	// FIXME we need to accommodate rotated content!
	CFont fContentCaption;
	{
		// Create fonts of specified size
		LOGFONT lf;
		unsigned long uFontAttributes = GetFontAttributes();
		memset( &lf, 0, sizeof( lf ) );
		strncpy( lf.lfFaceName, GetFontTypeface(), sizeof( lf.lfFaceName ) );
		CSize sLarge;
		sLarge.cx = 0;
		sLarge.cy = MulDiv( GetFontPointsize(), 3 * pDC->GetDeviceCaps( LOGPIXELSY ), 4 * 72 );
		pDC->DPtoLP( &sLarge );
		lf.lfWeight = uFontAttributes&0xfff;
		lf.lfItalic = uFontAttributes&0x1000 ? 1 : 0;
		lf.lfUnderline = uFontAttributes&0x2000 ? 1 : 0;
		lf.lfStrikeOut = uFontAttributes&0x4000 ? 1 : 0;

		// Create font(s)
		lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
		lf.lfHeight = -sLarge.cy;
		fContentCaption.CreateFontIndirect( &lf );
	}

	// Build array of vertices
	CArray<CVertex*,CVertex*> av;
	int nSize = m_pVertices.GetCount();
	int nOrder;
	pv = GetFirstVertex();
	for (nOrder = 0; pv != NULL && nOrder < nSize; nOrder++)
	{
		av.Add( pv );
		pv = pv->m_pSides[1]->m_pEndPoints[1];
	} // for all vertices
	CDbg::Out( 2, "Start render face(%s)\r\n", (LPCTSTR)m_szFaceName );
	int nOffset;
	nSize = av.GetSize();
	// Do background fill
	POINT pt[16];
	ASSERT( nSize <= sizeof(pt)/sizeof(pt[0]) );
	size_t PointCount;
	CLogPoint Start2( Start );
	for (nOffset = 0; nOffset < nSize; nOffset++)
	{
		int nIndex = (nStartVertex + nOffset) % nSize;
		pv = av[nIndex];
		// Save location for use within this function
		pv->m_loc->Set( Start2.m_dX, Start2.m_dY );
		CVertex* pvn = av[(nIndex+1)%nSize];
		double dLegOrientation = fmod( dOrientation - pv->m_dInsideAngle / 2 + 360.0, 360.0 );
		double dCurrentOrientation = dOrientation;
		dOrientation = fmod( dLegOrientation + 180 - pvn->m_dInsideAngle / 2 + 360.0, 360.0 );
		double dLegLength = pv->m_pSides[1]->m_dLength * dScale;

		// Draw leg
		CLogPoint End( Start2 );
		VectorMove( End.m_dX, End.m_dY, dLegOrientation, dLegLength );
		//CDbg::Out( 2, "leg orient %f slope %f sin() = %f cos() = %f  delta = %f, %f newpos = %f, %f\r\n",
		//	dLegOrientation, RadansToDeg( dSlopeRadians ), sin( dSlopeRadians ), cos( dSlopeRadians ),
		//	XDelta, YDelta, XEnd, YEnd );
		int nX1, nY1;
		nX1 = (int)floor( Start2.m_dX + 0.5 );
		nY1 = (int)floor( Start2.m_dY + 0.5 );
		pt[nOffset].x = nX1;
		pt[nOffset].y = nY1;
		Start2 = End;
	}
	PointCount = nOffset;
	// Fill if not white (transparent)
	if (bFaceBgSolid)
	{
		pDC->Polygon( pt, PointCount );
	}
	for (nOffset = 0; nOffset < nSize; nOffset++)
	{
		int nIndex = (nStartVertex + nOffset) % nSize;
		pv = av[nIndex];
		// Check for congruent vertex in a connected face
		// There are potentially four candidates
		CVertex* pvc;
		pvc = GetLeastCongruentVertex( pv, pv->m_pSides[0]->m_pOutwardConnectedEdge );
		// Note that we must also guard against looping
		if (pvc && pvc->m_pFace != pOrigin && pvc->m_pFace->GetLayoutDivision() == szDivision)
		{
			// Starting orientation is current + pv inside angle / 2 + pvc inside angle / 2
			double dNewOrientation = dOrientation + pv->m_dInsideAngle / 2 + pvc->m_dInsideAngle / 2;
			pvc->m_pFace->Render( pDC, this, pvc->GetOrder(), Start, szDivision,
					dNewOrientation, dScale, dPrintScale );
		} // Found one
		pvc = GetLeastCongruentVertex( pv, pv->m_pSides[1]->m_pOutwardConnectedEdge );
		// FIXME - rendering should be driven by the layout - we shouldn't be continually checking
		// the active layout division here.
		if (pvc && pvc->m_pFace != pOrigin && pvc->m_pFace->GetLayoutDivision() == szDivision)
		{
			// Starting orientation is current - pv inside angle / 2 - pvc inside angle / 2
			double dNewOrientation = dOrientation - pv->m_dInsideAngle / 2 - pvc->m_dInsideAngle / 2;
			// Set page group if not present in target
			CDbg::Out( "CFace::Render(this=%08x,%s) - GetPageGroup()=%08x\n", (DWORD)this, (LPCTSTR)this->m_szFaceName, GetPageGroup() );
			if (GetPageGroup() != NULL)
				pvc->m_pFace->SetPageGroup( this->GetPageGroup() );
			pvc->m_pFace->Render( pDC, this, pvc->GetOrder(), Start, szDivision,
					dNewOrientation, dScale, dPrintScale );
		} // Found another

		// Now move start to next vertex and draw leg
		// We always go clockwise, and orientations also rotate clockwise, which means
		// adding...
		// Given an orientation, the leg is always orientation - this vertex's inside angle / 2
		// The next orientation will be that leg + 180 - the next vertex's inside angle / 2
		CVertex* pvn = av[(nIndex+1)%nSize];
		double dLegOrientation = fmod( dOrientation - pv->m_dInsideAngle / 2 + 360.0, 360.0 );
		double dCurrentOrientation = dOrientation;
		dOrientation = fmod( dLegOrientation + 180 - pvn->m_dInsideAngle / 2 + 360.0, 360.0 );
		double dLegLength = pv->m_pSides[1]->m_dLength * dScale;

		// Draw leg
		CLogPoint End( Start );
		VectorMove( End.m_dX, End.m_dY, dLegOrientation, dLegLength );
		//CDbg::Out( 2, "leg orient %f slope %f sin() = %f cos() = %f  delta = %f, %f newpos = %f, %f\r\n",
		//	dLegOrientation, RadansToDeg( dSlopeRadians ), sin( dSlopeRadians ), cos( dSlopeRadians ),
		//	XDelta, YDelta, XEnd, YEnd );
		int nX1, nY1, nX2, nY2;
		nX1 = (int)floor( Start.m_dX + 0.5 );
		nY1 = (int)floor( Start.m_dY + 0.5 );
		nX2 = (int)floor( End.m_dX + 0.5 );
		nY2 = (int)floor( End.m_dY + 0.5 );

		// Display vertex name along current orientation
		if (CDbg::m_Level > 0)
		{
			double dXText1 = Start.m_dX;
			double dYText1 = Start.m_dY;
			VectorMove( dXText1, dYText1, dCurrentOrientation, 25.0 );
			pDC->TextOut( (int)dXText1, (int)dYText1, pv->GetName() );
			VectorMove( dXText1, dYText1, dCurrentOrientation, 55.0 );
			CString szTemp;
			szTemp.Format( "%s #%d", (LPCTSTR)pv->m_pFace->m_szFaceName, nOffset );
			pDC->TextOut( (int)dXText1, (int)dYText1, szTemp );

		} // Debugging

		// Display edge name if not printing
		if (!bPrinting)
		{
			CString szEdgeText;
			double dXText1 = Start.m_dX;
			double dYText1 = Start.m_dY;
			VectorMove( dXText1, dYText1, dLegOrientation, dLegLength / 2.0 );
			VectorMove( dXText1, dYText1, dLegOrientation + 90.0, 10.0 );
			if (pv->m_pSides[1]->IsBaseEdge())
			{
				szEdgeText.Format( "[%s]", (LPCTSTR)pv->m_pSides[1]->GetFQName() );
			}
			else
			{
				szEdgeText = pv->m_pSides[1]->GetFQName();
			}
			pDC->TextOut( (int)dXText1, (int)dYText1, (LPCTSTR)szEdgeText );
		}

		// Orientation used for image and text is base edge + m_fOrientation
		// relative to epicenter, or relative to midpoint of base for nsides > 3
		BOOL IsBaseEdge = (nOffset == 0);
		IsBaseEdge = (pv->m_pSides[0] == this->GetBaseEdge());

		// Determine epicenter
		double dXEpicenter;
		double dYEpicenter;
		if (this->m_pEdges.GetCount() == 4)
		{
			// For quadrangles, get the intersection of the lines connecting
			// opposite midpoints.
			CVertex *pvnn = pvn->m_pSides[1]->m_pEndPoints[1];
			CVertex *pvp = pv->m_pSides[0]->m_pEndPoints[0];
			dXEpicenter = Start.m_dX;
			dYEpicenter = Start.m_dY;
			double dLegOrientation = CFace::Modulo360( dCurrentOrientation + pv->m_dInsideAngle / 2 );
			//if (IsBaseEdge) pDC->TextOut( (int)dXEpicenter, (int)dYEpicenter, "s" ); // DELETEME
			VectorMove( dXEpicenter, dYEpicenter, dLegOrientation, pv->m_pSides[0]->m_dLength * dScale / 2.0 );
			//if (IsBaseEdge) pDC->TextOut( (int)dXEpicenter, (int)dYEpicenter, "m" ); // DELETEME
			// Move half the average height
			double dHeight1 = pv->m_pSides[0]->m_dLength * dScale;
			double dHeight2 = pvn->m_pSides[1]->m_dLength * dScale;
			if (pv->m_dInsideAngle < 89.5)
			{
				dHeight1 *= sin( CFace::DegToRadans( pv->m_dInsideAngle ) );
			}
			else if (pv->m_dInsideAngle > 90.5)
			{
				dHeight1 *= cos( CFace::DegToRadans( pv->m_dInsideAngle - 90.0 ) );
			}
			if (pvn->m_dInsideAngle < 89.5)
			{
				dHeight2 *= sin( CFace::DegToRadans( pvn->m_dInsideAngle ) );
			}
			else if (pvn->m_dInsideAngle > 90.5)
			{
				dHeight2 *= cos( CFace::DegToRadans( pvn->m_dInsideAngle - 90.0 ) );
			}
			double dHeight = (dHeight1 + dHeight2) / 4;
			VectorMove( dXEpicenter, dYEpicenter, CFace::Modulo360( dLegOrientation - 90.0 ), dHeight );

			//if (IsBaseEdge) pDC->TextOut( (int)dXEpicenter, (int)dYEpicenter, "q" ); // DELETEME
		}
		else
		{
			// pv's inside angle is A and is bisected by ray d
			// pvn's inside angle is B and is bisected by ray e
			// dLegLength is b
			// E is 180-A/2-B/2
			// leg de which bisects A and ends at epicenter is b * sin(B/2)/sin(E)
			double dE = 180 - pv->m_dInsideAngle / 2 - pvn->m_dInsideAngle / 2;
			double dELength = dLegLength * sin( DegToRadans( pvn->m_dInsideAngle / 2 ) ) / sin( DegToRadans( dE ) );
			dXEpicenter = Start.m_dX;
			dYEpicenter = Start.m_dY;
			VectorMove( dXEpicenter, dYEpicenter, dCurrentOrientation, dELength );

			//pDC->TextOut( (int)dXEpicenter, (int)dYEpicenter, "e" ); // DELETEME
		}

		// Display image if specified
		if (IsBaseEdge && m_pContent != NULL && m_pContent->IsLoaded())
		{
			// No stretching by default - get size and center it
			CSize s = m_pContent->GetSize();
			CRect rDest;
			//pDC->TextOut( (int)dXEpicenter, (int)dYEpicenter, "*" ); // DELETEME

			// Use proportionate area
			// Opposite leg is base of triangle
			double dBase = pvn->m_pSides[1]->m_dLength * dScale;
			// Get angle of perpendicular
			double dP = 90 - pvn->m_dInsideAngle;
			// Height perpendicular to base intersects pv
			double dHeight = dLegLength * cos( DegToRadans( dP ) );
			double dArea = dBase * dHeight / 2;
			double dPixArea = s.cx * s.cy;
			double dPixScale = 1.0;
			CSize sd;
			switch (GetScaleMethod())
			{
			case 0:
				dPixScale = GetAreaRatio() * sqrt( dArea / dPixArea );
				break;
			case 1:
				dPixScale = GetHeightRatio() * dHeight / s.cy;
				break;
			case 2:
				dPixScale = GetBaseRatio() * dBase / s.cx;
				break;
			case 3:
				dPixScale = GetOrgRatio();
				if (dPrintScale != 1.0)
				{
					dPixScale *= (dPrintScale / dScale);
				}
				break;
			}
			sd.cx = (int)(dPixScale * s.cx);
			sd.cy = (int)(dPixScale * s.cy);
			CDbg::Out( 1, "f %s b,h %.1lf, %.1lf bmp %d, %d ratio %.3lf new rect %d, %d\n",
				(LPCTSTR)this->m_szFaceName, dBase, dHeight, s.cx, s.cy, dPixScale, sd.cx, sd.cy );
			// Destination rectangle assumes no rotation and is used to pass
			// size and center information.
			rDest.SetRect( (int)(dXEpicenter-sd.cx/2), (int)(dYEpicenter-sd.cy/2),
				(int)(dXEpicenter+sd.cx/2), (int)(dYEpicenter+sd.cy/2) );
			// Set clipping if not overflowing
			//CRgn rgn;
			bool bClipping = (GetPictureFlow() == 0);
			// Enable rotation if specified
			double dImageRotation = 0;
			if (GetEnableRotation() != 0)
			{
				dImageRotation = dLegOrientation; //fmod( dLegOrientation - 90, 360 );
			}
			//if (bClipping)
			//{
			//	rgn.CreatePolygonRgn( pt, sizeof(pt)/sizeof(pt[0]), WINDING );
			//	pDC->SelectClipRgn( &rgn);
			//	//pDC->FillRgn( &rgn, &brTab );
			//}
			// If content has a caption, set font now
			bool bContentCaption = !m_pContent->GetCaption().IsEmpty();
			CFont *pOldFont;
			if (bContentCaption)
			{
				pOldFont = pDC->SelectObject( &fContentCaption );
			}
			m_pContent->Draw( pDC, dImageRotation, &rDest, bClipping, pt, PointCount );
			if (bContentCaption)
			{
				pDC->SelectObject( pOldFont );
			}
			//if (bClipping)
			//{
			//	CRgn rgnNull;
			//	rgnNull.Attach( NULL );
			//	pDC->SelectClipRgn( &rgnNull );
			//	rgn.DeleteObject();
			//}
		} // Display image

		// Display body if specified
		if (IsBaseEdge && (!m_strBody.IsEmpty() || !szDefaultFaceText.IsEmpty()))
		{
			// Process $ escapes in body
			CString szBody( GetBody( szDefaultFaceText ) );
			if (m_pOwner != NULL) m_pOwner->Substitute( szBody, (unsigned int)this, 0 );
			COLORREF rOld = pDC->SetTextColor( GetFaceColor() );
			// Create font of specified size
			LOGFONT lf;
			unsigned long uFontAttributes = GetFontAttributes();
			memset( &lf, 0, sizeof( lf ) );
			strncpy( lf.lfFaceName, GetFontTypeface(), sizeof( lf.lfFaceName ) );
			CSize s;
			s.cx = 0;
			s.cy = MulDiv( GetFontPointsize(), pDC->GetDeviceCaps( LOGPIXELSY ), 72 );
			pDC->DPtoLP( &s );
			lf.lfHeight = -s.cy;
			lf.lfWeight = uFontAttributes&0xfff;
			lf.lfItalic = uFontAttributes&0x1000 ? 1 : 0;
			lf.lfUnderline = uFontAttributes&0x2000 ? 1 : 0;
			lf.lfStrikeOut = uFontAttributes&0x4000 ? 1 : 0;

			// Create font
			lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
			CFont f;
			f.CreateFontIndirect( &lf );

			// Select it
			CFont* pOldFont = pDC->SelectObject( &f );

			// Parse into multiple lines
			static char szBuff[4096];
			strncpy( szBuff, szBody, sizeof( szBuff ) );
			// Make sure it's null-terminated
			szBuff[sizeof( szBuff )-1] = '\0';
			LPTSTR lp[50];
			int nLines = 0;
			LPTSTR lpw;
			int nMaxHeight = 0;
			// Determine maximum line height - this is what we'll use for all lines
			for (lpw = strtok( szBuff, "\r\n" );
				lpw && nLines < sizeof(lp)/sizeof(lp[0]);
				nLines++, lpw = strtok( NULL, "\r\n" ))
				{
					// Lines have been stored with ^ added as placeholders for
					// blank lines
					if (lpw[0] == '^' && lpw[1] == '\0')
					{
						continue;
					}
					// Strip trailing ^
					size_t nlplen = lstrlen( lpw );
					if (lpw[nlplen-1] == '^')
					{
						lpw[--nlplen] = '\0';
					}
					CSize st = pDC->GetTextExtent( lpw, nlplen );
					lp[nLines] = lpw;
					if (st.cy > nMaxHeight)
					{
						nMaxHeight = st.cy;
					}
				}
			int n;
			// Starting at epicenter, draw a perpendicular
			double dXPos, dYPos;
			dXPos = dXEpicenter;
			dYPos = dYEpicenter;
			double dPerpOrientation = fmod( dLegOrientation - (90 - pv->m_dInsideAngle) + 360.0, 360.0 );
			VectorMove( dXPos, dYPos, dPerpOrientation, (nLines*nMaxHeight)/2 );
			//dPerpOrientation = fmod( dPerpOrientation + 180, 360 );
			for (n = 0; n < nLines; n++)
			{
				if (lp[n] && lp[n][0]) RotatedTextOut( pDC, dXPos, dYPos, dPerpOrientation, 0, 0, CString( lp[n] ), false );
				VectorMove( dXPos, dYPos, dPerpOrientation, -nMaxHeight );
			}
			// Restore old font
			pDC->SelectObject( pOldFont );

			// Destroy resources
			f.DeleteObject();

			pDC->SetTextColor( rOld );
		} // Display body text

		pDC->MoveTo( nX1, nY1 );
		if (pv->m_pSides[1]->m_bConcave)
		{
			// Use dashed brush
			(void)pDC->SelectObject( &penConcave );
			pDC->LineTo( nX2, nY2 );
			(void)pDC->SelectObject( &penNormal );
		}
		else
		{
			pDC->LineTo( nX2, nY2 );
		} // normal pen

		CDbg::Out( 2, "vtx %s out at %d, %d - %d, %d\r\n", (LPCTSTR)pv->GetName(), nX1, nY1, nX2, nY2 );

		// Draw tab if no outward connected edge, or if in another division
		bool TabRequired = (pv->m_pSides[1]->m_pOutwardConnectedEdge == NULL);
		bool InOtherDivision = false;
		int TabUsage = GetUseTabs();
		if (!TabRequired && TabUsage != UTB_NONE)
		{
			TabRequired = (pv->m_pSides[1]->m_pOutwardConnectedEdge->m_pFace->GetLayoutDivision() != szDivision);
			InOtherDivision = TabRequired;
		}
		if (TabRequired && TabUsage != UTB_NONE)
		{
			pDC->MoveTo( nX1, nY1 );
			double dXt;
			double dYt;

			// Use 30 degree slope for tab ends
			double dTabAngle = GetTabShoulderAngle();
			// Use 1/9 leg length for tab width
			double dTabw = dLegLength / 9;
			// Tab hypotenuse
			double dTabh = dTabw / sin( DegToRadans( dTabAngle ) );
			// Tab base
			double dTabb = dTabw / tan( DegToRadans( dTabAngle ) );
			// Tab edge
			double dTabe = dLegLength - 2 * dTabb;

			pDC->SelectObject( &brTab );
			// Tab text
			CString szTabText;
			bool notConnected = true;
			if (TabUsage != UTB_BLANK)
			{
				szTabText = pv->m_pSides[1]->m_szTabText;
				// If tab text is empty, put something in there
				if (szTabText.IsEmpty())
				{
					szTabText.Format( "(%s)", (LPCTSTR)pv->m_pSides[1]->GetFQName() );
				}
				else
				{
					notConnected = false;
				}
			}
			if (InOtherDivision && TabUsage != UTB_BLANK)
			{
				// Construct tab text on the fly. Use whichever fully qualified face:edge name is lexically less.
				CString szTab1 = pv->m_pSides[1]->GetFQName();
				szTabText = pv->m_pSides[1]->m_pOutwardConnectedEdge->GetFQName();
				int nCompare = szTab1.Compare( szTabText );
				CDbg::Out( "Compare: is \"%s\" < \"%s\"? (%d)\n", (LPCTSTR)szTab1, (LPCTSTR)szTabText, nCompare );
				if (nCompare < 0)
				{
					// szTab1 is lexically inferior
					szTabText = szTab1;
				}
			}
			if (!szTabText.IsEmpty())
			{
				// Show text in middle of tab facing toward actual edge
				double dTabTextRotation = dLegOrientation + 90.0;
				dXt = Start.m_dX;
				dYt = Start.m_dY;
				// First move to middle of leg
				VectorMove( dXt, dYt, dLegOrientation, dLegLength / 2 );
				// Now move to just inside tab
				VectorMove( dXt, dYt, dTabTextRotation + 180, dTabw * GetTabTextOff() );
				// Font is created rotated and selected in RotatedTextOut()
				//// Render text rotated
				//CFont* pOldFont = pDC->SelectObject( &fTabLargeFont );
				RotatedTextOut( pDC, dXt, dYt, dTabTextRotation, (int)(dTabe*0.90), (int)(dTabw * GetTabTextPct()),
					szTabText );
				if (TabUsage != UTB_NOHINT)
				{

					//// Render help text in smaller font
					//pDC->SelectObject( &fTabSmallFont );
					VectorMove( dXt, dYt, dTabTextRotation + 180, dTabw * (GetTabTextPct() + GetTabTextOff()) );
					CString szHelpRef, szTemp;
					CEdge *pOutward = pv->m_pSides[1]->m_pOutwardConnectedEdge;
					szTemp.Format( "this face: %s", (LPCTSTR)pv->m_pFace->m_szFaceName );
						szHelpRef += szTemp;
					if (pv->m_pFace->GetPageNumber() >= 0)
					{
						szTemp.Format( " on page %d", pv->m_pFace->GetPageNumber() + 1 );
						szHelpRef += szTemp;
					}
					// Get outward connected edge using symbolic reference
					if (pOutward == NULL && !pv->m_pSides[1]->m_szSavedOutwardEdge.IsEmpty())
					{
						pOutward = this->m_pOwner->FindFQEdge( pv->m_pSides[1]->m_szSavedOutwardEdge );
					}
					if (pOutward != NULL)
					{
						szTemp.Format( "  connects to: %s", (LPCTSTR)pOutward->m_pFace->m_szFaceName );
						szHelpRef += szTemp;
						if (pOutward->m_pFace->GetPageNumber() >= 0)
						{
							szTemp.Format( " on page %d", pOutward->m_pFace->GetPageNumber() + 1 );
							szHelpRef += szTemp;
							if (InOtherDivision)
							{
								CString szOutDivision = pOutward->m_pFace->GetLayoutDivision();
								if (szOutDivision.IsEmpty()) szOutDivision = "*";
								szTemp.Format( "{div:%s}", szOutDivision );
								szHelpRef += szTemp;
							}
						}
						if (pv->m_pSides[1]->GetForcedSplit())
						{
							szHelpRef += " [forced]";
						}
					}
					else if (notConnected)
					{
						szHelpRef += " is not connected anywhere";
					}
					// Render connection comment
					RotatedTextOut( pDC, dXt, dYt, dTabTextRotation, (int)(dTabe * GetTabText2Off()), (int)(dTabw * GetTabText2Pct()),
						szHelpRef );
					//// Restore original font selection
					//pDC->SelectObject( pOldFont );
				} // UTB_NOHINT not specified
			} // Tab text specified

			// Draw tab outline
			dXt = Start.m_dX;
			dYt = Start.m_dY;
			VectorMove( dXt, dYt, dLegOrientation - dTabAngle, dTabh );
			pDC->SelectObject( &penTab );
			pDC->LineTo( (int)dXt, (int)dYt );
			VectorMove( dXt, dYt, dLegOrientation, dTabe );
			pDC->LineTo( (int)dXt, (int)dYt );
			VectorMove( dXt, dYt, dLegOrientation + dTabAngle, dTabh );
			pDC->LineTo( (int)dXt, (int)dYt );
			pDC->SelectObject( &penNormal );
			pDC->SelectObject( &brFace );

		} // outer edge, draw tab

		// Change origin
		Start = End;
	} // for all vertices
	CDbg::Out( 2, "end render face(%s)\r\n", (LPCTSTR)m_szFaceName );
	// Select original pen
	pDC->SelectObject( pOldPen );
	pDC->SelectObject( pOldBrush );
#if USE_TAB_FONTS
	// Free GDI resources associated with tab fonts
	fTabLargeFont.DeleteObject();
	fTabSmallFont.DeleteObject();
#endif
	// Free GDI resources associated with pens
	penConcave.DeleteObject();
	penNormal.DeleteObject();
	penTab.DeleteObject();
	brFace.DeleteObject();
	brTab.DeleteObject();
} // CFace::Render()

// Recursively find face containing point at specified scale and rotation
CFace*
CFace::FindContainingFace( CPoint const&pt, CFace* pOrigin, int nStartVertex, CLogPoint const& StartPoint,
	CEdge*& pEdge, double dOrientation, double dScale /*= 1.0*/ )
{
	CFace* pReturn = NULL;
	pEdge = NULL;
	CVertex* pv;
	CString szName;
	CLogPoint Start( StartPoint );

	// Build array of vertices
	CArray<CVertex*,CVertex*> av;
	int nSize = m_pVertices.GetCount();
	int nOrder;
	pv = GetFirstVertex();
	for (nOrder = 0; pv != NULL && nOrder < nSize; nOrder++)
	{
		av.Add( pv );
		pv = pv->m_pSides[1]->m_pEndPoints[1];
	} // for all vertices

	int nOffset;
	nSize = av.GetSize();

	// First check to see if it is within our face.
	// A point is within our face iff it falls within all n
	// vertices' inside angles (FIXME only true for convex polygons!).
	// FIXME This test does NOT work with non-convex polygonal faces!!!
	int nFallsWithin = 0;
	double dTestOrient = dOrientation;
	double dMinEdgeLegSum = 1e10;
	CLogPoint TestStart( StartPoint );
	for (nOffset = 0; nOffset < nSize; nOffset++)
	{
		int nIndex = (nStartVertex + nOffset) % nSize;
		pv = av[nIndex];

		CVertex* pvn = av[(nIndex+1)%nSize];
		double dLegOrientation = fmod( dTestOrient - pv->m_dInsideAngle / 2 + 360, 360 );
		double dPrevLegOrientation = fmod( dTestOrient + pv->m_dInsideAngle / 2 + 360, 360 );
		double dCurrentOrientation = dTestOrient;
		dTestOrient = fmod( dLegOrientation + 180 - pvn->m_dInsideAngle / 2 + 360.0, 360.0 );
		double dLegLength = pv->m_pSides[1]->m_dLength * dScale;

		// Move along leg
		CLogPoint End( TestStart );
		VectorMove( End.m_dX, End.m_dY, dLegOrientation, dLegLength );

		int nX1, nY1, nX2, nY2;
		nX1 = (int)floor( TestStart.m_dX + 0.5 );
		nY1 = (int)floor( TestStart.m_dY + 0.5 );
		nX2 = (int)floor( End.m_dX + 0.5 );
		nY2 = (int)floor( End.m_dY + 0.5 );

		// Get angle from start point to test point
		// Note that Y-axis is reversed with origin at top...
		double xdiff = pt.x - nX1;
		double ydiff = nY1 - pt.y;

		// Get angle from end point to test point
		double x2diff = pt.x - nX2;
		double y2diff = pt.y - nY2;

		// Get sum of lengths of the two legs of a triangle having
		// polygon side as the base and test point as the vertex.
		// Minimum sum is the closest side.
		double dLeg1 = sqrt( xdiff*xdiff + ydiff*ydiff );
		double dLegSum = dLeg1 + sqrt( x2diff*x2diff + y2diff*y2diff );
		if (pEdge == NULL || dLegSum < dMinEdgeLegSum)
		{
			pEdge = pv->m_pSides[1];
			dMinEdgeLegSum = dLegSum;
		}

		// Congruency with a vertex is an immediate "yes" !
		if (xdiff == 0 && ydiff == 0)
		{
			pEdge = pv->m_pSides[1];
			return this;
		}

		// Adjust for quadrant
		double dTestPointOrientation = RadansToDeg( atan2( ydiff, xdiff ) );
		double dOriginal = dTestPointOrientation;
		dTestPointOrientation = 90 - dTestPointOrientation;
		dTestPointOrientation = fmod( 360 + dTestPointOrientation, 360 );

		double dEndTestPointOrientation = RadansToDeg( atan2( y2diff, x2diff ) );
		dEndTestPointOrientation = 90 - dEndTestPointOrientation;
		dEndTestPointOrientation = fmod( 360 + dEndTestPointOrientation, 360 );

		CDbg::Out( 1, "vtx[%d](%s-%s) %s Tst %d from %d,%d -> %d,%d diff %.1f,%.1f next leg ends %d,%d diff %.1f,%.1f\n",
			nIndex, (LPCTSTR)pv->GetName(), (LPCTSTR)pvn->GetName(),
			(LPCTSTR)this->m_szFaceName, nFallsWithin,
			nX1, nY1,
			pt.x, pt.y,
			xdiff, ydiff,
			nX2, nY2,
			x2diff, y2diff );

		CDbg::Out( 1, "\torient %f (from %f) is %f <= n <= %f?\n",
			dTestPointOrientation, dOriginal,
			dLegOrientation, dPrevLegOrientation );

		CDbg::Out( 1, "\ttest orient %f - is\n", dEndTestPointOrientation );

		// Nearest edge will have the minimum sum of diff lengths

		// Test for inclusion.  Handle the case where dLegOrientation ~= 350 and
		// dPrevLegOrientation ~= 20
		if (dLegOrientation <= dTestPointOrientation &&
			dTestPointOrientation <= dPrevLegOrientation)
		{
			nFallsWithin++;
		}
		else if (dPrevLegOrientation < dLegOrientation &&
				(dTestPointOrientation >= dLegOrientation ||
				 dTestPointOrientation <= dPrevLegOrientation))
		{
			nFallsWithin++;
		}
		//else break; // Early out on first miss

		// Change origin
		TestStart = End;
	} // For all vertices

	// Are we contained?
	CDbg::Out( 1, "Total for %s: %d containments\n",
		(LPCTSTR)m_szFaceName, nFallsWithin );
	if (nFallsWithin == nSize)
	{
		return this;
	}

	// Clear edge result
	pEdge = NULL;

	for (nOffset = 0; nOffset < nSize; nOffset++)
	{
		int nIndex = (nStartVertex + nOffset) % nSize;
		pv = av[nIndex];
		// Check for congruent vertex in a connected face
		// There are potentially four candidates
		CVertex* pvc;
		pvc = GetLeastCongruentVertex( pv, pv->m_pSides[0]->m_pOutwardConnectedEdge );
		// Note that we must also guard against looping
		if (pvc && pvc->m_pFace != pOrigin)
		{
			// Starting orientation is current + pv inside angle / 2 + pvc inside angle / 2
			double dNewOrientation = dOrientation + pv->m_dInsideAngle / 2 + pvc->m_dInsideAngle / 2;
			pReturn = pvc->m_pFace->FindContainingFace( pt, this, pvc->GetOrder(), Start,
					pEdge, dNewOrientation, dScale );
			if (pReturn)
			{
				return pReturn;
			}
		} // Found one
		pvc = GetLeastCongruentVertex( pv, pv->m_pSides[1]->m_pOutwardConnectedEdge );
		if (pvc && pvc->m_pFace != pOrigin)
		{
			// Starting orientation is current - pv inside angle / 2 - pvc inside angle / 2
			double dNewOrientation = dOrientation - pv->m_dInsideAngle / 2 - pvc->m_dInsideAngle / 2;
			pReturn = pvc->m_pFace->FindContainingFace( pt, this, pvc->GetOrder(), Start,
					pEdge, dNewOrientation, dScale );
			if (pReturn)
			{
				return pReturn;
			}
		} // Found another

		// Now move start to next vertex and draw leg
		// We always go clockwise, and orientations also rotate clockwise, which means
		// adding...
		// Given an orientation, the leg is always orientation - this vertex's inside angle / 2
		// The next orientation will be that leg + 180 - the next vertex's inside angle / 2
		CVertex* pvn = av[(nIndex+1)%nSize];
		double dLegOrientation = fmod( dOrientation - pv->m_dInsideAngle / 2 + 360.0, 360.0 );
		double dCurrentOrientation = dOrientation;
		dOrientation = fmod( dLegOrientation + 180 - pvn->m_dInsideAngle / 2 + 360.0, 360.0 );
		double dLegLength = pv->m_pSides[1]->m_dLength * dScale;


		// Move along leg
		CLogPoint End( Start );
		VectorMove( End.m_dX, End.m_dY, dLegOrientation, dLegLength );

		int nX1, nY1, nX2, nY2;
		nX1 = (int)floor( Start.m_dX + 0.5 );
		nY1 = (int)floor( Start.m_dY + 0.5 );
		nX2 = (int)floor( End.m_dX + 0.5 );
		nY2 = (int)floor( End.m_dY + 0.5 );

		// Change origin
		Start = End;
	} // for all vertices

	return pReturn;
} // CFace::FindContainingFace()

// Re-initialize GetOrder() for vertices
void
CFace::SetVertexOrder()
{
	CVertex* pv;
	int nOrder;
	pv = GetFirstVertex();
	int nSize = m_pVertices.GetCount();
	for (nOrder = 0; pv != NULL && nOrder < nSize; nOrder++)
	{
		pv->m_nOrder = nOrder;
		pv = pv->m_pSides[1]->m_pEndPoints[1];
	} // for all vertices
} // CFace::SetVertexOrder()

// Find congruent vertex IFF its ordinal is the antecedent of the two vertices
// on the specified edge.  Returns NULL if not found.  pOtherEdge==NULL
// guarantees a NULL return value.
CVertex*
CFace::GetLeastCongruentVertex( CVertex* pReference, CEdge* pOtherEdge ) const
{
	if (!pOtherEdge)
	{
		return NULL;
	} // no other edge
	if (CMultiVertex::AreCongruent( pReference, pOtherEdge->m_pEndPoints[0] ))
	{
		// Compare ordinals
		if (pOtherEdge->m_pFace->IsAntecedent( pOtherEdge->m_pEndPoints[0], pOtherEdge->m_pEndPoints[1] ))
		{
			return pOtherEdge->m_pEndPoints[0];
		} // Found it
	} // Congruent with 0
	else if (CMultiVertex::AreCongruent( pReference, pOtherEdge->m_pEndPoints[1] ))
	{
		if (pOtherEdge->m_pFace->IsAntecedent( pOtherEdge->m_pEndPoints[1], pOtherEdge->m_pEndPoints[0] ))
		{
			return pOtherEdge->m_pEndPoints[1];
		} // found
	} // Congruent with 1
	// Did not find a congruent vertex
	return NULL;
} // CFace::GetLeastCongruentVertex()

// Return true if vertex a is the antecedent of vertex b.
// Normally ordinal(a) < ordinal(b) unless ((ordinal(a)+1)%numvertices) == ordinal(b),
// which also proves true
bool
CFace::IsAntecedent( CVertex* pA, CVertex* pB ) const
{
	// First check for exception where ordinal(b) > ordinal(a) but
	// (ordinal(b)+1) % numvertices == ordinal(a)
	if (pB->GetOrder() > pA->GetOrder() &&
		(pB->GetOrder() + 1) % m_pVertices.GetCount() == pA->GetOrder())
	{
		return true;
	}

	// Now check for obvious case where ordinal(a) < ordinal(b)
	return (pA->GetOrder() < pB->GetOrder());

} // CFace::IsAntecedent()

// Helper function for render: vector move.
// vector 0-360 in degrees
void
CFace::VectorMove( double& dX, double& dY, double dVectorAngle, double dDistance )
{
	double XDelta, YDelta;
	//static double adSlopeXDirections[4] = { 1.0, 1.0, -1.0, -1.0 };
	//static double adSlopeYDirections[4] = { -1.0, 1.0, 1.0, -1.0 };
	////static double adSlopeYDirections[4] = { 1.0, -1.0, -1.0, 1.0 };

	XDelta = /*adSlopeXDirections[nQuadrant] * */ sin( DegToRadans( dVectorAngle ) ) * dDistance;
	YDelta = /*adSlopeYDirections[nQuadrant] * */ cos( DegToRadans( dVectorAngle ) ) * dDistance;
	dX += XDelta;
	dY -= YDelta;
} // CFace::VectorMove()

// Write text at specified coordinates in rotated font
// with bounding box of specified width and height.
void
CFace::RotatedTextOut( CDC* pDC, double dX, double dY, double dOrientation,
	int nWidth, int nHeight, CString& szText, bool bScaleToFit /*=true*/ )
{
	CSize s = pDC->GetTextExtent( szText );
	CFont f;

	// Get height of current font
	TEXTMETRIC tm;
	pDC->GetTextMetrics( &tm );
	int nTargetFontHeight = tm.tmHeight;

	// Scale up or down?
	if (bScaleToFit)
	{
		if (s.cy < nHeight - 2)
		{
			// When expanding text, leave a little room
			nHeight = nHeight + 2;
			nTargetFontHeight = (int)floor( 0.5 + nTargetFontHeight * nHeight / (double)s.cy );
			if (nTargetFontHeight < tm.tmHeight)
			{
				nTargetFontHeight = tm.tmHeight;
			} // Truncation resulted in a smaller font
		} // Scale up
		else if (s.cy > nHeight)
		{
			// When reducing text, squeeze to an exact fit
			nTargetFontHeight = (int)floor( 0.5 + nTargetFontHeight * nHeight / (double)s.cy );
			// Select minimum size
			if (nTargetFontHeight < 4)
			{
				nTargetFontHeight = 4;
			}
		} // Scale down
	} // Scaling

	// Get face of current font
	CFont* pFont = pDC->GetCurrentFont();
	LOGFONT lf;
	pFont->GetLogFont( &lf );
	// Create font
	lf.lfHeight = nTargetFontHeight;
	lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
	// Convert to degrees / 10, counterclockwise
	lf.lfEscapement = (int)((360.0 - fmod( dOrientation, 360.0 )) * 10.0);
	lf.lfWidth = 0;
	f.CreateFontIndirect( &lf );

	// Select it
	CFont* pOldFont = pDC->SelectObject( &f );

	// Change alignment to top center
	UINT uAlign = pDC->GetTextAlign();
	pDC->SetTextAlign( TA_CENTER | TA_TOP );

	// Draw text
	pDC->TextOut( (int)floor( dX + 0.5 ), (int)floor( dY + 0.5 ), szText );

	// Restore text alignment
	pDC->SetTextAlign( uAlign );

	// Restore old font
	pDC->SelectObject( pOldFont );

	// Destroy resources
	f.DeleteObject();

} // CFace::RotatedTextOut()

void CFace::ClearAllEdges()
{
	POSITION pos = m_pEdges.GetStartPosition();
	CString szName;
	CEdge* pe;
	while (pos != NULL)
	{
		m_pEdges.GetNextAssoc( pos, szName, pe );
		delete pe;
		//m_pEdges.SetAt( szName, NULL );
	}
	m_pEdges.RemoveAll();

}

void CFace::ClearAllVertices()
{
	CVertex* pv;
	POSITION pos = m_pVertices.GetStartPosition();
	CString szName;
	while (pos != NULL)
	{
		m_pVertices.GetNextAssoc( pos, szName, pv );
		delete pv;
		//m_pVertices.SetAt( szName, NULL );
	}
	m_pVertices.RemoveAll();

}

int CFace::CalcInsideAngles(bool bForceCalc /*=false*/)
{
	if (m_pEdges.GetCount() != 3)
	{
		// If inside angles are specified for nedges-3 vertices, calculate remainder
		// else calculate all as the complement of 360 / nedges
		int numAnglesSpecified = 0;
		POSITION posEdge;
		CEdge* pEdge;
		CString szName;
		double dAngleSum = 0.0;
		for (posEdge = m_pEdges.GetStartPosition(); posEdge != NULL; )
		{
			m_pEdges.GetNextAssoc( posEdge, szName, pEdge );
			if (pEdge->m_pEndPoints[0]->m_dInsideAngle > 0.0)
			{
				numAnglesSpecified++;
			}
			dAngleSum += pEdge->m_pEndPoints[0]->m_dInsideAngle;
		}
		// FIXME calculate missing angles rather than requiring they all be specified
		if (numAnglesSpecified == m_pEdges.GetCount() && dAngleSum + 0.0001 >= 360.0)
		{
			;
		}
		else
		{
			// Calculate inside angles as the complement of 360 / nedges
			double dInside = 360.0 / m_pEdges.GetCount();
			dInside = 180.0 - dInside;
			for (posEdge = m_pEdges.GetStartPosition(); posEdge != NULL; )
			{
				m_pEdges.GetNextAssoc( posEdge, szName, pEdge );
				pEdge->m_pEndPoints[0]->m_dInsideAngle = dInside;
			}
		}
		return 0;
	} // Only supported for triangular faces

	CEdge* peTriangle[3];
	POSITION pos;
	CString szEdgeName;
	int nIndex;
	int nCountNonzero = 0;
	for (nIndex = 0, pos = m_pEdges.GetStartPosition(); pos != NULL && nIndex < 3; nIndex++)
	{
		m_pEdges.GetNextAssoc( pos, szEdgeName, peTriangle[nIndex] );
		if (!bForceCalc && peTriangle[nIndex]->m_pEndPoints[1]->m_dInsideAngle != 0.0)
		{
			nCountNonzero++;
		}
	}

	// If not forcing and all three are calculated, nothing to do
	if (!bForceCalc && nCountNonzero == 3)
	{
		return 0;
	} // All are calculated

	// Calculate vertex angles using law of cosines
	// Given known lengths for all three sides,
	// derive using law of cosines.
	// Given a triangle with sides a, b and c,
	// and an angle A facing side a, B facing b,
	// and C facing c, we know that
	// c^2 = a^2 + b^2 - 2ab * cos( C )
	// See http://www.alltel.net/~okrebs/page94.html

	// We can also use the law of sines once we have
	// one angle:
	// sin(A) / a == sin(B) / b == sin(C) / c
	// See http://www.alltel.net/~okrebs/page93.html

	CEdge* pa = peTriangle[0];
	CEdge* pb = peTriangle[1];
	CEdge* pc = peTriangle[2];

	CVertex* pA = pb->m_pEndPoints[1];
	CVertex* pB = pc->m_pEndPoints[1];
	CVertex* pC = pa->m_pEndPoints[1];

	// Solve left side
	double dLeft = pc->m_dLength * pc->m_dLength;

	// Solve known values on right
	double dRight1 = pa->m_dLength * pa->m_dLength
		+ pb->m_dLength * pb->m_dLength;
	double dRight2 = 2 * pa->m_dLength * pb->m_dLength;

	// Now we have
	// dLeft == dRight1 - dRight2 * cos( C )
	// or
	// -dLeft == -dRight1 + dRight2 * cos( C )
	// or
	// dRight1 - dLeft == dRight2 * cos( C )
	// or
	// (dRight1 - dLeft) / dRight2 == cos( C )

	// Solve for coefficient of right side, which is cos(C)
	double cInside = acos( (dRight1 - dLeft) / dRight2 );
	pC->m_dInsideAngle = RadansToDeg( cInside );

	// Now get ratio to apply law of sines
	double dRatio = sin( cInside ) / pc->m_dLength;

	// Solve other angles
	// sin(A) / a == dRatio, therefore
	// sin(A) == a * dRatio, therefore
	// A = asin( a * dRatio )
	pA->m_dInsideAngle = RadansToDeg( asin( pa->m_dLength * dRatio ) );
	pB->m_dInsideAngle = RadansToDeg( asin( pb->m_dLength * dRatio ) );

	// Success
	return 0;
}

// Get the base edge for this face (i.e. the base against which orientation
// is calculated). Currently this is designated when the shape is created.
CEdge* CFace::GetBaseEdge() const 
{ 
	if (!m_szBaseEdgeName.IsEmpty())
	{
		CEdge* pEdge = this->GetEdge( m_szBaseEdgeName );
		if (pEdge != NULL)
		{
			return pEdge;
		}
	}
	// Default - get lexically last edge
	return GetLastEdge(); 
}

// Get lexically last edge in face
CEdge* CFace::GetLastEdge() const
{
	// Normally we'd name edges as follows, starting clockwise
	// from the prime vertex at lower left:
	//     .      ^
	//    / \     |
	//  a/   \b   | face
	// /_______\  | orientation
	//     c      |
	// Note that face orientation should be defined
	// as perpendicular to c. FIXME

	CEdge *pEdge, *pLastEdge = NULL;
	POSITION pos;
	CString szEdgeName, szLastEdgeName;
	bool bFirstPass = true;
	for (pos = m_pEdges.GetStartPosition(); pos != NULL; )
	{
		m_pEdges.GetNextAssoc( pos, szEdgeName, pEdge );
		if (bFirstPass || szEdgeName > szLastEdgeName)
		{
			szLastEdgeName = szEdgeName;
			pLastEdge = pEdge;
		}
		bFirstPass = false;
	}

	ASSERT( pLastEdge != NULL );

	return pLastEdge;

} // CFace::GetLastEdge()

// Get lexically first edge in face
CEdge* CFace::GetFirstEdge() const
{
	CEdge *pEdge, *pLeastEdge = NULL;
	POSITION pos;
	CString szEdgeName, szLeastEdgeName;
	bool bFirstPass = true;
	for (pos = m_pEdges.GetStartPosition(); pos != NULL; )
	{
		m_pEdges.GetNextAssoc( pos, szEdgeName, pEdge );
		if (bFirstPass || szEdgeName < szLeastEdgeName)
		{
			szLeastEdgeName = szEdgeName;
			pLeastEdge = pEdge;
		}
		bFirstPass = false;
	}

	ASSERT( pLeastEdge != NULL );

	return pLeastEdge;

} // CFace::GetFirstEdge()

// Get lexically first vertex in face
CVertex* CFace::GetFirstVertex() const
{
	CEdge* pEdge = GetFirstEdge();
	ASSERT( pEdge );
	CVertex* pVertex = pEdge->m_pEndPoints[0];
	ASSERT( pVertex );
	return pVertex;
}

// Get edge by name
CEdge* CFace::GetEdge(LPCTSTR lpEdgeName) const
{
	CEdge* pEdge;
	if (!m_pEdges.Lookup( lpEdgeName, pEdge ))
	{
		pEdge = NULL;
	}
	return pEdge;
}

// Get vertex by name
CVertex* CFace::GetVertex(LPCTSTR lpVertexName)
{
	CString szName;
	CVertex* pVertex;
	if (!m_pVertices.Lookup( szName, pVertex ))
	{
		pVertex = NULL;
	}
	return pVertex;
}

// Count outward joins
int CFace::GetJoinCount()
{
	int nReturn = 0;
	POSITION pos;
	CEdge* pEdge;
	CString szEdgeName;
	for (pos = m_pEdges.GetStartPosition(); pos != NULL; )
	{
		m_pEdges.GetNextAssoc( pos, szEdgeName, pEdge );
		if (pEdge->m_pOutwardConnectedEdge != NULL)
		{
			nReturn++;
		}
	}
	return nReturn;
}

// Get min and max edge lengths and return number of edges
int CFace::GetMinMaxEdge(double &dMin, double &dMax)
{
	POSITION pos;
	CEdge* pEdge;
	CString szEdgeName;
	dMax = -999999999999;
	dMin =  999999999999;
	int nCount = 0;
	for (pos = m_pEdges.GetStartPosition(); pos != NULL; )
	{
		m_pEdges.GetNextAssoc( pos, szEdgeName, pEdge );
		double d = pEdge->m_dLength;
		if (d < dMin)
		{
			dMin = d;
		}
		if (d > dMax)
		{
			dMax = d;
		}
		nCount++;
	}
	return nCount;
}

// Get body after evaluating $ escapes
CString
CFace::GetBody( CString& szDefault ) const
{
	CString sz( m_strBody );
	if (sz.IsEmpty())
	{
		sz = szDefault;
		// ^\r\n is used as a placeholder for empty lines
		sz.Replace( "^\r\n", "^" );
		sz.Replace( "^", "^\r\n" );
	}
	sz.Replace( "$face", m_szFaceName );
	return sz;
} // CFace::GetBody()

// Recursively get joined faces
void
CFace::GetJoinedFaces( CMap<CFace const*,CFace const*,int,int>& mapFaceCount ) const
{
	POSITION pos;
	CFace* pFace;
	CString szEdgeName;
	CEdge* pEdge;
	int nCount;
	for (pos = this->m_pEdges.GetStartPosition(); pos != NULL; )
	{
		m_pEdges.GetNextAssoc( pos, szEdgeName, pEdge );
		if (pEdge->m_pOutwardConnectedEdge == NULL)
		{
			continue;
		}
		pFace = pEdge->m_pOutwardConnectedEdge->m_pFace;
		if (pFace == NULL)
		{
			continue;
		} // Not connected
		// Find in list
		if (mapFaceCount.Lookup( pFace, nCount ))
		{
			// Increment count
			nCount++;
			mapFaceCount.SetAt( pFace, nCount );
			continue;
		}
		// Recurse the first time
		nCount = 1;
		mapFaceCount.SetAt( pFace, nCount );
		pFace->GetJoinedFaces( mapFaceCount );
	} // for all edges

} // CFace::GetJoinedFaces()

// Construct a recursive join list for this face
CString
CFace::GetTextJoinList() const
{
	// This is text that could follow "face xxx joins "
	// First build map of faces
	CMap<CFace const*,CFace const*,int,int> mapFaceCount;
	// Set ourselves with a count of 1 so we do not end up in a loop
	mapFaceCount.SetAt( this, 1 );
	GetJoinedFaces( mapFaceCount );
	// Now format return list
	POSITION pos;
	CString szReturn;
	CString szFace;
	int nFaceCount;
	int nTotalCount = 0;
	CFace const* pFace;
	for (pos = mapFaceCount.GetStartPosition(); pos != NULL; )
	{
		mapFaceCount.GetNextAssoc( pos, pFace, nFaceCount );
		if (pFace == this) continue;
		szFace.Format( "%s%s", nTotalCount>0 ? ", " : "", (LPCTSTR)pFace->m_szFaceName );
		szReturn += szFace;
		if (nFaceCount>1)
		{
			szFace.Format( "(%d)", nFaceCount );
			szReturn += szFace;
		}
		nTotalCount++;
	} //
	
	return szReturn;
} // CFace::GetTextJoinList()

// Get list of groups
int
CFace::GetGroups( CArray<CString,LPCTSTR>& aGroups ) const
{
	aGroups.RemoveAll();
	if (m_szGroupNames.IsEmpty())
	{
		return 0;
	} // Nothing
	CString sz;
	LPSTR lp = sz.GetBuffer( m_szGroupNames.GetLength() + 1 );
	lstrcpy( lp, m_szGroupNames );
	for (lp = strtok( lp, ":" ); lp != NULL; lp = strtok( NULL, ":" ))
	{
		aGroups.Add( lp );
	} // for all group names
	sz.ReleaseBuffer();
	return aGroups.GetSize();
} // CFace::GetGroups()

// Is face a part of named group?
bool
CFace::IsPartOfGroup( LPCTSTR lpGroupName ) const
{
	if (m_szGroupNames.IsEmpty())
	{
		return false;
	}
	CString sz;
	sz.Format( ":%s:", lpGroupName );
	return (m_szGroupNames.Find( sz ) >= 0);
} // CFace::IsPartOfGroup()

// Add to group
void
CFace::AddToGroup( LPCTSTR lpGroupName )
{
	if (IsPartOfGroup( lpGroupName ))
	{
		CDbg::Out( "Warning: tried to add group %s to face %s twice, already %s\n",
			lpGroupName, (LPCTSTR)this->m_szFaceName, (LPCTSTR)m_szGroupNames );
		return;
	}
	if (m_szGroupNames.IsEmpty())
	{
		m_szGroupNames = ':';
	}
	m_szGroupNames += lpGroupName;
	m_szGroupNames += ':';
} // CFace::AddToGroup()

// Do depth-first recursion to clear concave attribute from all outward edges
void
CFace::ClearOutwardEdgeConcavity( CFace* pRoot, CFace* pFrom )
{
	// Go through all edges
	POSITION pos;
	CEdge* pEdge;
	CString szEdgeName;
	for (pos = m_pEdges.GetStartPosition(); pos != NULL; )
	{
		m_pEdges.GetNextAssoc( pos, szEdgeName, pEdge );
		if (pEdge->m_pOutwardConnectedEdge == NULL)
		{
			pEdge->m_bConcave = false;
		}
		else if (pEdge->m_pOutwardConnectedEdge->m_pFace != pRoot
				&& pEdge->m_pOutwardConnectedEdge->m_pFace != pFrom)
		{
			pEdge->m_pOutwardConnectedEdge->m_pFace->ClearOutwardEdgeConcavity( pRoot, this );
		}
	}
} // CFace::ClearOutwardEdgeConcavity()

// Get page number if available
int CFace::GetPageNumber() const
{
	if (m_PageGroup != NULL)
	{
		CDbg::Out( "CFace::GetPageNumber(this=%s:%08x) - m_PageGroup(%08x) page number %d\n",
			(LPCTSTR)m_szFaceName, (DWORD)this, (DWORD)m_PageGroup, m_PageGroup->GetPageNumber() );
		return m_PageGroup->GetPageNumber();
	}
	else
		CDbg::Out( "CFace::GetPageNumber(this=%s:%08x) - m_PageGroup==NULL\n", (LPCTSTR)this->m_szFaceName, (DWORD)this );
	// Default
	return -1;
}

CPageGroup* CFace::SetPageGroup(CPageGroup* pGroup, BOOL bRecursive /*= TRUE*/ )
{
	CPageGroup* pOld = m_PageGroup;
	m_PageGroup = pGroup;
	CDbg::Out( "CFace::SetPageGroup(%08x) for face %s (this=%08x) (prev=%08x)\n",
		(DWORD)pGroup, (LPCTSTR)this->m_szFaceName, (DWORD)this, (DWORD)pOld );
	// Go clockwise through connected faces
	//if (bRecursive)
	//{
	//	ConnectPageGroups( this, pGroup, TRUE );
	//}
	return pOld;
}

// Recursively connect page groups until we come back to ourself. 
// If we go clockwise around edges we should avoid endless recursion.
void CFace::ConnectPageGroups( CFace* pEnd, CPageGroup* pGroup, BOOL bClockwise )
{
	static int RDepth = 0;
	RDepth++;
	if (RDepth > 20)
	{
		RDepth--;
		return;
	}
	// Start with base edge
	CEdge* pStartEdge = this->GetBaseEdge();
	CEdge* pEdge;
	if (bClockwise)
		pEdge = pStartEdge->GetClockwiseNext();
	else
		pEdge = pStartEdge->GetClockwisePrev();
	for ( ; pEdge->m_pOutwardConnectedEdge == NULL; )
	{
		if (pEdge == pStartEdge)
		{
			RDepth--;
			return;
		}
		if (bClockwise)
			pEdge = pEdge->GetClockwiseNext();
		else
			pEdge = pEdge->GetClockwisePrev();
	}
	if (pEdge->m_pOutwardConnectedEdge == NULL)
	{
		RDepth--;
		return;
	}
	CFace* pFace = pEdge->m_pOutwardConnectedEdge->m_pFace;
	if (pFace == pEnd)
	{
		RDepth--;
		return;
	}
	pFace->SetPageGroup( pGroup, FALSE );
	// Alternate clockwise and counter-clockwise
	pFace->ConnectPageGroups( pEnd, pGroup, bClockwise );
	RDepth--;
}

// Recursively build a map of connected faces
void CFace::BuildConnectedFaceMap( CMap<CFace*,CFace*,int,int>& map, BOOL bForceRecursion /*= TRUE*/ )
{
	int n;
	if (!map.Lookup( this, n ))
	{
		map.SetAt( this, 1 );
	}
	else if (!bForceRecursion)
	{
		return;
	}
	// Process connected faces without forcing recursion
	CEdge* pEdge = this->GetBaseEdge();
	CEdge* pLastEdge = pEdge->GetClockwisePrev();
	for ( ; ; pEdge = pEdge->GetClockwiseNext())
	{
		if (pEdge->m_pOutwardConnectedEdge != NULL)
			pEdge->m_pOutwardConnectedEdge->m_pFace->BuildConnectedFaceMap( map, FALSE );
		if (pEdge == pLastEdge) break;
	}
}

// Set layout division. "" == default. Returns index of new division
int CFace::SetLayoutDivision( LPCTSTR lpNew )
{
	m_szLayoutDivision = lpNew;
	// Add if not existing
	return m_pOwner->GetDivisionIndex( lpNew, true );
}

CMap<char,char,CString,CString> CFace::m_mapPolyNameExtension;

// Increment a character in a string recursively. Grow by 1 and reset to A's
void IncrementCharAt( LPSTR lp, size_t index )
{
	// If we've reached the end of possible choices, increment previous
	if (lp[index] == '9')
	{
		if (index > 0)
		{
			IncrementCharAt( lp, index-1 );
		}
		else
		{
			size_t NewLength = strlen( lp ) + 1;
			strncpy( lp, "AAAAAAAAAA", NewLength );
			lp[NewLength] = '\0';
			return;
		}
	}
	else if (lp[index] == 'Z')
	{
		lp[index] = '0';
	}
	else 
	{
		lp[index]++;
	}
}

// Create unique name using specified polygon identifier
CString CFace::UniquePolyName( char cType )
{
	CString sz;
	if (!m_mapPolyNameExtension.Lookup(cType,sz))
	{
		m_mapPolyNameExtension.SetAt(cType,"A");
	}
	sz.Format("%c%s", cType, (LPCTSTR)m_mapPolyNameExtension[cType] );
	// Increment type
	CString szType = m_mapPolyNameExtension[cType];
	// Allocate room for growth plus trailing null
	LPSTR lp = szType.GetBuffer( szType.GetLength() + 2 );
	IncrementCharAt( lp, strlen( lp ) - 1 );
	szType.ReleaseBuffer();
	m_mapPolyNameExtension[cType] = szType;
	return sz;
}

// Reset unique name for specified polygon
void CFace::ResetPolyName( char cType )
{
	m_mapPolyNameExtension.SetAt(cType,"A");
}

// Free edge def
void CFace::FreeEdgeDef()
{
	if (m_pEdgeDef != NULL)
	{
		EDGEDEF* p = m_pEdgeDef;
		m_pEdgeDef = NULL;
		// Free character array entries
		for (int n = 0; n < p->nEdges; n++)
		{
			free( (void*)p->aszName[n] );
			p->aszName[n] = NULL;
		}
		// Free character array
		delete p->aszName;
		p->aszName = NULL;
		// Free length array
		delete p->adLength;
		p->adLength = NULL;
		// Free angle array if allocated
		if (p->adEndAngles != NULL)
		{
			delete p->adEndAngles;
			p->adEndAngles = NULL;
		}
		// Free object
		delete p;
	}
}

// Save edge def
void CFace::SaveEdgeDef( const char **ppszEdgeNames, double* pdLengths, int nSides, double* pdAngles /*=NULL*/ )
{
	// Free existing definition
	FreeEdgeDef();
	m_pEdgeDef = new EDGEDEF;
	m_pEdgeDef->nEdges = nSides;
	m_pEdgeDef->adLength = new double[nSides];
	m_pEdgeDef->adEndAngles = new double[nSides];
	m_pEdgeDef->aszName = (LPCTSTR*)new char *[nSides];
	for (int n = 0; n < nSides; n++)
	{
		m_pEdgeDef->aszName[n] = strdup( ppszEdgeNames[n] );
		m_pEdgeDef->adLength[n] = pdLengths[n];
		m_pEdgeDef->adEndAngles[n] = 0.0;
		if (pdAngles != NULL)
		{
			m_pEdgeDef->adEndAngles[n] = pdAngles[n];
		}
	}
}

// Get edge definition set used in CreatePolygon() or create one
EDGEDEF * CFace::GetSafeEdgeDef()
{
	// Use existing edge def, if present
	if (GetEdgeDef() != NULL)
	{
		return GetEdgeDef();
	}
	// Create edge definition set
	m_pEdgeDef = new EDGEDEF;
	m_pEdgeDef->nEdges = this->m_pEdges.GetCount();
	m_pEdgeDef->adLength = new double[m_pEdgeDef->nEdges];
	m_pEdgeDef->aszName = (LPCTSTR*)new char *[m_pEdgeDef->nEdges];
	m_pEdgeDef->adEndAngles = new double[m_pEdgeDef->nEdges];
	// Start with lexically first edge and go clockwise
	int n;
	CEdge* pEdge = this->GetFirstEdge();
	for (n = 0; n < m_pEdgeDef->nEdges; n++)
	{
		m_pEdgeDef->adLength[n] = pEdge->m_dLength;
		m_pEdgeDef->adEndAngles[n] = pEdge->m_pEndPoints[1]->m_dInsideAngle;
		m_pEdgeDef->aszName[n] = strdup( pEdge->GetName() );
		pEdge = pEdge->GetClockwiseNext();
	}
	return GetEdgeDef();
}

// Check for polygon intersections in currently joined set of faces.
// Return number of collisions found. GetVertexCoordinates() should have been called first to set up m_loc
int CFace::CollisionCount( CArray<CFace*,CFace*>& targetGroup )
{
	// If < 2 faces, no need to check
	CMap<CFace const*,CFace const*,int,int> mapFaceCount;
	this->GetJoinedFaces( mapFaceCount );
	if (targetGroup.GetSize() < 1)
	{
		CDbg::Out( "Target group has < 1 faces\n" );
	}
	// this (the CFace pointer) is our candidate. Check it and all joined faces against all faces in the
	// target group.
	int collisionCount = 0;
	POSITION pos;
	CFace const* pFace;
	CString faceList;
	int count;
	int n;
	faceList.Format( "CollisionCount(%s)", (LPCTSTR)this->m_szFaceName );
	// Add all faces to target group. Do a brute-force product join
	// and select combinations where the left side is lexically less than
	// the right side face name.
	for (pos = mapFaceCount.GetStartPosition(); pos != NULL; )
	{
		mapFaceCount.GetNextAssoc( pos, pFace, count );
		targetGroup.Add( (CFace*)pFace );
	}
	// Do the cross-product join
	for (pos = mapFaceCount.GetStartPosition(); pos != NULL; )
	{
		mapFaceCount.GetNextAssoc( pos, pFace, count );
		// Go through the target group
		for (n = 0; n < targetGroup.GetSize(); n++)
		{
			// Skip equivalencies
			if (pFace == targetGroup[n])
			{
				continue;
			}
			// Skip combinations where left side name is lexically greater
			if (pFace->m_szFaceName.Compare( targetGroup[n]->m_szFaceName ) > 0)
			{
				continue;
			}
			//CDbg::m_Level--; DELETEME
			if (pFace->CollidesWith( *(targetGroup[n]) ))
			{
				faceList += ',';
				faceList += pFace->m_szFaceName;
				faceList += '!';
				faceList += targetGroup[n]->m_szFaceName;
				collisionCount++;
			}
			//CDbg::m_Level++; DELETEME
		}
	}
	CDbg::Out( "%d collisions - %s\n", collisionCount, (LPCTSTR)faceList );
	return collisionCount;
}

// Helper functions for CollidesWith().
// Return cartesian quadrant as origin:0
static int Quadrant( CLogPoint* p )
{
	// Note that our coordinate system is screwy. Normally x positive goes to the
	// right and y positive goes up, therefore the quadrants go clockwise.
	// Our quadrants and vector angles go counterclockwise, because
	// x positive goes right and y positive goes down. Therefore
	// 0 degrees is pointing straight down, 90 degrees is right, 180 is
	// straight up, and 270 is to the left.
	if (p->m_dX >= 0)
	{
		return p->m_dY < 0 ? 1 : 0;
	}
	return p->m_dY < 0 ? 2 : 3;
}

// Return vector angle in degrees for a point
static long double VectorAngle( CLogPoint* p )
{
	long double d;
	d = atan2( p->m_dX, p->m_dY );
	d = CFace::RadansToDeg( d );
	d += 180.0;
	return d;
}

// true if point is included in inside angle of vertex
static bool PointIncluded( CVertex* a, CLogPoint* p )
{
	// Vertices are arranged clockwise. Thus we know the following:
	// - the vector angle to successor(a) should be less than the vector angle of predecessor(a)
	// - if the vector angle of successor(a) is not less than the vector angle of predecessor(a), 
	//		add 360 to the vector angle of predecessor(a)
	// - if the vector angle to a test point b[n] is less than the vector angle of successor(a),
	//		add 360 to it
	// - if the vector angle of the test point b[n] falls between the vector angles for a's successor
	//		and predecessor, count it as included
	// - We can always assume the orientation of successor(a) is 0 degrees

	// Point is not included if it is congruent to the vertex
	if (abs( p->m_dX - a->m_loc->m_dX ) < 0.0000001 && abs( p->m_dY - a->m_loc->m_dY ) < 0.0000001)
	{
		return false;
	}

	// We need to find the vector angles based on coordinates.
	// Remember the counterclockwise vector system.

	// We only need to find the vector angle for pred(a), subtract from
	// the vector angle for p, and test against the inside angle of a.
	// Translate relative to vertex
	CLogPoint succA( *a->m_pSides[1]->m_pEndPoints[1]->m_loc );
	CLogPoint predA( *a->m_pSides[0]->m_pEndPoints[0]->m_loc );
	CLogPoint prel( *p );
	succA -= *(a->m_loc);
	predA -= *(a->m_loc);
	prel -= *(a->m_loc);
	long double succAVector = VectorAngle( &succA );
	long double predAVector = VectorAngle( &predA );
	long double pVector = VectorAngle( &prel );
	pVector -= predAVector;
	if (pVector < 0)
	{
		pVector += 360.0;
	}
	// Interior inclusion test is satisfied iff point is not on a boundary
	if (pVector > 0.00001 && pVector < a->m_dInsideAngle - 0.00001)
	{
		CDbg::Out( "        Col %s:%s 0<%.3f<%.3f (%.3f,%.3f) suc%s@%.5f,%.5f vtx%s@%.5f,%.5f tst@%.5f,%.5f prd%s@%.5f,%.5f\n", 
			(LPCTSTR)a->m_pFace->m_szFaceName, (LPCTSTR)a->GetName(), 
			pVector, a->m_dInsideAngle,
			succAVector, predAVector,
			(LPCTSTR)a->m_pSides[1]->m_pEndPoints[1]->m_szVertexName,
				a->m_pSides[1]->m_pEndPoints[1]->m_loc->m_dX,
				a->m_pSides[1]->m_pEndPoints[1]->m_loc->m_dY,
			(LPCTSTR)a->m_szVertexName,
				a->m_loc->m_dX,
				a->m_loc->m_dY,
			p->m_dX,
			p->m_dY,
			(LPCTSTR)a->m_pSides[0]->m_pEndPoints[0]->m_szVertexName,
				a->m_pSides[0]->m_pEndPoints[0]->m_loc->m_dX,
				a->m_pSides[0]->m_pEndPoints[0]->m_loc->m_dY);
		return true;
	}
	return false;
}

// VIC is Vertex Inclusion Count, i.e. the number of vertices of face B which lie within the interior
// angle of a vertex of face A.
static int VIC( CVertex* a, CArray<CVertex*,CVertex*>& b )
{
	int n;
	int includeCount = 0;
	for (n = 0; n < b.GetSize(); n++)
	{
		if (PointIncluded( a, b[n]->m_loc ))
		{
			CDbg::Out( "      PointInc(%s,%s[%d])\n", 
				(LPCTSTR)a->m_szVertexName, (LPCTSTR)b[n]->m_szVertexName, n );
			includeCount++;
		}
	}
	return includeCount;
}

// VICNZ is the number of VIC values for vertices of face A where the VIC for an individual vertex, with respect
// to vertices of face B, is nonzero.
static int VICNZ( CArray<CVertex*,CVertex*>& a, CArray<CVertex*,CVertex*>& b )
{
	int n;
	int nzCount = 0;
	int count;
	for (n = 0; n < a.GetSize(); n++)
	{
		count = VIC( a[n], b );
		if (count > 0)
		{
			CDbg::Out( "   VIC(%s:%s,b) = %d\n", (LPCTSTR)a[n]->m_pFace->m_szFaceName, (LPCTSTR)a[n]->m_szVertexName, count );
			nzCount++;
		}
	}
	return nzCount;
}

// Check for this face colliding with another. We check for any intersection of the interiors
// of the two polygons. m_loc must be set in vertices (via GetVertexCoordinates)
bool CFace::CollidesWith( CFace const& dest ) const
{
	// Do we have required coordinates for all vertices?
	int n;
	CVertex* v;
	CEdge* pEdge;
	CEdge* pEdgeFirst;
	int sourceSides = 0, destSides = 0;
	// Check for breakpoints
	if (this->m_szFaceName=="SBs4" || m_szFaceName=="SFs1")
	{
		//_asm int 3;
	}
	// Collect vertices in arrays for easier traversal in clockwise fashion
	CArray<CVertex*,CVertex*> aSource, aDest;
	// Traverse edges clockwise
	pEdgeFirst = this->GetFirstEdge();
	for (pEdge = pEdgeFirst; pEdge != NULL && (sourceSides == 0 || pEdge != pEdgeFirst); pEdge = pEdge->GetClockwiseNext())
	{
		v = pEdge->m_pEndPoints[0];
		if (v->m_loc == NULL)
		{
			// If in doubt, shrug
			CDbg::Out( "Warning: vertex[%s] of face %s has no location set!\n", (LPCTSTR)v->GetName(), (LPCTSTR)this->m_szFaceName );
			return false;
		}
		sourceSides++;
		aSource.Add( v );
	}
	pEdgeFirst = dest.GetFirstEdge();
	for (pEdge = pEdgeFirst; pEdge != NULL && (destSides == 0 || pEdge != pEdgeFirst); pEdge = pEdge->GetClockwiseNext())
	{
		v = pEdge->m_pEndPoints[0];
		if (v->m_loc == NULL)
		{
			CDbg::Out( "Warning: vertex[%s] of test face %s has no location set!\n", (LPCTSTR)v->GetName(), (LPCTSTR)dest.m_szFaceName );
			return false;
		}
		destSides++;
		aDest.Add( v );
	}

	// Case 1: check for one polygon enclosed entirely in the other
	// (this is not going to happen here so we'll skip it)

	// Case 2: check for any interior collision
	int SourceDestVIC = VICNZ( aSource, aDest );
	int DestSourceVIC = VICNZ( aDest, aSource );
	if (SourceDestVIC >= sourceSides - 1 &&
		DestSourceVIC >= destSides - 1)
	{
		CDbg::Out( "Collision between %s and %s (%d,%d)\n    src coords    --    dst coords\n", 
			(LPCTSTR)this->m_szFaceName, (LPCTSTR)dest.m_szFaceName, 
			SourceDestVIC, DestSourceVIC );
		for (n = 0; ; n++)
		{
			if (n < sourceSides && n < destSides)
			{
				CDbg::Out( "  %s %7.5f,%7.5f -- %s %7.5f,%7.5f\n", 
					(LPCTSTR)aSource[n]->m_szVertexName,
					aSource[n]->m_loc->m_dX, aSource[n]->m_loc->m_dY,
					(LPCTSTR)aDest[n]->m_szVertexName,
					aDest[n]->m_loc->m_dX, aDest[n]->m_loc->m_dY );
			}
			else if (n < sourceSides)
			{
				CDbg::Out( "  %7.5f,%7.5f --\n", 
					aSource[n]->m_loc->m_dX, aSource[n]->m_loc->m_dY );
			}
			else if (n < destSides)
			{
				CDbg::Out( "             -- %7.5f,%7.5f\n", 
					aDest[n]->m_loc->m_dX, aDest[n]->m_loc->m_dY );
			}
			else
			{
				break;
			}
		}
		return true;
	}
	else
	{
		CDbg::Out( "No collision between %s and %s\n", (LPCTSTR)this->m_szFaceName, (LPCTSTR)dest.m_szFaceName );
	}

	return false;
}

// Reduce a value modulo 360
double CFace::Modulo360( double angle )
{
	if (angle < 0.0) angle += 360.0;
	if (angle >= 360.0) angle -= 360.0;
	if (angle > 360.0)
	{
		int n = floor( angle / 360 );
		angle -= (n * 360);
	}
	return angle;
}


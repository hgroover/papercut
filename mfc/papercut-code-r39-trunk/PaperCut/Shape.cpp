/*!

	@file	 Shape.cpp
	
	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: Shape.cpp 16 2006-04-15 06:39:12Z henry_groover $

  Implementation of CShape, the object representing a complete shape

*/

#include "stdafx.h"
#include "PaperCut.h"
#include "Shape.h"
#include "ShapeLayout.h"
#include "PageGroup.h"
#include "TrapezoidMidpoint.h"

#include <math.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

LPCTSTR CShape::szMIN_STELLATION_RATIO_WARNING_fsf = 
	"Warning: minimum stellation ratio of %.1f%% exceeded.\r\n"
	"The maximum possible for this %s is %.1f%%. However,\r\n"
	"using that ratio would result in a flat face rather than\r\n"
	"the desired pyramid.";

static double adAngles[] = { 0.0, 0.0, 0.0, 0.0,
							0.0, 0.0, 0.0, 0.0, 
							0.0, 0.0, 0.0, 0.0 };

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CShape::CShape( const char *pszBasicType /*= NULL*/ )
{
	m_bValid = true;
	// Set parent to point to global preferences
	Reset();
	SetParent( &(MYAPP()->m_prefs) );
	m_IsRegular = -1; // Indeterminate
	m_dLayoutDefaultPageWidth = 3;
	m_dLayoutDefaultPageWidthOverHeight = 0.762;
	m_dDefaultSizeRatio = 0.4; // Default to 40% of page's smallest dimension
	m_nActiveDivision = 0;
	m_aDivisions.Add( "" ); // Element 0 is always the default
	if (pszBasicType) SetupShape( pszBasicType );
}

CShape::CShape( const CShape& src )
{
	m_bValid = true;
	// Set parent to point to global preferences
	Reset();
	SetParent( &(MYAPP()->m_prefs) );
	m_IsRegular = -1; // Indeterminate
	CDbg::Out( "CShape::CShape(CShape&): Creating shape %08x from %08x\n",
		(void*)this,
		(void*)&src );
	(*this) = src;
}

CShape::~CShape()
{
	ClearAllFaces();
}

CShape&
CShape::operator =( CShape const& src )
{
	ClearAllFaces();
	m_IsRegular = -1; // Indeterminate
	POSITION pos;
	CString szFaceName;
	CFace* pSrcFace;
	for (pos = src.m_mapFaces.GetStartPosition(); pos != NULL; )
	{
		src.m_mapFaces.GetNextAssoc( pos, szFaceName, pSrcFace );
		CFace* pf = new CFace( this, *pSrcFace );
		m_mapFaces.SetAt( szFaceName, pf );
	} // for all faces
	m_szName = src.m_szName;
	m_dLayoutDefaultPageWidth = src.m_dLayoutDefaultPageWidth;
	m_dLayoutDefaultPageWidthOverHeight = src.m_dLayoutDefaultPageWidthOverHeight;
	m_dDefaultSizeRatio = src.m_dDefaultSizeRatio;
	this->m_nLayoutFit = src.m_nLayoutFit;
	this->m_bHasLayoutFit = src.m_bHasLayoutFit;
	CopyDict( src );
	m_aDivisions.RemoveAll();
	for (int n = 0; n < src.m_aDivisions.GetCount(); n++)
	{
		m_aDivisions.Add( src.m_aDivisions[n] );
	}
	this->SetActiveDivision( src.GetActiveDivisionIndex() );
	return *this;
} // CShape::operator =

// Copy dictionary from another shape object
void CShape::CopyDict( const CShape& src )
{
	POSITION pos;
	CString szEntryName, szEntryValue;
	for (pos = src.m_mapVariableDictionary.GetStartPosition(); pos != NULL; )
	{
		src.m_mapVariableDictionary.GetNextAssoc( pos, szEntryName, szEntryValue );
		m_mapVariableDictionary.SetAt( szEntryName, szEntryValue );
	}
} // CShape::CopyDict()

CFace* CShape::FindFace(const char *pszName)
{
	CFace* pReturn;
	if (!m_mapFaces.Lookup( pszName, pReturn ))
	{
		return NULL;
	}
	return pReturn;
}

// Free multi-vertex vector
void CShape::FreeMultiVertices()
{
	int n;
	int nSize = m_aMultiVertices.GetSize();
	for (n = 0; n < nSize; n++)
	{
		delete m_aMultiVertices[n];
		m_aMultiVertices[n] = NULL;
	}
	m_aMultiVertices.RemoveAll();
}

void CShape::ClearAllFaces()
{
	POSITION pos;
	CString sz;
	for (pos = m_mapFaces.GetStartPosition(); pos != NULL; )
	{
		CFace* p;
		m_mapFaces.GetNextAssoc( pos, sz, p );
		delete p;
		m_mapFaces.SetAt( sz, NULL );
	}
	m_mapFaces.RemoveAll();

	FreeMultiVertices();
	m_mapUnifiedVertices.RemoveAll();
	m_IsRegular = -1; // Indeterminate
}

// Returns 0 if successful
int
CShape::JoinFaceEdge(const char *pszFaceName, const char *pszEdgeName, const char *pszFace2Name, const char *pszEdge2Name)
{
	CFace* pFace1 = FindFace( pszFaceName );
	if (!pFace1)
	{
		return -1;
	}
	CFace* pFace2 = FindFace( pszFace2Name );
	if (!pFace2)
	{
		return -1;
	}
	return pFace1->JoinFaceEdge( pszEdgeName, pFace2, pszEdge2Name );
}

// m_mapUnifiedVertices maps face:vertex to multi-vertex object
// Vertices are congruent if they
// 1. Belong to different faces
// 2. Belong to faces which are joined along an edge
// 3. Both share edges which are joined
// 4. Shared edge comes after one vertex in clockwise rotation
//		but before the other.
// Note that vertex congruency is associative, so if a vertex in
// one face is congruent with a vertex in an adjacent face, it is
// also congruent with a vertex in a face adjacent to the adjacent
// face.
int CShape::MapVertices()
{
	CString szCompositeName;
	POSITION pos;
	CString szFaceName;
	CFace* pFace;
	m_mapUnifiedVertices.RemoveAll();
	FreeMultiVertices();
	for (pos = m_mapFaces.GetStartPosition(); pos != NULL; )
	{
		m_mapFaces.GetNextAssoc( pos, szFaceName, pFace );
		POSITION posVertex;
		CString szVertexName;
		// Set order of vertices within face
		pFace->SetVertexOrder();
		CVertex* pv;
		for (posVertex = pFace->m_pVertices.GetStartPosition();
			posVertex != NULL; )
		{
				pFace->m_pVertices.GetNextAssoc( posVertex, szVertexName, pv );
				szCompositeName.Format( "%s:%s",
					(const char *)pFace->m_szFaceName,
					(const char *)szVertexName );
				CMultiVertex* pvCheck;
				if (m_mapUnifiedVertices.Lookup( szCompositeName, pvCheck ))
				{
					CString szMsg;
					szMsg.Format( "Error: got duplicate for %s", (const char*)szCompositeName );
					AfxMessageBox( szMsg );
					continue;
				} // Error
				// Check for congruency
				int nMultiVertex;
				CMultiVertex* pmv = NULL;
				int nMVSize = m_aMultiVertices.GetSize();
				for (nMultiVertex = 0; nMultiVertex < nMVSize; nMultiVertex++, pmv = NULL)
				{
					pmv = m_aMultiVertices[nMultiVertex];
					if (pmv->IsCongruent( pv ))
					{
						pmv->AddMember( pv );
						m_mapUnifiedVertices.SetAt( szCompositeName, pmv );
						break;
					} // Congruent
				} // for all vertices
				if (pmv != NULL)
				{
					continue;
				} // We found one
				// If we fell through here, we need to create one
				pmv = new CMultiVertex();
				pmv->AddMember( pv );
				m_mapUnifiedVertices.SetAt( szCompositeName, pmv );
				m_aMultiVertices.Add( pmv );
		} // for all vertices
	} // for all faces
	return 0;
}

// Create symbolic representation of shape
// "shape", "{", face, ..., "}"
CString
CShape::SaveSymbolic( CArray<CString,const char*>& a ) const
{
	CString sz;
	a.RemoveAll();
	a.Add( "object shape" );
	a.Add( "{" );
	sz.Format( "name %s", GetName() );
	a.Add( sz );
	// Add dictionary entries:
	// dictentry name value....
	POSITION pos;
	CString szEntryName, szEntryValue;
	for (pos = this->m_mapVariableDictionary.GetStartPosition(); pos != NULL; )
	{
		m_mapVariableDictionary.GetNextAssoc( pos, szEntryName, szEntryValue );
		szEntryValue.Replace( "\r\n", "\\n" );
		sz = "dictentry ";
		sz += szEntryName;
		sz += ' ';
		sz += szEntryValue;
		a.Add( sz );
	}
	// Get any properties we may have set
	CString szProperties( GetSymbolicForm() );
	szProperties.TrimRight();
	if (!szProperties.IsEmpty())
	{
		sz = "settings ";
		sz += szProperties;
		a.Add( sz );
	}
	//sz.Format( "%d", m_apFaces.GetSize() );
	//a.Add( sz );
	CString szDebug;
	szDebug = sz;
	CString szFaceName;
	CFace* pFace;
	for (pos = m_mapFaces.GetStartPosition(); pos != NULL; )
	{
		m_mapFaces.GetNextAssoc( pos, szFaceName, pFace );
		szDebug += ",{";
		szDebug += pFace->SaveSymbolic( a );
		szDebug += '}';
		//sz.Format( "%d", aFace.GetSize() + aFaceEdges.GetSize() + aFaceVertices.GetSize() );
		//a.Add( sz );
	} // for all faces
	a.Add( "}" ); // Close shape
	return szDebug;
} // CShape::SaveSymbolic()

// Restore from symbolic representation, stage 1
// numfaces, nface1elements, face1...
// This is where we allocate all the face, edge and vertex objects.
int
CShape::CreateFromSymbolic1( CArray<CString,const char*>& a )
{
	// What we have:
	/****
	object shape
	{
		name My shape
		object face
		{
			name A
			object edge
			{
				name a
				...
	****/
	ClearAllFaces();
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
		if (MoveObject( aObject, a, nCurrent + 1, false ) < 0)
		{
			return PE_ERROR;
		}
		// Dispatch to appropriate object
		switch (PEReturn)
		{
		case PE_OBJECT_FACE:
			{
			CFace* pFace = new CFace( this );
			nNested = pFace->CreateFromSymbolic1( aObject );
			if (nNested < 0)
			{
				delete pFace;
				return PE_ERROR;
			}
			// pFace should now have its name set properly
			m_mapFaces.SetAt( pFace->m_szFaceName, pFace );
			}
			// Skip to last entry of face - for ( ; ; ) will skip past it
			nCurrent += (nNested - 1);
			break;
		default:
			return PE_ERROR;
		}
	}
	return 0;
} // CShape::CreateFromSymbolic1()

// Restore from symbolic representation, stage 2
// This is where we recurse down and fix up all the
// references in objects created in CreateFromSymbolic1()
int
CShape::FixupSymbolic()
{
	POSITION pos;
	CString szFaceName;
	CFace* pFace;
	for (pos = m_mapFaces.GetStartPosition(); pos != NULL; )
	{
		m_mapFaces.GetNextAssoc( pos, szFaceName, pFace );
		pFace->FixupSymbolic();
	}
	return 0;
} // CShape::FixupSymbolic()

// Return true if shape is valid
bool CShape::IsValid() const
{
	// We need to have at least one face
	return m_bValid && (m_mapFaces.GetCount() > 0);
}

// Push a face:vertex join onto map of face:edge => face:edge joins
// It is not a push but a replacement/insert.  However the map
// is used as a named entry unordered queue.
void
CShape::PushFaceEdgeJoin( const char *pszFaceEdge, const char *pszDestFaceEdge )
{
	m_mapFaceEdge.SetAt( pszFaceEdge, pszDestFaceEdge );
} // CShape::PushFaceEdgeJoin()

// Variant with face:edge values already parsed
void
CShape::PushFaceEdgeJoin( const char *pszFace, const char *pszEdge, const char *pszDestFace, const char *pszDestEdge )
{
	CString szFaceEdge;
	CString szDestFaceEdge;
	szFaceEdge.Format( "%s:%s", pszFace, pszEdge );
	szDestFaceEdge.Format( "%s:%s", pszDestFace, pszDestEdge );
	PushFaceEdgeJoin( szFaceEdge, szDestFaceEdge );
} // CShape::PushFaceEdgeJoin()

// Separate all faces from each other
void
CShape::CutAllFaceJoins( bool bSaveSymbolic /*= false*/ )
{
	POSITION pos;
	CString szFaceName;
	CFace* pFace;
	int nTotalCuts = 0;
	for (pos = m_mapFaces.GetStartPosition(); pos != NULL; )
	{
		m_mapFaces.GetNextAssoc( pos, szFaceName, pFace );
		nTotalCuts += pFace->DetachEdges( bSaveSymbolic );
	} // for all faces
} // CShape::CutAllFaceJoins()

// Pop a face:vertex from the map and if found, join with its
// destination.  Returns true if found
bool
CShape::PopFaceEdgeJoin( const char *pszFaceEdge, CMap<CString,const char*,CString,const char*>& mapPopped, CShapeLayout *pLayout )
{
	// Note that we will pop both this edge and the one it references
	CString szDestFaceEdge;
	if (!m_mapFaceEdge.Lookup( pszFaceEdge, szDestFaceEdge ))
	{
		CDbg::Out( "Warning: did not find %s in PopFaceEdgeJoin()\n", pszFaceEdge );
		return false;
	} // Did not find it

	// If dest is empty, delete them both
	if (szDestFaceEdge.IsEmpty())
	{
		CDbg::Out( "Removing empty entry for %s\n", pszFaceEdge );
		m_mapFaceEdge.RemoveKey( pszFaceEdge );
		return false;
	}

	// Parse into face and edge components
	CString szEdge( pszFaceEdge );
	CString szFace;
	// Get part up to colon as face name
	szFace = szEdge.SpanExcluding( ":" );
	// Delete face name and colon, leaving edge name
	szEdge.Delete( 0, szFace.GetLength() + 1 );

	CString szDestFace;
	CString szDestEdge;
	// Get part up to colon as face name
	szDestFace = szDestFaceEdge.SpanExcluding( ":" );
	// Delete face name and colon, leaving edge name
	szDestEdge = szDestFaceEdge.Mid( szDestFace.GetLength() + 1 );

	// Check for minimal join angle.  Note the positional
	// dependency: szFace : szEdge are assumed to be part
	// of a layout we're assembling, and szDestFace : szDestFaceEdge
	// are "candidates" which we may not have room for.
	CFace* pFace1 = FindFace( szFace );
	if (!pFace1) 
	{
		CDbg::Out( 0, "Error: could not find face %s specified in map entry %s:%s\n",
				(LPCTSTR)szFace, (LPCTSTR)szFace, (LPCTSTR)szEdge );
		return false;
	}
	CEdge* pEdge1 = pFace1->GetEdge( szEdge );
	ASSERT( pEdge1 );
	
	// Get destination
	CFace* pFace2 = FindFace( szDestFace );
	ASSERT( pFace2 );
	CEdge* pEdge2 = pFace2->GetEdge( szDestEdge );
	ASSERT( pEdge2 );

	// hgroover 20070407 - if new forced-split attribute is present on either this edge or
	// the edge we're joining to, disallow the join.
	if (pEdge1->GetForcedSplit() || pEdge2->GetForcedSplit())
	{
		return false;
	}

	// Get sum of angles for end 0 of edge, end 1 of dest
	// Note that we'll be summing face angles in a clockwise
	// rotation, but within each face we'll walk vertices
	// counterclockwise.
	double dAngleSum0 = pEdge2->m_pEndPoints[1]->m_dInsideAngle;
	CEdge* pWalker = pEdge1;
	while (pWalker)
	{
		dAngleSum0 += pWalker->m_pEndPoints[0]->m_dInsideAngle;
		pWalker = pWalker->m_pEndPoints[0]->m_pSides[0]->m_pOutwardConnectedEdge;
	}

	// Violation on this end?
	double dMinJoinAngle = this->GetMinFitAngle();
	if (dAngleSum0 + dMinJoinAngle > 360.0)
	{
		// Remove from list
		m_mapFaceEdge.RemoveKey( pszFaceEdge );
		CDbg::Out( "Rejected layout combo for %s, angle sum %f\n",
			pszFaceEdge, dAngleSum0 );
		return false;
	} // Vie-oh-layshun!

	// Get sum of angles for end 1 of edge, end 0 of dest
	// We'll be summing face angles counterclockwise, but
	// walking vertices clockwise.
	double dAngleSum1 = pEdge2->m_pEndPoints[0]->m_dInsideAngle;
	pWalker = pEdge1;
	while (pWalker)
	{
		dAngleSum1 += pWalker->m_pEndPoints[1]->m_dInsideAngle;
		pWalker = pWalker->m_pEndPoints[1]->m_pSides[1]->m_pOutwardConnectedEdge;
	}

	// Do we have a violation?
	if (dAngleSum1 + dMinJoinAngle > 360.0)
	{
		// Remove from list
		m_mapFaceEdge.RemoveKey( pszFaceEdge );
		CDbg::Out( "Rejected other end of combo for %s, angle sums %f, %f\n",
			pszFaceEdge, dAngleSum0, dAngleSum1 );
		return false;
	} // Violation on end 1

	// Is a page group specified, and will it fit on the page?
	if (true)
	{
		bool bFit;
		// Join edges temporarily
		CEdge *pSaveE1Out, *pSaveE2Out;
		pSaveE1Out = pEdge1->m_pOutwardConnectedEdge;
		pSaveE2Out = pEdge2->m_pOutwardConnectedEdge;
		pEdge1->m_pOutwardConnectedEdge = pEdge2;
		pEdge2->m_pOutwardConnectedEdge = pEdge1;
		// Trial fit - this is also where we check for polygon collisions
		// Note that we're using the face reference from the layout, which is not
		// joined, for the purposes of polygon collisions.
		CFace* pUnjoined = pLayout->m_pShape->FindFace( szFace );
		CPageGroup TrialPage( pLayout->m_dPageWidthInLogicalUnits, pLayout->m_dPageWidthOverHeight );
		bFit = TrialPage.AddFace( pFace1, 0, false, pUnjoined, pLayout->m_pShape->GetLayoutFit() );
		// Restore previous connection (should be null)
		pEdge1->m_pOutwardConnectedEdge = pSaveE1Out;
		pEdge2->m_pOutwardConnectedEdge = pSaveE2Out;
		if (!bFit)
		{
			CDbg::Out( "Did not fit in page\n" );
			// Remove from list
			m_mapFaceEdge.RemoveKey( pszFaceEdge );
			return false;
		} // no fit
	} // Test for fit in page group

	// Remove both
	m_mapFaceEdge.RemoveKey( pszFaceEdge );
	m_mapFaceEdge.RemoveKey( szDestFaceEdge );

	// Add to popped list
	mapPopped.SetAt( pszFaceEdge, szDestFaceEdge );

	//CDbg::Out( "Joining %s with %s\n", pszFaceEdge, (LPCTSTR)szDestFaceEdge );

	// Join it.  0 return indicates no errors.
	return (JoinFaceEdge( szFace, szEdge, szDestFace, szDestEdge ) == 0);

} // CShape::PopFaceEdgeJoin()

// Parsed variant
bool
CShape::PopFaceEdgeJoin( const char *pszFace, const char *pszEdge, CMap<CString,const char*,CString,const char*>& mapPopped, CShapeLayout *pLayout )
{
	CString szFaceEdge;
	szFaceEdge.Format( "%s:%s", pszFace, pszEdge );
	return PopFaceEdgeJoin( szFaceEdge, mapPopped, pLayout );
} // CShape::PopFaceEdgeJoin()

// Get combined join list into any map
// Return number of entries
int
CShape::BuildJoinList( CMap<CString,const char*,CString,const char*>& map ) const
{
	int nReturn = 0;
	POSITION pos;
	CString szFaceName;
	CFace* pFace;
	int nTotalCuts = 0;
	map.RemoveAll();
	for (pos = m_mapFaces.GetStartPosition(); pos != NULL; )
	{
		m_mapFaces.GetNextAssoc( pos, szFaceName, pFace );
		CArray<CString,const char*> aSource, aDest;
		int nFaceCount = pFace->GetJoinList( aSource, aDest );
		nReturn += nFaceCount;
		int n2;
		for (n2 = 0; n2 < nFaceCount; n2++)
		{
			CDbg::Out( 2, "BuildJoinList face(%d): %s - %s\n", n2, (LPCTSTR)aSource[n2], (LPCTSTR)aDest[n2] );
			map.SetAt( aSource[n2], aDest[n2] );
		} // for all joins in face
	} // for all faces
	return nReturn;
} // CShape::BuildJoinList()

// Remove face from map of face:edge -> face:edge joins wherever
// it appears as a target (on the right hand side).  Return
// number of entries removed.
int
CShape::RemoveFaceAsTarget(CFace *pFace, bool bRemoveAsKey /*=false*/, CMap<CString,const char*,int,int>* pRightPopMap /*=NULL*/)
{
	POSITION pos;
	bool bDone = false;
	CString szLeft, szRight;
	LPCSTR pszTrigger;
	int nRemoved = 0;
	while (!bDone)
	{
		bDone = true;
		for (pos = m_mapFaceEdge.GetStartPosition(); pos != NULL; )
		{
			m_mapFaceEdge.GetNextAssoc( pos, szLeft, szRight );
			bool bCompare = (pFace->m_szFaceName.Compare( szRight.SpanExcluding( ":" ) ) == 0);
			pszTrigger = "right";
			if (!bCompare && bRemoveAsKey)
			{
				bCompare = (pFace->m_szFaceName.Compare( szLeft.SpanExcluding( ":" ) ) == 0);
				pszTrigger = "left";
			}
			if (bCompare)
			{
				bDone = false;
				if (pRightPopMap)
				{
					int nRemovals;
					if (pRightPopMap->Lookup( szRight.SpanExcluding( ":" ), nRemovals ))
					{
						nRemovals++;
					}
					else
					{
						nRemovals = 1;
					}
					pRightPopMap->SetAt( szRight.SpanExcluding( ":" ), nRemovals );
					// Do the same for left side.  Orphan test will be if nRemovals == nfaces*2
					if (pRightPopMap->Lookup( szLeft.SpanExcluding( ":" ), nRemovals ))
					{
						nRemovals++;
					}
					else
					{
						nRemovals = 1;
					}
					pRightPopMap->SetAt( szLeft.SpanExcluding( ":" ), nRemovals );
				} // right-triggered and pop map defined
				CDbg::Out( "Removing %s -> %s from map, triggered by %s\n",
						(LPCTSTR)szLeft, (LPCTSTR)szRight, pszTrigger );
				m_mapFaceEdge.RemoveKey( szLeft );
				nRemoved++;
				break;
			}
		} // for all entries
	} // not done
	return nRemoved;
}

// Strip elements from one array to another, up to and including
// ending brace ("}").  Returns total removed from source.
int
CShape::MoveObject( CArray<CString,LPCTSTR>& aDest,
				    CArray<CString,LPCTSTR>& aSrc,
					int nStartSource, bool bEmptyFromSource /*= true*/ )
{
	int nLimit = aSrc.GetSize();
	int n;
	int nNestLevel = 0;
	int nTotalRemoved = 0;
	for (n = nStartSource; n < nLimit; n++)
	{
		if (aSrc[n].Compare( "{" ) == 0)
		{
			nNestLevel++;
		}
		if (aSrc[n].Compare( "}" ) == 0)
		{
			nNestLevel--;
		}
		aDest.Add( aSrc[n] );
		if (bEmptyFromSource)
		{
			aSrc[n].Empty();
		}
		nTotalRemoved++;

		// This is the first place we can safely test for no nesting
		if (!nNestLevel) break;
	} // for all in source
	return nTotalRemoved;
} // CShape::MoveObject()

// Parse a single element.  If bAllowExternalJoins, hook up
// outward connections (should be false on pass 1)
int CShape::ParseElement(LPCTSTR lpElement, bool bAllowExternalJoins /*=true*/)
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
		m_szName = lpElement;
		return PE_ATTRIBUTE;
	}
	else if (!sz.CompareNoCase( "settings" ))
	{
		// Pass to underlying CPreferenceSet object
		ParseSymbolicForm( CString( lpElement ) );
		return PE_ATTRIBUTE;
	}
	else if (!sz.CompareNoCase( "dictentry" ))
	{
		// Add to shape dictionary
		sz = lpElement;
		CString szEntryName;
		szEntryName = sz.SpanExcluding( " \t" );
		lpElement += szEntryName.GetLength();
		lpElement += strspn( lpElement, " \t" );
		sz = lpElement;
		sz = sz.SpanExcluding( "\r\n" );
		// Restore escaped newlines
		sz.Replace( "\\n", "\r\n" );
		this->m_mapVariableDictionary.SetAt( szEntryName, sz );
		return PE_DICTENTRY;
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
		if (!stricmp( lpElement, "face" ))
		{
			return PE_OBJECT_FACE;
		}

		CDbg::Out( "Got unexpected object in %s:%u - %s %s\n",
			__FILE__,
			__LINE__,
			(LPCTSTR)sz,
			lpElement );
		return PE_OBJECT_UNKNOWN;
	}
	else if (!sz.CompareNoCase( "layout" ))
	{
		// Ignore layout
	}

	CDbg::Out( 0, "Got unexpected element in %s:%u - %s %s\n",
			__FILE__,
			__LINE__,
			(LPCTSTR)sz, lpElement );
	return PE_UNKNOWN;

}

// Get first face
CFace*
CShape::GetFirstFace() const
{
	POSITION pos;
	bool bFirstPass = true;
	pos = m_mapFaces.GetStartPosition();
	if (pos == NULL)
	{
		return NULL;
	}
	// Get first face in lexical order.
	// This will provide consistency
	CString sz, szLeast;
	CFace *p, *pLeast;
	while (pos != NULL)
	{
		m_mapFaces.GetNextAssoc( pos, sz, p );
		if (bFirstPass || sz < szLeast)
		{
			szLeast = sz;
			pLeast = p;
		}
		bFirstPass = false;
	}
	return pLeast;
} // CShape::GetFirstFace()

// Stellate (pop out) faces by making each face into a pyramid.
// The edges of the pyramid are equal to the base edges * ratio
int CShape::Stellate(double dRatio, BOOL bKepler /*=FALSE*/)
{
	int nAdded = 0;

	// Turn off rendering
	m_bValid = false;

	m_IsRegular = -1; // Indeterminate

	// Reset count of minimum ratio not met
	this->ResetMinimumRatio();

	// Build array of all faces (since we'll be increasing
	// the current number of faces by 200-500%).
	CArray<CFace*,CFace*> Faces;
	POSITION pos;
	CString szFaceName;
	CFace* pFace;
	for (pos = m_mapFaces.GetStartPosition(); pos != NULL; )
	{
		m_mapFaces.GetNextAssoc( pos, szFaceName, pFace );
		Faces.Add( pFace );
	} // for all faces

	CDbg::Out( "\n\n--- Stellate %f Kepler %d ---\n", dRatio, bKepler );

	// FIXME We need an option to do a fixed pyramid height.
	// As it stands currently the height will vary according to the longest edge
	// in each face.  There could be considerable variance on a second-order
	// stellation.

	// Go through the list of faces
	int nFace;
	int nSize = Faces.GetSize();

	if (bKepler)
	{
		// For Kepler stellation, we need to bisect all edges of each face, similar
		// to a 2-frequency geodesic breakdown
		int nFaceCount = KStellate( dRatio );
		m_bValid = true;
		return nFaceCount;
	}
	else
	for (nFace = 0; nFace < nSize; nFace++)
	{
		nAdded += StellateFace( Faces[nFace], dRatio );
	} // For all faces

	// Now re-create multi-vertex map
	MapVertices();

	// Caller must re-create m_mapFaceEdge and other supplemental objects

	// Enable rendering
	m_bValid = true;

	// Complain if minimum ratio was exceeded
	if (this->m_bMinStellationRatioExceeded)
	{
		CString sz;
		sz.Format( CShape::szMIN_STELLATION_RATIO_WARNING_fsf,
			dRatio * 100.0, "shape", this->m_dMinStellationRatio * 100.0 );
		::AfxMessageBox( sz, MB_OK );
	}

	// Return number of faces we ADDED
	return nAdded;
}

// Stellate a single face.  Return number of faces added.
int
CShape::StellateFace( CFace* pFace, double dRatio )
{
	int nAdded = 0;
	CString szName1, szName2, szName3, szName4;
	CString szFaceGroupName;
	bool IsSquare = (pFace->m_pEdges.GetSize() == 4);

	// Construct names for new faces by adding "s1", "s2" and "s3" (also "s4" if face is square)
	// Destroy current face when we're done
	// Make base edge the original base edges

	// In theory we can do this for unlimited faces, but pyramid faces will always be triangular.
	// We can only find the epicenter by bisection for triangles and regular convex polygonal faces.

		// Make sure those new edge names are unique!
		szName1 = MakeUniqueFaceName( pFace->m_szFaceName, "s1" );
		szName2 = MakeUniqueFaceName( pFace->m_szFaceName, "s2" );
		szName3 = MakeUniqueFaceName( pFace->m_szFaceName, "s3" );
		if (IsSquare) 
		{
			szName4 = MakeUniqueFaceName( pFace->m_szFaceName, "s4" );
		}
		szFaceGroupName = MakeUniqueFaceName( pFace->m_szFaceName, "_grp" );

		// Remove current face from collection. We'll destroy it later
		this->m_mapFaces.RemoveKey( pFace->m_szFaceName );

		// Find the first edge for each face (the one orientation is calculated from)
		CEdge* pBaseEdge = pFace->GetBaseEdge();
		ASSERT( pBaseEdge != NULL );

		// Reset orientation from base edge to 90 degrees
		pFace->m_fOrientation = 90.0;

		// Get other two (or three) edges from which we'll construct szName1 and szName2
		CEdge *pName1Edge, *pName2Edge, *pName3Edge, *pName4Edge;
		pName4Edge = NULL;
		pName1Edge = pBaseEdge->m_pEndPoints[1]->m_pSides[1];
		if (IsSquare)
		{
			// Keep base edge as pName3Edge and make pName1Edge opposite to base
			pName4Edge = pBaseEdge->m_pEndPoints[1]->m_pSides[1];
			ASSERT( pName4Edge != NULL );
			pName1Edge = pName1Edge->m_pEndPoints[1]->m_pSides[1];
		}
		ASSERT( pName1Edge != NULL );
		pName2Edge = pName1Edge->m_pEndPoints[1]->m_pSides[1];
		ASSERT( pName2Edge != NULL );
		pName3Edge = pBaseEdge;

		// Set concave attribute on original edges when we're done
		// FIXME This is NOT necessarily true.  It is a function of the original
		// FIXME dihedral join angle and the height of the stellation!!!
		// FIXME But anyway this is just a folding guide, it is not structural...

		CDbg::Out( "Face %s first edge %s splicing %s and %s%s\n",
			(LPCTSTR)pFace->m_szFaceName,
			(LPCTSTR)pBaseEdge->GetName(),
			(LPCTSTR)pName1Edge->GetName(),
			(LPCTSTR)pName2Edge->GetName(),
			IsSquare ? " [square]" : "" );
		// Calculate the edge lengths for the new pyramid
		// 1. Find the centerpoint of the face from the base edge
		// 2. Find the segment lengths from base edge endpoints to center
		// 3. Find pyramid height
		// 4. Solve for pyramid edges using Pythagorean theorem

		// Construct new faces
		CFace* pName1Face = new CFace( this, szName1 );
		CFace* pName2Face = new CFace( this, szName2 );
		CFace* pName3Face = new CFace( this, szName3 );
		ASSERT( pName1Face != NULL );
		ASSERT( pName2Face != NULL );
		ASSERT( pName3Face != NULL );
		CFace* pName4Face = NULL;

		double dTargetEdgeLength ;
		if (IsSquare)
		{
			pName4Face = new CFace( this, szName4 );
			ASSERT( pName4Face != NULL );
			// Simply apply ratio to maximum base edge for the target (ideal) edge length.
			// In an irregular pyramid this will be the longest. The corresponding center leg
			// must also be the longest, and we'll check for invalid ratios below once we've
			// determined the center leg lengths.
			double dMaxBaseEdge = max( max( max( pBaseEdge->m_dLength, pName1Edge->m_dLength ), pName2Edge->m_dLength ), pName4Edge->m_dLength );
			dTargetEdgeLength =  dMaxBaseEdge * dRatio;
			/*!
			// To construct a pyramid, we need a center. Take the intersection
			// of the lines joining opposite midpoints as the center.
			// We can take advantage of the Varignon Parallelogram http://mathworld.wolfram.com/VarignonParallelogram.html
			// Inscribing a quadrilateral which joins the midpoints of adjacent sides of a quadrilateral
			// always forms a parallelogram. Also see http://mathworld.wolfram.com/VarignonsTheorem.html for
			// other interesting properties, such as the area of the Varignon parallelogram for a convex quadrilateral
			// is half that of the quadrilateral itself.
			// See trapezoid-midpoint-calc.gif for labels.

			The base edge is c in this diagram.
			We need the lengths of I, J, K and L.
			Segment x connects the midpoints of b and d, and segment y connects the midpoints of a and c.
			The Varignon parallelogram connecting the midpoints of a, b, c, and d has legs of lengths g and h.
			Angles alpha, gamma, delta, and theta are all known. The half-lengths of a, b, c and d are
			m, n, p, and q respectively. The half-lengths of x and y are z and o, respectively.

			The following derivations make use of the SAS theorem and law of cosines, described at
			http://mathworld.wolfram.com/SASTheorem.html and http://mathworld.wolfram.com/LawofCosines.html
			g = sqrt( n*n + p*p - 2 * n * p * cos( alpha ) );
			h = sqrt( p*p + q*q - 2 * p * q * cos( gamma ) );

			We derive these angles using the SSS theorem and the law of cosines, described at
			http://mathworld.wolfram.com/SSSTheorem.html
			phi = acos( ( g*g + p*p - n*n ) / (2 * g * p) );
			mu = acos( ( h*h + p*p - q*q ) / (2 * h * p) );
			rho = acos( (m*m + g*g - q*q) / (2 * m * g) );
			tau = acos( (m*m + h*h - n*n) / (2 * m * h) );

			We can derive angle w as
			w = 180 - mu - phi
			and derive x using the law of cosines:
			x = sqrt( h*h + g*g - 2*h*g * cos( w ) );
			z = x/2;
			More angles we need are
			chi = 180 - alpha - phi
			xi = 180 - theta - tau
			And we can derive
			eta = 180 - xi - chi
			and can derive y using the law of cosines:
			y = sqrt( h*h + g*g - 2*h*g * cos( eta ) );
			o = y/2;

			Now we can derive the angles on either side of the bimeridian y using SSS and the law of cosines:
			T = phi + acos( (o*o + g*g - z*z) / (2 * o * g) );
			R = 180 - T;
			E = rho + acos( (o*o + g*g - z*z) / (2 * o * g) );
			F = 180 - E;

			And we can use SAS to derive our final results:
			I = sqrt( o*o + p*p - 2*o*p * cos( R ) );
			J = sqrt( o*o + p*p - 2*o*p * cos( T ) );
			K = sqrt( o*o + m*m - 2*o*m * cos( E ) );
			L = sqrt( o*o + m*m - 2*o*m * cos( F ) );

			Test case parameters are in Trapezoid-base-pyramid-testcase.xls
			*/
			TrapezoidMidpointCalc tm;
			tm.c = pBaseEdge->m_dLength;
			tm.alpha = pBaseEdge->m_pEndPoints[0]->m_dInsideAngle;
			tm.gamma = pBaseEdge->m_pEndPoints[1]->m_dInsideAngle;
			tm.d = pBaseEdge->m_pEndPoints[1]->m_pSides[1]->m_dLength;
			CEdge* pOppositeBase = pBaseEdge->m_pEndPoints[1]->m_pSides[1]->m_pEndPoints[1]->m_pSides[1];
			tm.delta = pOppositeBase->m_pEndPoints[0]->m_dInsideAngle;
			tm.a = pOppositeBase->m_dLength;
			tm.theta = pOppositeBase->m_pEndPoints[1]->m_dInsideAngle;
			tm.b = pOppositeBase->m_pEndPoints[1]->m_pSides[1]->m_dLength;
			// FIXME if we don't have enough inputs, we're screwed - do something about it
			if (!tm.Calculate())
			{
				return 0;
			}
			// We've applied ratio to the maximum edge length. We'll determine pyramid height
			// using the putative maximum edge length compared to the maximum incenter leg.
			double dMaxLeg = max( tm.I, max( tm.J, max( tm.K, tm.L ) ) );
			if (dMaxLeg > dTargetEdgeLength)
			{
				this->m_bMinStellationRatioExceeded = true;
				/// This would yield a height of 0
				this->m_dMinStellationRatio = dMaxLeg / dMaxBaseEdge;
			}
			/// Get pyramid height using pythagorean theorem
			double dPyramidHeight = sqrt( dTargetEdgeLength*dTargetEdgeLength - dMaxLeg*dMaxLeg );
			CDbg::Out( "Pyramid height using max base edge %f and max incenter leg %f is %f\n",
				dMaxBaseEdge, dMaxLeg, dPyramidHeight );
			/// Determine individual edge lengths
			double Lh, Jh, Ih, Kh;
			double dPyramidHeight2 = dPyramidHeight * dPyramidHeight;
			Lh = sqrt( tm.L*tm.L + dPyramidHeight2 );
			Jh = sqrt( tm.J*tm.J + dPyramidHeight2 );
			Ih = sqrt( tm.I*tm.I + dPyramidHeight2 );
			Kh = sqrt( tm.K*tm.K + dPyramidHeight2 );
			CDbg::Out( "Created new faces %s, %s, and %s from %s\n", (LPCTSTR)szName1, (LPCTSTR)szName2,
				(LPCTSTR)szName4,
				(LPCTSTR)pFace->m_szFaceName );
			CDbg::Out( "Pyramid edge lengths (clockwise from right of base edge are %f, %f, %f, %f and base edge length is %f\n",
				Jh, Ih, Kh, Lh, pBaseEdge->m_dLength );

			/// Pyramid face lengths will go clockwise and end with the base edge
			const char *apszNames[3] = { "a", "b", "c" };
			double adLengths[3] = { Lh, Kh, pName1Edge->m_dLength };
			static double adAngles[] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
			pName1Face->CreatePolygon( apszNames, adLengths, adAngles, 3 );
			// Set north pole to tip of pyramid
			pName1Face->m_fOrientation = 90.0;
			pName1Face->SetBaseEdgeName( apszNames[2] );

			adLengths[0] = Jh;
			adLengths[1] = Lh;
			adLengths[2] = pName2Edge->m_dLength;
			pName2Face->CreatePolygon( apszNames, adLengths, adAngles, 3 );
			// Set north pole to tip of pyramid
			pName2Face->m_fOrientation = 90.0;
			pName2Face->SetBaseEdgeName( apszNames[2] );

			adLengths[0] = Ih;
			adLengths[1] = Jh;
			adLengths[2] = pName3Edge->m_dLength;
			pName3Face->CreatePolygon( apszNames, adLengths, adAngles, 3 );
			// Set north pole to tip of pyramid
			pName3Face->m_fOrientation = 90.0;
			pName3Face->SetBaseEdgeName( apszNames[2] );

			adLengths[0] = Kh;
			adLengths[1] = Ih;
			adLengths[2] = pName4Edge->m_dLength;
			pName4Face->CreatePolygon( apszNames, adLengths, adAngles, 3 );
			pName4Face->m_fOrientation = 90.0;
			pName4Face->SetBaseEdgeName( apszNames[2] );

			// Set up joins
			CEdge* pName1Face_c = pName1Face->GetEdge( "c" );
			ASSERT( pName1Face_c != NULL );
			pName1Face_c->m_bConcave = true;
			pName1Face_c->JoinOutward( pName1Edge->m_pOutwardConnectedEdge );

			CEdge* pName1Face_b = pName1Face->GetEdge( "b" );
			ASSERT( pName1Face_b != NULL );
			pName1Face_b->JoinOutward( pName4Face->GetEdge( "a" ) );

			CEdge* pName1Face_a = pName1Face->GetEdge( "a" );
			ASSERT( pName1Face_a != NULL );
			pName1Face_a->JoinOutward( pName2Face->GetEdge( "b" ) );

			CEdge* pName2Face_c = pName2Face->GetEdge( "c" );
			ASSERT( pName2Face_c != NULL );
			pName2Face_c->m_bConcave = true;
			pName2Face_c->JoinOutward( pName2Edge->m_pOutwardConnectedEdge );

			CEdge* pName2Face_a = pName2Face->GetEdge( "a" );
			ASSERT( pName2Face_a != NULL );
			pName2Face_a->JoinOutward( pName3Face->GetEdge( "b" ) );

			CEdge* pName3Face_c = pName3Face->GetEdge( "c" );
			ASSERT( pName3Face_c != NULL );
			pName3Face_c->m_bConcave = true;
			pName3Face_c->JoinOutward( pName3Edge->m_pOutwardConnectedEdge );

			CEdge* pName3Face_a = pName3Face->GetEdge( "a" );
			ASSERT( pName3Face_a != NULL );
			pName3Face_a->JoinOutward( pName4Face->GetEdge( "b" ) );

			CEdge* pName4Face_c = pName4Face->GetEdge( "c" );
			ASSERT( pName4Face_c != NULL );
			pName4Face_c->m_bConcave = true;
			pName4Face_c->JoinOutward( pName4Edge->m_pOutwardConnectedEdge );

		}
		else
		{
		// 1&2. Let's consider an edge c with facing angle C.
		// The other two edges (going clockwise) are a and b,
		// with facing angles A and B respectively:
		//
		//         /\                     |
		//        /E| \                   |
		//       /  |   \                 |
		//      /  d|     \               |
		//     /f   |       \             |
		//    /    F|         \           |
		//   /  e  /C\         \          |
		//  /D /a        b\     \         |
		// /B_______c___________A_\       |
		//
		// We already know B, c, and A.
		// We also know C = 180 - B - A.
		// The law of sines gives us
		// sin(B) / b = sin(C) / c = sin(A) / a
		// Therefore sin(C) = c * sin(B)/b
		//			 b * sin(C) = c * sin(B)
		//			 b = c * sin(B) / sin(C)
		// and for a: sin(C) = c * sin(A) / a
		//			 a * sin(C) = c * sin(A)
		//			 a = c * sin(A) / sin(C)
		// and		 d = f * sin(D) / sin(F)
		double a, b, c, d, f, g;
		double A, B, C, D, E, F;
		c = pBaseEdge->m_dLength;
		f = pName1Edge->m_dLength;
		g = pName2Edge->m_dLength;
		B = pBaseEdge->m_pEndPoints[1]->m_dInsideAngle / 2;
		A = pBaseEdge->m_pEndPoints[0]->m_dInsideAngle / 2;
		C = 180 - A - B;
		D = B;
		E = pName1Edge->m_pEndPoints[1]->m_dInsideAngle / 2;
		F = 180 - D - E;
		double dSinC = sin( CFace::DegToRadans( C ) );
		b = c * sin( CFace::DegToRadans( B ) ) / dSinC;
		a = c * sin( CFace::DegToRadans( A ) ) / dSinC;
		d = f * sin( CFace::DegToRadans( D ) ) / sin( CFace::DegToRadans( F ) );

		// 3. Pyramid height will be the longest
		// centerpoint segment * dRatio
		// New method: pyramid height is sqrt(C^2 - A^2) * dRatio
		// Thus a 100% stellation will yield a pyramid whose longest upward edge (on an equilateral base)
		// is equal to its longest base edge.
		// FIXME see notes above - provide an option for fixed height
		// Calculate minimum percentage and save it for display later
		dTargetEdgeLength = max( c, max( f, g ) );
		double dHeight = max( b, max( a, d ) );
		if (dHeight > dTargetEdgeLength * dRatio)
		{
			double dMinStellationRatio = dHeight / dTargetEdgeLength;
			if (dMinStellationRatio > m_dMinStellationRatio)
			{
				this->m_dMinStellationRatio = dMinStellationRatio;
			}
			dHeight = 0.0;
			this->m_bMinStellationRatioExceeded = true;
		}
		else
		{
			dTargetEdgeLength *= dRatio;
			dHeight = sqrt( dTargetEdgeLength * dTargetEdgeLength - dHeight * dHeight );
		}

		// 4. Edges are now solveable using Pythagorean method
		double dBaseEdgeEnd1Length = sqrt( a * a + dHeight * dHeight );
		double dBaseEdgeEnd0Length = sqrt( b * b + dHeight * dHeight );
		double dCommonEdge12Length = sqrt( d * d + dHeight * dHeight );

		// Let's be consistent - dump clockwise
		CDbg::Out( "a=%f, d=%f, b=%f, pyramid edges %f, %f, %f, height %f\n",
			a, d, b,
			dBaseEdgeEnd1Length,
			dCommonEdge12Length,
			dBaseEdgeEnd0Length,
			dHeight );
		// Lazy method
		//double dBaseEdgeEnd1Length = pName1Edge->m_dLength * dLazyRatio;
		//double dBaseEdgeEnd0Length = pName2Edge->m_dLength * dLazyRatio;
		//double dCommonEdge12Length = dBaseEdgeEnd1Length;
		CDbg::Out( "Created new faces %s, %s from %s\n", (LPCTSTR)szName1, (LPCTSTR)szName2,
			(LPCTSTR)pFace->m_szFaceName );
		CDbg::Out( "Pyramid edge lengths are %f, %f, %f and base edge length is %f\n",
			dBaseEdgeEnd1Length, dCommonEdge12Length, dBaseEdgeEnd0Length, pBaseEdge->m_dLength );

		CDbg::Out( "Original length for %s edge %f, for %s edge %f\n",
			(LPCTSTR)szName1, pName1Edge->m_dLength,
			(LPCTSTR)szName2, pName2Edge->m_dLength );
		const char *apszNames[3] = { "a", "b", "c" };
		double adLengths[3] = { dCommonEdge12Length, dBaseEdgeEnd1Length, pName1Edge->m_dLength };
		static double adAngles[] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
		pName1Face->CreatePolygon( apszNames, adLengths, adAngles, 3 );
		// Set north pole to tip of pyramid
		pName1Face->m_fOrientation = 90.0;
		pName1Face->SetBaseEdgeName( apszNames[2] );

		adLengths[0] = dBaseEdgeEnd0Length;
		adLengths[1] = dCommonEdge12Length;
		adLengths[2] = pName2Edge->m_dLength;
		pName2Face->CreatePolygon( apszNames, adLengths, adAngles, 3 );
		// Set north pole to tip of pyramid
		pName2Face->m_fOrientation = 90.0;
		pName2Face->SetBaseEdgeName( apszNames[2] );

		adLengths[0] = dBaseEdgeEnd1Length;
		adLengths[1] = dBaseEdgeEnd0Length;
		adLengths[2] = pName3Edge->m_dLength;
		pName3Face->CreatePolygon( apszNames, adLengths, adAngles, 3 );
		// Set north pole to tip of pyramid
		pName3Face->m_fOrientation = 90.0;
		pName3Face->SetBaseEdgeName( apszNames[2] );

		// Keep outward join along base edge but insert 2 new faces
		// on the other two.
		// Note the order dependency here: connect to what
		// was previously the base edge leg n outer join,
		// then connect to base edge leg n.
		CEdge* pName1Face_c = pName1Face->GetEdge( "c" );
		ASSERT( pName1Face_c != NULL );
		// Set concave attribute.  See FIXME notes above
		// re: assuming concavity on all stellations!
		pName1Face_c->m_bConcave = true;
		// Also set text for FixupSymbolic()
		pName1Face_c->JoinOutward( pName1Edge->m_pOutwardConnectedEdge );

		CEdge* pName1Face_b = pName1Face->GetEdge( "b" );
		ASSERT( pName1Face_b != NULL );
		pName1Face_b->JoinOutward( pName3Face->GetEdge( "a" ) );

		CEdge* pName2Face_c = pName2Face->GetEdge( "c" );
		ASSERT( pName2Face_c != NULL );
		// Set concave attribute.  See FIXME notes above
		// re: assuming concavity on all stellations!
		pName2Face_c->m_bConcave = true;
		pName2Face_c->JoinOutward( pName2Edge->m_pOutwardConnectedEdge );
		CEdge* pName2Face_a = pName2Face->GetEdge( "a" );
		ASSERT( pName2Face_a != NULL );
		pName2Face_a->JoinOutward( pName3Face->GetEdge( "b" ) );

		CEdge* pName3Face_c = pName3Face->GetEdge( "c" );
		ASSERT( pName3Face_c != NULL );
		pName3Face_c->m_bConcave = true;
		pName3Face_c->JoinOutward( pName3Edge->m_pOutwardConnectedEdge );

		// Join two new faces along common edge
		CEdge* pName1Face_a = pName1Face->GetEdge( "a" );
		ASSERT( pName1Face_a != NULL );
		CEdge* pName2Face_b = pName2Face->GetEdge( "b" );
		ASSERT( pName2Face_b != NULL );
		pName1Face_a->JoinOutward( pName2Face_b );

		} // triangular base

		// Make them all part of the new group
		pFace->AddToGroup( szFaceGroupName );
		// Newly created faces take the entire group collection from original face
		pName1Face->CopyGroups( pFace );
		pName2Face->CopyGroups( pFace );
		pName3Face->CopyGroups( pFace );
		if (IsSquare)
		{
			pName4Face->CopyGroups( pFace );
		}


		// Add new faces to our map
		m_mapFaces.SetAt( pName1Face->m_szFaceName, pName1Face );
		m_mapFaces.SetAt( pName2Face->m_szFaceName, pName2Face );
		m_mapFaces.SetAt( pName3Face->m_szFaceName, pName3Face );
		if (IsSquare)
		{
			m_mapFaces.SetAt( pName4Face->m_szFaceName, pName4Face );
			nAdded++;
		}
		nAdded += 2;

		// Destroy original face, which has already been freed from map
		delete pFace;
		pFace = NULL;

		return nAdded;

} // CShape::StellateFace()

void CShape::SetValid(bool bValid)
{
	m_bValid = bValid;
}

CString CShape::MakeUniqueFaceName(LPCTSTR lpBase, LPCTSTR lpHint)
{
	CString sz;
	sz = lpBase;
	sz += lpHint;
	CFace* pf;
	int nPass = 0;
	while (m_mapFaces.Lookup( sz, pf ))
	{
		CDbg::Out( "Duplicate name attempt %s\n", (LPCTSTR)sz );
		// Replicate leftmost (usually face) on first pass
		if (nPass == 0)
		{
			sz += sz.Left(1);
		} // First pass
		else if (nPass == 1)
		{
			// Rather than again replicating what was originally leftmost,
			// replicate what was originally rightmost
			sz += sz.Mid(sz.GetLength()-2,1);
		} // Second pass
		else
		{
			// From here on out, we have a speech impedimenttttttt
			sz += sz.Right(1);
		} // All others
		nPass++;
	} // Finding a dupe
	return sz;
}

// Helper functions for Geodesic()
static int StartFace(int Row) { return 1+(Row-1)*(Row-1); }
static int EndFace(int Row) { return Row*Row; }
static int EndColumn(int Row) { return 1+EndFace(Row)-StartFace(Row); }
static int FaceNum(int Row, int Column) { return StartFace(Row)+Column-1; }
static int RowComplement(int Row, int Rows) { return 1+Rows-Row; }
static int ColumnComplement(int Row, int Column) { return EndColumn(Row)-Column+1; }
static int Row(int Face, int Rows)
{
	int nRow = 1;
	for ( ; Face > nRow*nRow; nRow++) ;
	return nRow;
} // Row
static int Column(int Face, int Rows)
{
	int PrevRow = Row( Face, Rows ) - 1;
	return Face - PrevRow * PrevRow;
} // Column

// Return face index for the face in another face group
// e.g. for a 2v breakdown a-a, 2 in this face connects to 1 in the other
// for a-b, 2 connects to 4; for a-c, 2 connects to 2; etc.
static int OtherFace(int Row, int Column, int Rows, unsigned char cFromEdge, unsigned char cToEdge)
{
#define MAKESCALAR(from,to)	((((unsigned char)from)<<8)|(unsigned char)to)
	switch (MAKESCALAR(cFromEdge,cToEdge))
	{
	case MAKESCALAR('a','a'):
		return StartFace( RowComplement( Row, Rows ) );
	case MAKESCALAR('a','b'):
		return EndFace( Row );
	case MAKESCALAR('a','c'):
		return FaceNum( Rows, 1+(RowComplement(Row,Rows)-1)*2 );
	case MAKESCALAR('b','a'):
		return StartFace( Row );
	case MAKESCALAR('b','b'):
		return EndFace( RowComplement( Row, Rows ) );
	case MAKESCALAR('b','c'):
		return FaceNum( Rows, 1+(Row-1)*2 );
	case MAKESCALAR('c','a'):
		return StartFace( RowComplement( (Column+1)/2, Rows ) );
	case MAKESCALAR('c','b'):
		return EndFace( (Column+1)/2 );
	case MAKESCALAR('c','c'):
		return FaceNum( Row, ColumnComplement( Row, Column ) );
	}
	ASSERT( true );
	return 0;
} // OtherFace()

// We need to define a winding order for each type of polyhedron.
// For the purpose of defining windings, assume we rename all faces
// A-T and all edges a-c.
// Every face gets three characters: navigation and up to two joins,
// plus a separator for readability.
	static const char *pszWindingEdge[] = {
		/*f=4*/				/*f=6*/						/*f=8*/								/*f=20*/
		/*A   B   C   D*/	/*A   B   C   D   E   F*/	/*A   B   C   D   E   F   G   H*/	/*A   B   C   D   E   F   G   H   I   J   K   L   M   N   O   P   Q   R   S   T*/
		"bbc|bbc|cbc|   |",	"bbc|bbc|cbc|bb |bb | b |",	"bbc|bbc|bbc|cbc|bb |bb |bb | b |",	"bbc|bbc|bbc|bbc|cbc|aa |bbc|aa |bbc|aa |bbc|aa |bbc|aa |cbc|bb |bb |bb |bb | b |"
	};
	static const char *pszWindingFace[] = {
		" BD| CD| AD|   |",	" BF| CE| AD| E | F | D |",	" BH| CG| DF| AE| F | G | H | E |",	" BH| CJ| DL| EN| AF| G | HT| I | JS| K | LR| M | NQ| O | FP| Q | R | S | T | P |"
	};
	static const char *pszWindingFaceEdge[] = {
		"aaa|aab|aac|c  |",	"aac|aac|aac|ca |aa |aa |",	"aac|aac|aac|aac|ca |aa |aa |aa |",	"aac|aac|aac|aac|aac|ca |abc|ba |abc|ba |abc|ba |abc|ba |abc|ca |aa |aa |aa |aa |"
		// In this last row, the first column in each group is the starting edge for navigation.  It is significant when != a
	};

	// Use this to build a list in winding order
#define GWNAVFROM(winding)				((unsigned char)pszWindingEdge[nWindex][winding*4+0])
#define	GWNAVARRIVESON(winding)			((unsigned char)pszWindingFaceEdge[nWindex][winding*4+0])

#define	GWJOINFROM(winding,offset)		((unsigned char)pszWindingEdge[nWindex][winding*4+1+offset])
#define GWJOINTOFACE(winding,offset)	((unsigned char)pszWindingFace[nWindex][winding*4+1+offset])
#define	GWJOINTOEDGE(winding,offset)	((unsigned char)pszWindingFaceEdge[nWindex][winding*4+1+offset])

#define	GWJOINTOFACEINDEX(winding,offset,letter)	(GWJOINTOFACE(winding,offset)-(unsigned char)'A')
#define GWJOINTOEDGEINDEX(winding,offset,letter)	(GWJOINTOEDGE(winding,offset)-(unsigned char)'a')
#define	GWNAVARRIVALINDEX(winding)		(GWNAVARRIVESON(winding)-(unsigned char)'a')

#define MAXSUPPORTEDFACES	20

	// Base chords for various regular polyhedra indexed by (nfaces-1)
	static const double adBaseChords[] = {
		0.0,	// 1 = invalid
		0.0,	// 2 = invalid
		0.0,	// 3 = invalid
		1.6329909385929866032346536715315,	// tetrahedron
		0.0,	// 5 = invalid
		1.4,	// sextahedron
		0.0,	// 7 = invalid
		1.3,	// octahedron
		0.0,	// 9 = invalid
		0.0,	// 10 = invalid
		0.0,	// 11 = invalid
		0.0,	// 12 = invalid
		0.0,	// 13 = invalid
		0.0,	// 14 = invalid
		0.0,	// 15 = invalid
		0.0,	// 16 = invalid
		0.0,	// 17 = invalid
		0.0,	// 18 = invalid
		0.0,	// 19 = invalid
		1.051462224 // icosahedron
	};

	// Chord factor indices going in face order
	// Indexed by (freq-1)^2+nface-1
	static const int anChordIndex[1*1+2*2+3*3+4*4][3] = {
		// No data for 1v
		{ -1, -1, -1 },
		// 2^2 faces for 2v
		{ 0, 0, 1 },
		{ 0, 1, 0 },
		{ 1, 1, 1 },
		{ 1, 0, 0 },
		// 3^2 faces for 3v
		{ 0, 0, 1 },
		{ 1, 2, 2 },
		{ 2, 2, 1 },

		{ 2, 1, 2 },
		{ 0, 1, 0 },
		{ 2, 1, 2 },

		{ 2, 2, 1 },
		{ 1, 2, 2 },
		{ 1, 0, 0 },
		// 4^2 faces for 4v
		{ 0, 0, 2 },
		{ 3, 1, 4 },
		{ 1, 1, 2 },
		{ 1, 3, 4 },

		{ 3, 4, 1 },
		{ 5, 4, 4 },
		{ 5, 5, 5 },
		{ 4, 5, 4 },

		{ 4, 3, 1 },
		{ 0, 2, 0 },
		{ 1, 2, 1 },
		{ 1, 4, 3 },

		{ 4, 4, 5 },
		{ 4, 1, 3 },
		{ 2, 1, 1 },
		{ 2, 0, 0 },
		// 5^2 faces for 5v
		// 6^2 faces for 6v
	};
	// Starting offsets into anChordIndex
	static const int anChordIndexOffset[] = {
		0,			//1v - not valid
		1*1,		//2v
		1*1 + 2*2,	//3v
		1*1 + 2*2 + 3*3,	//4v
		1*1 + 2*2 + 3*3 + 4*4,	// 5v
		1*1 + 2*2 + 3*3 + 4*4 + 5*5	// 6v
	};

	// Edge names to use
	static const char *apszNames[] = { "a", "b", "c" };


// Generate geodesic with specified breakdown factor (2, 3, 4, etc)
// Currently only works with icosahedron (20), octahedron (8), hexahedron (6)
// and tetrahedron (4 faces).
int CShape::Geodesic(int Breakdown, bool bStellateCenter )
{

	// Izit a supported breakdown?
	if (Breakdown < 2 || Breakdown > 4)
	{
		return 0;
	} // Not supported

	// Izit a supported number of faces?
	// Get an index into pszWindingEdge[], pszWindingFace[], and pszWindingFaceEdge[]
	// or reject the number of faces.
	int nWindex;
	int nTotalFaces = GetNumFaces();
	switch (nTotalFaces)
	{
	case 4:
		nWindex = 0;
		break;
	case 6:
		nWindex = 1;
		break;
	case 8:
		nWindex = 2;
		break;
	case 20:
		nWindex = 3;
		break;
	default:
		return 0;
	}

	m_IsRegular = -1; // Indeterminate

	ASSERT( nTotalFaces <= MAXSUPPORTEDFACES );

	// Invalid for drawing
	m_bValid = false;

	// Ha ha ha, well the comments below were a nice try but no cigar.
	// Sanity check data against entry / edit errors
	ASSERT( nWindex < sizeof(pszWindingEdge)/sizeof(pszWindingEdge[0]) );
	ASSERT( sizeof(pszWindingEdge)==sizeof(pszWindingFace) );
	ASSERT( sizeof(pszWindingEdge)==sizeof(pszWindingFaceEdge) );
	ASSERT( strlen( pszWindingEdge[nWindex] ) == (size_t)nTotalFaces * 4 );
	ASSERT( strlen( pszWindingFace[nWindex] ) == (size_t)nTotalFaces * 4 );
	ASSERT( strlen( pszWindingFaceEdge[nWindex] ) == (size_t)nTotalFaces * 4 );

	// Construct list of faces and face edges in winding order
	CFace* aWindingFace[MAXSUPPORTEDFACES];
	CEdge* aWindingFaceEdge[MAXSUPPORTEDFACES][3];

	int nWindFace;
	int nWindEdge;
	// Start with any face
	CFace* pWindFace = GetFirstFace();
	ASSERT( pWindFace != NULL );
	CEdge* pWindEdge = pWindFace->GetFirstEdge();
	CDbg::Out( "\n--- Winding order starting with face %s ---\n",
		(LPCTSTR)pWindFace->m_szFaceName );
	CArray<CFace*,CFace*> aCenterFace;
	for (nWindFace = 0; nWindFace < nTotalFaces && pWindFace != NULL && pWindEdge != NULL; nWindFace++)
	{
		ASSERT( pWindFace != NULL );
		aWindingFace[nWindFace] = pWindFace;
		unsigned char cNav = GWNAVFROM(nWindFace);
		int nArrivalIndex = GWNAVARRIVALINDEX(nWindFace);
		for (nWindEdge = 0; nWindEdge < 3; nWindEdge++)
		{
			ASSERT( pWindEdge != NULL );
			// Starting pWindEdge may not be side a
			aWindingFaceEdge[nWindFace][(nWindEdge+nArrivalIndex)%3] = pWindEdge;
			pWindEdge = pWindEdge->GetClockwiseNext();
		}
		CDbg::Out( "\t%3d face %s edges %s,%s,%s\n",
			nWindFace, (LPCTSTR)pWindFace->m_szFaceName,
			(LPCTSTR)aWindingFaceEdge[nWindFace][0]->GetName(),
			(LPCTSTR)aWindingFaceEdge[nWindFace][1]->GetName(),
			(LPCTSTR)aWindingFaceEdge[nWindFace][2]->GetName() );
		// Have we reached the end of navigation?
		if (cNav == ' ')
		{
			// This assert must not fail, otherwise we'd not have everything in
			// our winding.
			ASSERT( nWindFace == nTotalFaces-1 );
			break;
		}
		// Get next face
		ASSERT( cNav >= 'a' && cNav <= 'c' );
		pWindEdge = aWindingFaceEdge[nWindFace][cNav-'a'];
		if (pWindEdge->m_pOutwardConnectedEdge == NULL)
		{
			AfxMessageBox( "Sorry, cannot do a geodesic - all edges must be connected!" );
			m_bValid = true;
			return 0;
		}
		pWindEdge = pWindEdge->m_pOutwardConnectedEdge;
		pWindFace = pWindEdge->m_pFace;
	}
	CDbg::Out( "======\n\n" );

	// Take each face and make it the top triangle in a new mesh
	// Every face in the new mesh (including the present face) is
	// getting renamed.  Going left to right from top to bottom
	// (page reading order) we number faces in the mesh from 1 to n
	// where n is the breakdown factor squared.

	// There are a number of interesting numeric properties
	// we take advantage of.  A few more basic assumptions:
	// 1. We always rename sides a, b, c clockwise starting
	//    from -> a/\   b
	//          a/____\ b
	//              c
	//    as if the face was in the orientation shown above.
	//    This means that ALL face:edge joins are face1:a - face2:a,
	//    face1:b - face2:b, face1:c - face2:c
	// 2. Rows are numbered from top downward as follows:
	//    row 1           1
	//    row 2        2  3  4
	//    row 3     5  6  7  8  9
	//    row 4  10 11 12 13 14 15 16
	// 3. Columns are numbered {n,n+1,n+2,...} where n is the starting
	//    number for each row
	// 4. Starting numbers for rows are (row-1)^2 + 1
	// 5. Ending numbers for rows are row^2
	// 6. For row 1 ... row Breakdown-1, odd-numbered columns have "up"
	//    orientation and join downward with row+1, column+1
	// 7. Column numbers for start and end of each row are always odd
	// 8. For row Breakdown, odd-numbered columns join downward with
	//    row Breakdown, column (ncolumns(row) - column + 1) in "down" face
	// 9. For column 1, join left with "left" face column
	//    ncolumns(Breakdown-row+1),row Breakdown-row+1
	// A. For column {1..ncolumns(row)-1}, always join right with
	//    column+1 in same row
	// B. For column ncolumns(row), join right with "right" face, 
	//    column 1, row Breakdown-row+1
	// C. All joins are reflexive, i.e. join left with column + 1 means
	//    column + 1 joins right with us automatically

	// Chord factors
	// Indexed by [Breakdown-2][a,b,c,etc]
	static const double adChords[][6] = {
		{	0.546533058,	0.62573786,														0, 0, 0, 0	},
		{	0.3486,			0.4035,			0.4124,											0, 0, 0	},
		//{	0.366958816,	0.403828246,	0.42406256,										0, 0, 0	},
		{	0.24079,		0.28011,		0.28079,		0.28397,		0.29755,		0.30901 },
		//{	0.275904484,	0.298088063,	0.31286893,		0.321244071,					0, 0 },
		{	0.220977648,	0.235728588,	0.246576912,	0.25393573,		0.258184299,	0 },
		{	0.184263108,	0.194761968,	0.202961917,	0.209056927,	0.2132469,		0.21569298 },
	};
	int nChordBaseIdx = Breakdown - 2;

	// Get base chord for polyhedron
	double adBaseChord = adBaseChords[nTotalFaces - 1];

	// Edge lengths to be set up from chord factors
	double adLengths[] = { 1.0, 1.0, 1.0 };

	// Save all current face:edge joins in symbolic format
	CutAllFaceJoins( true );

	// Calculate radius.  We'll keep edge lengths more or less
	// constant, which means radius *= Breakdown
	CEdge* pe = aWindingFaceEdge[0][0];
	double dRadius = (pe->m_dLength * Breakdown ) / adBaseChord;

	// Number of rows in mesh is Breakdown
	int Rows = Breakdown;

	// Number of faces is Breakdown^2
	int nNewFaceCount = Breakdown * Breakdown;

	// Go through all faces
	int nFace;
	int nTotalCreated = 0;
	for (nFace = 0; nFace < nTotalFaces; nFace++)
	{
		CFace* pFace = aWindingFace[nFace];

		CString szNewFaceName;
		CString szBaseFaceName;
		szBaseFaceName = pFace->m_szFaceName;
		CString szFaceGroupName;
		szFaceGroupName.Format( "%s_g%d", (LPCTSTR)pFace->m_szFaceName, Breakdown );
		// Add to existing list - should not be possible since geodesic cannot be done after stellation...
		pFace->AddToGroup( szFaceGroupName );
		// Save complete group list
		CString szGroupList = pFace->GetGroupText();

		// Remove old face from map
		this->m_mapFaces.RemoveKey( pFace->m_szFaceName );

		// Get up to two joins
		unsigned char cFrom[] = { GWJOINFROM( nFace, 0 ), GWJOINFROM( nFace, 1 ), 0 };
		unsigned char cJoinFace[] = { GWJOINTOFACE( nFace, 0 ), GWJOINTOFACE( nFace, 1 ), 0 };
		unsigned char cJoinEdge[] = { GWJOINTOEDGE( nFace, 0 ), GWJOINTOEDGE( nFace, 1 ), 0 };

		// From join=a affects start column on each row
		// From join=b affects end column on each row
		// From join=c affects last row
		// All nine from-to combinations (a-a, a-b, a-c, b-a, b-b, b-c, c-a, c-b, c-c)
		// follow different dispatch rules.

		// Join names for other faces internal to the original face
		CString szDownFaceName; // Downward join
		CString szRightFaceName; // Right join

		// Get basenames of joined faces
		CString szJoinFaceName;
		CString szFaceBase[2];
		int nJoin;
		int nJoinCount = 0;
		for (nJoin = 0; nJoin < 2; nJoin++)
		{
			if (cJoinFace[nJoin] != ' ')
			{
				//int nEdgeIndex = GWJOINTOEDGEINDEX( nFace, nJoin, cFrom[nJoin] );
				int nFaceIndex = GWJOINTOFACEINDEX( nFace, nJoin, cJoinFace[nJoin] );
				szFaceBase[nJoin] = aWindingFace[nFaceIndex]->m_szFaceName;
				nJoinCount++;
			}
		}

		// These are the left right and down faces relative to us -
		// whether we are connected left with someone's
		// a, b or c edge depends on the polyhedron...

		CDbg::Out( "Geo(%d): exploding face %s, connecting %d (%s -> %s:%s)\n",
			Breakdown, (LPCTSTR)szBaseFaceName,
			nJoinCount, cFrom, cJoinFace, cJoinEdge  );

		// Delete original face later
		//delete pFace;
		pFace = NULL;

		// Construct new faces.  Face 1 is already built
		int nNewFace;
		int nNewRow;
		int nNewCol;
		CFace* pCenterFace = NULL;
		for (nNewFace = 1; nNewFace <= nNewFaceCount; nNewFace++)
		{
			nNewRow = Row( nNewFace, Rows );
			nNewCol = Column( nNewFace, Rows );

			// Setup edge lengths
			int nNewEdge;
			for (nNewEdge = 0; nNewEdge < 3; nNewEdge++)
			{
				adLengths[nNewEdge] = dRadius
					* adChords
						[nChordBaseIdx]
							[anChordIndex
								[anChordIndexOffset
									[Breakdown-1
									]+nNewFace-1
								]
								[nNewEdge]
							];
			}

			// Construct new face name
			szNewFaceName.Format( "%sg%d", (LPCTSTR)szBaseFaceName, nNewFace );

			CDbg::Out( "new face %s uses lengths a=%f, b=%f, c=%f\n", (LPCTSTR)szNewFaceName,
				adLengths[0], adLengths[1], adLengths[2] );

			// Construct face object
			pFace = new CFace( this, szNewFaceName );
			// Stellate all even column faces (3, 6, 8, 11, 13, 15, etc)
			if (bStellateCenter && nNewCol % 2 == 0)
			{
				pCenterFace = pFace;
				aCenterFace.Add( pCenterFace );
			}
			pFace->CreatePolygon( apszNames, adLengths, adAngles, 3 );

			// Join outward edges according to rules
			CEdge* pea = pFace->GetEdge( "a" );
			CEdge* peb = pFace->GetEdge( "b" );
			CEdge* pec = pFace->GetEdge( "c" );
			ASSERT( pea != NULL && peb != NULL && pec != NULL );

			// Set up endpoints for symbolic join
			pea->m_aszEndPoints[0] = "c.a";
			pea->m_aszEndPoints[1] = "a.b";
			peb->m_aszEndPoints[0] = "a.b";
			peb->m_aszEndPoints[1] = "b.c";
			pec->m_aszEndPoints[0] = "b.c";
			pec->m_aszEndPoints[1] = "c.a";

			// Copy groups from original face
			pFace->SetGroupText( szGroupList );

			// Display text
			//pFace->SetBody( "$face" );

	// 9. For column 1, join left with "left" face column
	//    1, row Breakdown-row+1
			const char *pszFrom;
			if (nNewCol == 1 && (pszFrom = strchr( (const char*)&cFrom[0], 'a' )))
			{
				nJoin = (int)(pszFrom - (const char*)&cFrom[0]);
				int nOther = OtherFace( nNewRow, nNewCol, Rows, cFrom[nJoin], cJoinEdge[nJoin] );
				szJoinFaceName.Format( "%sg%d", (LPCTSTR)szFaceBase[nJoin], nOther );
				pea->m_szOutwardConnectedEdge.Format( "%s:%c",
					(LPCTSTR)szJoinFaceName,
					cJoinEdge[nJoin] );
				CDbg::Out( "Geo(%d) %s -> %s\n", Breakdown,
					(LPCTSTR)pea->GetFQName(),
					(LPCTSTR)pea->m_szOutwardConnectedEdge );
			} // Use left face

			// Select edge to join right
			CEdge* peRight = peb;
			char cRight = 'b';
			// Even-numbered column
			// This is mutually exclusive of previous case because 1 is not even...
			if (nNewCol % 2 == 0)
			{
				peRight = pea;
				cRight = 'a';
			}
			// Odd-numbered column
			else
			{
				// Downward joins always use c
	// 6. For row 1 ... row Breakdown-1, odd-numbered columns have "up"
	//    orientation and join downward with row+1, column+1
				if (nNewRow < Rows)
				{
					szDownFaceName.Format( "%sg%d", (LPCTSTR)szBaseFaceName, FaceNum( nNewRow+1, nNewCol+1 ) );
					pec->m_szOutwardConnectedEdge.Format( "%s:c", (LPCTSTR)szDownFaceName );
					CDbg::Out( "Geo(%d) %s -> %s\n", Breakdown, (LPCTSTR)pec->GetFQName(), (LPCTSTR)pec->m_szOutwardConnectedEdge );
				}
	// 8. For row Breakdown, odd-numbered columns join downward with
	//    row Breakdown, column (ncolumns(row) - column + 1) in "down" face
				else if (pszFrom = strchr( (const char*)&cFrom[0], 'c' ))
				{
					nJoin = (int)(pszFrom - (const char*)&cFrom[0]);
					int nOther = OtherFace( nNewRow, nNewCol, Rows, cFrom[nJoin], cJoinEdge[nJoin] );
					szJoinFaceName.Format( "%sg%d", (LPCTSTR)szFaceBase[nJoin], nOther );
					pec->m_szOutwardConnectedEdge.Format( "%s:%c",
						(LPCTSTR)szJoinFaceName,
						cJoinEdge[nJoin] );
					CDbg::Out( "Geo(%d) %s -> %s\n", Breakdown,
						(LPCTSTR)pec->GetFQName(),
						(LPCTSTR)pec->m_szOutwardConnectedEdge );
				}
			}

	// A. For column {1..ncolumns(row)-1}, always join right with
	//    column+1 in same row
			if (nNewFace < EndFace(nNewRow))
			{
				szRightFaceName.Format( "%sg%d", (LPCTSTR)szBaseFaceName, nNewFace+1 );
				peRight->m_szOutwardConnectedEdge.Format( "%s:%c", (LPCTSTR)szRightFaceName, cRight );
				CDbg::Out( "Geo(%d) %s -> %s\n", Breakdown, (LPCTSTR)peRight->GetFQName(), (LPCTSTR)peRight->m_szOutwardConnectedEdge );
			} // Join right within this face
	// B. For column ncolumns(row), join right with "right" face, 
	//    column end, row Breakdown-row+1
			else if (pszFrom = strchr( (const char*)&cFrom[0], 'b' ))
			{
				nJoin = (int)(pszFrom - (const char*)&cFrom[0]);
				int nOther = OtherFace( nNewRow, nNewCol, Rows, cFrom[nJoin], cJoinEdge[nJoin] );
				szJoinFaceName.Format( "%sg%d", (LPCTSTR)szFaceBase[nJoin], nOther );
				peb->m_szOutwardConnectedEdge.Format( "%s:%c",
					(LPCTSTR)szJoinFaceName,
					cJoinEdge[nJoin] );
				CDbg::Out( "Geo(%d) %s -> %s\n", Breakdown,
					(LPCTSTR)peb->GetFQName(),
					(LPCTSTR)peb->m_szOutwardConnectedEdge );
			} // Join to right face

			// Add to map
			m_mapFaces.SetAt( szNewFaceName, pFace );
			nTotalCreated++;

		} // for all faces being added

	} // for all original faces

	// Delete original faces
	for (nFace = 0; nFace < nTotalFaces; nFace++)
	{
		ASSERT( aWindingFace[nFace] != NULL );
		delete aWindingFace[nFace];
		aWindingFace[nFace] = NULL;
	}

	// Fixup all the edge references we've changed
	FixupSymbolic();

	// If specified, stellate center faces
	if (bStellateCenter && aCenterFace.GetSize() > 0)
	{
		size_t n;
		size_t nLimit = aCenterFace.GetSize();
		for (n = 0; n < nLimit; n++)
		{
			nTotalCreated += StellateFace( aCenterFace[n], 1.0 );
		}
	} // Stellating face

	// Create map of unified vertices
	// Since we've destroyed faces we need to do it from scratch
	RebuildVertexMaps();

	// Shape is valid for drawing
	this->m_bValid = true;

	// Number of total faces
	return nTotalCreated;
}

// Stellate shape a la Kepler's Stella Octangula.  Return number of faces added.
int
CShape::KStellate( double dRatio )
{
	int Breakdown = 2;
	bool bStellate = true;

	// Izit a supported number of faces?
	// Get an index into pszWindingEdge[], pszWindingFace[], and pszWindingFaceEdge[]
	// or reject the number of faces.
	int nWindex;
	int nTotalFaces = GetNumFaces();
	switch (nTotalFaces)
	{
	case 4:
		nWindex = 0;
		break;
	case 6:
		nWindex = 1;
		break;
	case 8:
		nWindex = 2;
		break;
	case 20:
		nWindex = 3;
		break;
	default:
		return 0;
	}

	// Get base chord for polyhedron
	double adBaseChord = adBaseChords[nTotalFaces - 1];
	m_IsRegular = -1; // Indeterminate

	ASSERT( nTotalFaces <= MAXSUPPORTEDFACES );

	// Invalid for drawing
	m_bValid = false;

	// Sanity check data against entry / edit errors
	ASSERT( nWindex < sizeof(pszWindingEdge)/sizeof(pszWindingEdge[0]) );
	ASSERT( sizeof(pszWindingEdge)==sizeof(pszWindingFace) );
	ASSERT( sizeof(pszWindingEdge)==sizeof(pszWindingFaceEdge) );
	ASSERT( strlen( pszWindingEdge[nWindex] ) == (size_t)nTotalFaces * 4 );
	ASSERT( strlen( pszWindingFace[nWindex] ) == (size_t)nTotalFaces * 4 );
	ASSERT( strlen( pszWindingFaceEdge[nWindex] ) == (size_t)nTotalFaces * 4 );

	// Construct list of faces and face edges in winding order
	CFace* aWindingFace[MAXSUPPORTEDFACES];
	CEdge* aWindingFaceEdge[MAXSUPPORTEDFACES][3];

	int nWindFace;
	int nWindEdge;
	// Start with any face
	CFace* pWindFace = GetFirstFace();
	ASSERT( pWindFace != NULL );
	CEdge* pWindEdge = pWindFace->GetFirstEdge();
	CDbg::Out( "\n--- Winding order starting with face %s ---\n",
		(LPCTSTR)pWindFace->m_szFaceName );
	CArray<CFace*,CFace*> aCenterFace;
	for (nWindFace = 0; nWindFace < nTotalFaces && pWindFace != NULL && pWindEdge != NULL; nWindFace++)
	{
		ASSERT( pWindFace != NULL );
		aWindingFace[nWindFace] = pWindFace;
		unsigned char cNav = GWNAVFROM(nWindFace);
		int nArrivalIndex = GWNAVARRIVALINDEX(nWindFace);
		for (nWindEdge = 0; nWindEdge < 3; nWindEdge++)
		{
			ASSERT( pWindEdge != NULL );
			// Starting pWindEdge may not be side a
			aWindingFaceEdge[nWindFace][(nWindEdge+nArrivalIndex)%3] = pWindEdge;
			pWindEdge = pWindEdge->GetClockwiseNext();
		}
		CDbg::Out( "\t%3d face %s edges %s,%s,%s\n",
			nWindFace, (LPCTSTR)pWindFace->m_szFaceName,
			(LPCTSTR)aWindingFaceEdge[nWindFace][0]->GetName(),
			(LPCTSTR)aWindingFaceEdge[nWindFace][1]->GetName(),
			(LPCTSTR)aWindingFaceEdge[nWindFace][2]->GetName() );
		// Have we reached the end of navigation?
		if (cNav == ' ')
		{
			// This assert must not fail, otherwise we'd not have everything in
			// our winding.
			ASSERT( nWindFace == nTotalFaces-1 );
			break;
		}
		// Get next face
		ASSERT( cNav >= 'a' && cNav <= 'c' );
		pWindEdge = aWindingFaceEdge[nWindFace][cNav-'a'];
		if (pWindEdge->m_pOutwardConnectedEdge == NULL)
		{
			AfxMessageBox( "Sorry, cannot do a k-stellate - all edges must be connected!" );
			m_bValid = true;
			return 0;
		}
		pWindEdge = pWindEdge->m_pOutwardConnectedEdge;
		pWindFace = pWindEdge->m_pFace;
	}
	CDbg::Out( "======\n\n" );

	// Take each face and make it the top triangle in a new mesh
	// Every face in the new mesh (including the present face) is
	// getting renamed.  Going left to right from top to bottom
	// (page reading order) we number faces in the mesh from 1 to n
	// where n is the breakdown factor squared.

	// There are a number of interesting numeric properties
	// we take advantage of.  A few more basic assumptions:
	// 1. We always rename sides a, b, c clockwise starting
	//    from -> a/\   b
	//          a/____\ b
	//              c
	//    as if the face was in the orientation shown above.
	//    This means that ALL face:edge joins are face1:a - face2:a,
	//    face1:b - face2:b, face1:c - face2:c
	// 2. Rows are numbered from top downward as follows:
	//    row 1           1
	//    row 2        2  3  4
	// 3. Columns are numbered {n,n+1,n+2,...} where n is the starting
	//    number for each row
	// 4. Starting numbers for rows are (row-1)^2 + 1
	// 5. Ending numbers for rows are row^2
	// 6. For row 1 ... row Breakdown-1, odd-numbered columns have "up"
	//    orientation and join downward with row+1, column+1
	// 7. Column numbers for start and end of each row are always odd
	// 8. For row Breakdown, odd-numbered columns join downward with
	//    row Breakdown, column (ncolumns(row) - column + 1) in "down" face
	// 9. For column 1, join left with "left" face column
	//    ncolumns(Breakdown-row+1),row Breakdown-row+1
	// A. For column {1..ncolumns(row)-1}, always join right with
	//    column+1 in same row
	// B. For column ncolumns(row), join right with "right" face, 
	//    column 1, row Breakdown-row+1
	// C. All joins are reflexive, i.e. join left with column + 1 means
	//    column + 1 joins right with us automatically

	// Chord factors
	// Indexed by [Breakdown-2][a,b,c,etc]
	// Note that for this shape, chords are simply (original face length) / 2
	// This keeps the original planar configuration of the face.
	static const double adChords[][6] = {
		{	0.5,	0.5,														0, 0, 0, 0	},
	};
	int nChordBaseIdx = Breakdown - 2;

	// Edge lengths to be set up from chord factors
	double adLengths[] = { 1.0, 1.0, 1.0 };

	// Save all current face:edge joins in symbolic format
	CutAllFaceJoins( true );

	// Calculate radius.  We'll keep edge lengths more or less
	// constant, which means radius *= Breakdown
	CEdge* pe = aWindingFaceEdge[0][0];
	double dRadius = (pe->m_dLength * Breakdown ) / adBaseChord;

	// Number of rows in mesh is Breakdown
	int Rows = Breakdown;

	// Number of faces is Breakdown^2
	int nNewFaceCount = Breakdown * Breakdown;

	// Go through all faces
	int nFace;
	int nTotalCreated = 0;
	for (nFace = 0; nFace < nTotalFaces; nFace++)
	{
		CFace* pFace = aWindingFace[nFace];

		CString szNewFaceName;
		CString szBaseFaceName;
		szBaseFaceName = pFace->m_szFaceName;
		CString szFaceGroupName;
		szFaceGroupName.Format( "%s_k%d", (LPCTSTR)pFace->m_szFaceName, Breakdown );

		// Make it part of group
		pFace->AddToGroup( szFaceGroupName );

		// Save entire group list in original order
		CString szGroupList = pFace->GetGroupText();

		// Remove old face from map
		this->m_mapFaces.RemoveKey( pFace->m_szFaceName );

		// Get up to two joins
		unsigned char cFrom[] = { GWJOINFROM( nFace, 0 ), GWJOINFROM( nFace, 1 ), 0 };
		unsigned char cJoinFace[] = { GWJOINTOFACE( nFace, 0 ), GWJOINTOFACE( nFace, 1 ), 0 };
		unsigned char cJoinEdge[] = { GWJOINTOEDGE( nFace, 0 ), GWJOINTOEDGE( nFace, 1 ), 0 };

		// From join=a affects start column on each row
		// From join=b affects end column on each row
		// From join=c affects last row
		// All nine from-to combinations (a-a, a-b, a-c, b-a, b-b, b-c, c-a, c-b, c-c)
		// follow different dispatch rules.

		// Join names for other faces internal to the original face
		CString szDownFaceName; // Downward join
		CString szRightFaceName; // Right join

		// Get basenames of joined faces
		CString szJoinFaceName;
		CString szFaceBase[2];
		int nJoin;
		int nJoinCount = 0;
		for (nJoin = 0; nJoin < 2; nJoin++)
		{
			if (cJoinFace[nJoin] != ' ')
			{
				//int nEdgeIndex = GWJOINTOEDGEINDEX( nFace, nJoin, cFrom[nJoin] );
				int nFaceIndex = GWJOINTOFACEINDEX( nFace, nJoin, cJoinFace[nJoin] );
				szFaceBase[nJoin] = aWindingFace[nFaceIndex]->m_szFaceName;
				nJoinCount++;
			}
		}

		// These are the left right and down faces relative to us -
		// whether we are connected left with someone's
		// a, b or c edge depends on the polyhedron...

		CDbg::Out( "KStellate(%d): exploding face %s, connecting %d (%s -> %s:%s)\n",
			Breakdown, (LPCTSTR)szBaseFaceName,
			nJoinCount, cFrom, cJoinFace, cJoinEdge  );

		// Delete original face later
		//delete pFace;
		pFace = NULL;

		// Construct new faces.  Face 1 is already built
		int nNewFace;
		int nNewRow;
		int nNewCol;
		CFace* pCenterFace = NULL;
		for (nNewFace = 1; nNewFace <= nNewFaceCount; nNewFace++)
		{
			nNewRow = Row( nNewFace, Rows );
			nNewCol = Column( nNewFace, Rows );

			// Setup edge lengths
			int nNewEdge;
			for (nNewEdge = 0; nNewEdge < 3; nNewEdge++)
			{
				adLengths[nNewEdge] = dRadius
					* adChords
						[nChordBaseIdx]
							[anChordIndex
								[anChordIndexOffset
									[Breakdown-1
									]+nNewFace-1
								]
								[nNewEdge]
							];
			}

			// Construct new face name
			static const char szNewFaceFmt[] = "%sk%d";
			szNewFaceName.Format( szNewFaceFmt, (LPCTSTR)szBaseFaceName, nNewFace );

			CDbg::Out( "new face %s uses lengths a=%f, b=%f, c=%f\n", (LPCTSTR)szNewFaceName,
				adLengths[0], adLengths[1], adLengths[2] );

			// Construct face object
			pFace = new CFace( this, szNewFaceName );
			// Stellate all even column faces (3, 6, 8, 11, 13, 15, etc)
			if (bStellate && nNewCol % 2 == 0)
			{
				pCenterFace = pFace;
				aCenterFace.Add( pCenterFace );
			}
			pFace->CreatePolygon( apszNames, adLengths, adAngles, 3 );

			// Join outward edges according to rules
			CEdge* pea = pFace->GetEdge( "a" );
			CEdge* peb = pFace->GetEdge( "b" );
			CEdge* pec = pFace->GetEdge( "c" );
			ASSERT( pea != NULL && peb != NULL && pec != NULL );

			// Set up endpoints for symbolic join
			pea->m_aszEndPoints[0] = "c.a";
			pea->m_aszEndPoints[1] = "a.b";
			peb->m_aszEndPoints[0] = "a.b";
			peb->m_aszEndPoints[1] = "b.c";
			pec->m_aszEndPoints[0] = "b.c";
			pec->m_aszEndPoints[1] = "c.a";

			// Copy groups from original face
			pFace->SetGroupText( szGroupList );


	// 9. For column 1, join left with "left" face column
	//    1, row Breakdown-row+1
			const char *pszFrom;
			if (nNewCol == 1 && (pszFrom = strchr( (const char*)&cFrom[0], 'a' )))
			{
				nJoin = (int)(pszFrom - (const char*)&cFrom[0]);
				int nOther = OtherFace( nNewRow, nNewCol, Rows, cFrom[nJoin], cJoinEdge[nJoin] );
				szJoinFaceName.Format( szNewFaceFmt, (LPCTSTR)szFaceBase[nJoin], nOther );
				pea->m_szOutwardConnectedEdge.Format( "%s:%c",
					(LPCTSTR)szJoinFaceName,
					cJoinEdge[nJoin] );
				CDbg::Out( "KStell(%d) %s -> %s\n", Breakdown,
					(LPCTSTR)pea->GetFQName(),
					(LPCTSTR)pea->m_szOutwardConnectedEdge );
			} // Use left face

			// Select edge to join right
			CEdge* peRight = peb;
			char cRight = 'b';
			// Even-numbered column
			// This is mutually exclusive of previous case because 1 is not even...
			if (nNewCol % 2 == 0)
			{
				peRight = pea;
				cRight = 'a';
			}
			// Odd-numbered column
			else
			{
				// Downward joins always use c
	// 6. For row 1 ... row Breakdown-1, odd-numbered columns have "up"
	//    orientation and join downward with row+1, column+1
				if (nNewRow < Rows)
				{
					szDownFaceName.Format( szNewFaceFmt, (LPCTSTR)szBaseFaceName, FaceNum( nNewRow+1, nNewCol+1 ) );
					pec->m_szOutwardConnectedEdge.Format( "%s:c", (LPCTSTR)szDownFaceName );
					CDbg::Out( "KStell(%d) %s -> %s\n", Breakdown, (LPCTSTR)pec->GetFQName(), (LPCTSTR)pec->m_szOutwardConnectedEdge );
				}
	// 8. For row Breakdown, odd-numbered columns join downward with
	//    row Breakdown, column (ncolumns(row) - column + 1) in "down" face
				else if (pszFrom = strchr( (const char*)&cFrom[0], 'c' ))
				{
					nJoin = (int)(pszFrom - (const char*)&cFrom[0]);
					int nOther = OtherFace( nNewRow, nNewCol, Rows, cFrom[nJoin], cJoinEdge[nJoin] );
					szJoinFaceName.Format( szNewFaceFmt, (LPCTSTR)szFaceBase[nJoin], nOther );
					pec->m_szOutwardConnectedEdge.Format( "%s:%c",
						(LPCTSTR)szJoinFaceName,
						cJoinEdge[nJoin] );
					CDbg::Out( "KStell(%d) %s -> %s\n", Breakdown,
						(LPCTSTR)pec->GetFQName(),
						(LPCTSTR)pec->m_szOutwardConnectedEdge );
				}
			}

	// A. For column {1..ncolumns(row)-1}, always join right with
	//    column+1 in same row
			if (nNewFace < EndFace(nNewRow))
			{
				szRightFaceName.Format( szNewFaceFmt, (LPCTSTR)szBaseFaceName, nNewFace+1 );
				peRight->m_szOutwardConnectedEdge.Format( "%s:%c", (LPCTSTR)szRightFaceName, cRight );
				CDbg::Out( "KStell(%d) %s -> %s\n", Breakdown, (LPCTSTR)peRight->GetFQName(), (LPCTSTR)peRight->m_szOutwardConnectedEdge );
			} // Join right within this face
	// B. For column ncolumns(row), join right with "right" face, 
	//    column end, row Breakdown-row+1
			else if (pszFrom = strchr( (const char*)&cFrom[0], 'b' ))
			{
				nJoin = (int)(pszFrom - (const char*)&cFrom[0]);
				int nOther = OtherFace( nNewRow, nNewCol, Rows, cFrom[nJoin], cJoinEdge[nJoin] );
				szJoinFaceName.Format( szNewFaceFmt, (LPCTSTR)szFaceBase[nJoin], nOther );
				peb->m_szOutwardConnectedEdge.Format( "%s:%c",
					(LPCTSTR)szJoinFaceName,
					cJoinEdge[nJoin] );
				CDbg::Out( "KStell(%d) %s -> %s\n", Breakdown,
					(LPCTSTR)peb->GetFQName(),
					(LPCTSTR)peb->m_szOutwardConnectedEdge );
			} // Join to right face

			// Add to map
			m_mapFaces.SetAt( szNewFaceName, pFace );
			nTotalCreated++;

		} // for all faces being added

	} // for all original faces

	// Delete original faces
	for (nFace = 0; nFace < nTotalFaces; nFace++)
	{
		ASSERT( aWindingFace[nFace] != NULL );
		delete aWindingFace[nFace];
		aWindingFace[nFace] = NULL;
	}

	// Fixup all the edge references we've changed
	FixupSymbolic();

	// If specified, stellate center faces
	if (bStellate && aCenterFace.GetSize() > 0)
	{
		size_t n;
		size_t nLimit = aCenterFace.GetSize();
		for (n = 0; n < nLimit; n++)
		{
			nTotalCreated += StellateFace( aCenterFace[n], dRatio );
		}
	} // Stellating face

	// Create map of unified vertices
	// Since we've destroyed faces we need to do it from scratch
	RebuildVertexMaps();

	// Shape is valid for drawing
	this->m_bValid = true;

	// Number of total faces
	return nTotalCreated;

} // CShape::KStellate()

// Get minimum, maximum sides per face and min/max edge lengths.  Returns number of faces.
int CShape::GetStats(int &MinSides, int &MaxSides, double &dMinLength, double &dMaxLength)
{
	POSITION pos;
	CString szFaceName;
	CFace* pFace;
	MinSides =  99999999;
	MaxSides = -99999999;
	dMinLength = 99999999999;
	dMaxLength = -99999999999;
	int nCount = 0;
	for (pos = m_mapFaces.GetStartPosition(); pos != NULL; )
	{
		m_mapFaces.GetNextAssoc( pos, szFaceName, pFace );
		double dMin, dMax;
		int nSides = pFace->GetMinMaxEdge( dMin, dMax );
		if (nSides < MinSides)
		{
			MinSides = nSides;
		}
		if (nSides > MaxSides)
		{
			MaxSides = nSides;
		}
		if (dMin < dMinLength)
		{
			dMinLength = dMin;
		}
		if (dMax > dMaxLength)
		{
			dMaxLength = dMax;
		}
		nCount++;
	} // for all faces
	return nCount;
}

bool CShape::IsRegular()
{
	// m_IsRegular is a tri-state: if -1 it is indeterminate
	if (this->m_IsRegular < 0)
	{
		double dMin, dMax;
		GetStats( m_nMinSides, m_nMaxSides, dMin, dMax );
		m_IsRegular = (m_nMinSides == m_nMaxSides && dMin == dMax) ? 1 : 0;
	}
	return (m_IsRegular != 0);
}

bool CShape::IsTriangular()
{
	IsRegular();
	return (m_nMinSides == m_nMaxSides && m_nMinSides == 3);
}

// Get list of faces sorted in lexical order.  Return number in list
int
CShape::GetSortedFaces( CArray<CFace*,CFace*>& aFaces ) const
{
	// First populate it
	aFaces.RemoveAll();
	POSITION pos;
	CString szFaceName;
	CFace* pFace;
	for (pos = m_mapFaces.GetStartPosition(); pos != NULL; )
	{
		m_mapFaces.GetNextAssoc( pos, szFaceName, pFace );
		aFaces.Add( pFace );
	}

	int nTotalFaces = aFaces.GetSize();
	int nFace;
	bool bDone = false;
	while (!bDone)
	{
		bDone = true;
		for (nFace = 0; nFace < nTotalFaces-1; nFace++)
		{
			if (aFaces[nFace+1]->m_szFaceName < aFaces[nFace]->m_szFaceName)
			{
				bDone = false;
				CFace* p = aFaces[nFace];
				aFaces[nFace] = aFaces[nFace+1];
				aFaces[nFace+1] = p;
			} // Out of order
		}
	}

	return nTotalFaces;
} // CShape::GetSortedFaces()

// Set defaults for layout
void
CShape::SetLayoutDefaults( double dPageWidth, double dPageWidthOverHeight )
{
	m_dLayoutDefaultPageWidth = dPageWidth;
	m_dLayoutDefaultPageWidthOverHeight = dPageWidthOverHeight;
} // CShape::SetLayoutDefaults()

// Remove a face, unlink from all connections, and delete associated object
void
CShape::DeleteFace( LPCTSTR lpFaceName, bool bRedoMaps /*=true*/ )
{
	CFace *p = FindFace( lpFaceName );
	if (!p)
	{
		return;
	}
	// Detach outward joins
	p->DetachEdges();
	// Delete from face map
	m_mapFaces.RemoveKey( lpFaceName );
	// Delete the face itself
	delete p;
	// Vertex map must get rebuilt
	RebuildVertexMaps();
} // CShape::DeleteFace()

// Rebuild vertex maps from scratch
void CShape::RebuildVertexMaps()
{
	int n;
	int nSize = m_aMultiVertices.GetSize();
	for (n = 0; n < nSize; n++)
	{
		delete m_aMultiVertices[n];
		m_aMultiVertices[n] = NULL;
	}
	m_aMultiVertices.RemoveAll();
	m_mapUnifiedVertices.RemoveAll();
	MapVertices();
}

// Delete an entire group of faces
void
CShape::DeleteFaceGroup( LPCTSTR lpFaceGroupName )
{
	CArray<CFace*,CFace*> aFaces;
	POSITION pos;
	CString szFaceName;
	CFace* pFace;
	for (pos = m_mapFaces.GetStartPosition(); pos != NULL; )
	{
		m_mapFaces.GetNextAssoc( pos, szFaceName, pFace );
		if (pFace->IsPartOfGroup( lpFaceGroupName ))
		{
			aFaces.Add( pFace );
		}
	}
	int nFacesToDelete = aFaces.GetSize();
	int nFace;
	for (nFace = 0; nFace < nFacesToDelete; nFace++)
	{
		// Delete face, defer rebuilding vertex maps
		DeleteFace( aFaces[nFace]->m_szFaceName, false );
	}
	// Vertex map must get rebuilt
	RebuildVertexMaps();
} // CShape::DeleteFaceGroup()

// Load transform for pathnames (if file is not specified by absolute path, use m_szBaseDir
// to make it an absolute path)
CString
CShape::LoadTransformPath( LPCTSTR lpInputRelativePath )
{
	// Izit already absolute?
	if (strpbrk( lpInputRelativePath, ":/\\" ) != NULL)
	{
		return lpInputRelativePath;
	} // already has drive and/or path separators

	// Base dir already has trailing slash
	CString sz( m_szBaseDir );
	sz += lpInputRelativePath;

	return sz;
} // CShape::LoadTransformPath()

// Save transform for pathnames (convert to relative path if matching m_szBaseDir)
CString
CShape::SaveTransformPath( LPCTSTR lpInputAbsolutePath )
{
	CString sz( lpInputAbsolutePath );
	sz.Replace( '\\', '/' );
	if (sz.GetLength() < m_szBaseDir.GetLength())
	{
		return lpInputAbsolutePath;
	} // No possible match - length is less than base directory
	sz = sz.Left( m_szBaseDir.GetLength() );
	if (!sz.CompareNoCase( m_szBaseDir ))
	{
		// Skip common part
		lpInputAbsolutePath += sz.GetLength();
	} // Got a match
	return lpInputAbsolutePath;
} // CShape::SaveTransformPath()

// Set base directory.  Input may be a file pathname or a directory
// path ending in backslash.
void
CShape::SetTransformBaseDir( LPCTSTR lpPath )
{
	m_szBaseDir.Empty();
	if (!lpPath || !*lpPath)
	{
		return;
	}
	LPCTSTR lpSlash = strrchr( lpPath, '\\' );
	if (!lpSlash) lpSlash = strrchr( lpPath, '/' );
	if (!lpSlash)
	{
		return;
	} // No directory
	m_szBaseDir = lpPath;
	// Get everything up to and including slash
	m_szBaseDir = m_szBaseDir.Left( lpSlash - lpPath + 1 );
	// Make canonical
	m_szBaseDir.Replace( '\\', '/' );
} // CShape::SetTransformBaseDir()

// Substitute $varname$ escapes via dictionary. Return total number of substitutions.
int
CShape::Substitute( CString& sz, unsigned int data1 /* = 0 */, unsigned int data2 /* = 0 */ )
{
	int nTotal = 0;

	if (sz.IsEmpty()) return nTotal;
	if (sz.Find('$') < 0) return nTotal;

	// 1.0.287: Parse text for '$' + varname [+ '(' + int_arg + ')'] + '$'
	//			Support nested expressions and precedence
	//			Evaluate text as
	//			text :: [immtext] [varex] [immtext]
	//			immtext :: str8
	//			varex :: ['$' expr '$']
	//			expr :: varname ['(' text ')'
	//			varname :: ascii8
	//			str8 :: ascii8, '(', ')', '$$'
	//			ascii8 :: all 8-bit ascii characters except '$', '(', ')'
	// All of the following take a CString&, modify it, and return number of substitutions
	// SubText(sz,data1,data2)
	// SubExpr(sz,data1,data2)
	nTotal = SubText( sz, data1, data2 );

	return nTotal;

} // CShape::Substitute()

// Recursive parsing used by Substitute - parse text (see Substitute implementation)
int CShape::SubText( CString& sz, unsigned int data1, unsigned int data2 )
{
	// Scan for variable expression
	int nTotal = 0;
	int nDollar;
	if ((nDollar = sz.Find( '$' )) < 0) return 0;

	// Parse [immtext] [varex] [immtext]
	CString ImmText1, ImmText2;

	int nNext = ScanEscapedText( ImmText1, sz );
	if (nNext >= sz.GetLength()) 
	{
		sz = ImmText1; // Save escaped version
		return 0; // No variable part
	}
	sz = sz.Mid( nNext + 1 );
	CString szVar;
	szVar = sz.SpanExcluding( "$(" );
	sz = sz.Mid( szVar.GetLength() );
	CString szParenText;
	if (sz[0] == '(')
	{
		// Skip nested text
		int nMatched = ScanMatchingParen( (LPCTSTR)sz );
		szParenText = sz.Mid( 1, nMatched - 2 );
		ImmText2 = sz.Mid( nMatched + 1 );
	}
	else
	{
		ImmText2 = sz.Mid( 1 ); // Skip dollar
	}
	nTotal += SubExpr( szVar, szParenText, data1, data2 );
	// If anything left, process it recursively
	if (ImmText2.GetLength() > 0)
	{
		nTotal += SubText( ImmText2, data1, data2 );
	}
	// Assemble final result
	sz = ImmText1 + szVar + ImmText2;
	return nTotal;
}

// Scan for matching paren (handling nested parens). Return offset from lp[0] or -1 if not found. lp[0]=='('
int CShape::ScanMatchingParen( LPCTSTR lp )
{
	int nOffset = 0;
	// Skip opening paren
	nOffset++;
	// Starting level = 1
	int Level = 1;
	while (lp[nOffset] && Level > 0)
	{
		switch (lp[nOffset])
		{
		case '(': 
			Level++;
			break;
		case ')':
			Level--;
			break;
		}
		nOffset++;
	}
	// FIXME throw an error if Level > 0
	ASSERT( Level == 0 );
	return nOffset;
}

// Extract immediate text, escaping '$$' --> '$'. Return index to first non-text
int CShape::ScanEscapedText( CString& szText, LPCTSTR lp )
{
	LPTSTR lpDest;
	LPTSTR lpStart = lpDest = szText.GetBuffer( lstrlen( lp ) );
	int n = 0;
	while (lp[n] != '\0')
	{
		int nStart = n;
		size_t skipped = strcspn( &lp[n], "$" );
		strncpy( lpDest, &lp[n], skipped );
		lpDest += skipped;
		n += skipped;
		if (lp[n] == '$' && lp[n+1] != '$')
			break;
		*lpDest = lp[n];
		lpDest++;
		n++;
		if (lp[n] == '$') n++; // Skip extra
	}
	*lpDest = '\0';
	szText.ReleaseBuffer();
	return n;
}

// Recursive parsing used by Substitute - parse expression (see Substitute implementation)
int CShape::SubExpr( CString& sz, CString szParens, unsigned int data1, unsigned int data2 )
{
	// We have a variable followed by optional argument in parens
	int nTotal = 0;
	if (szParens.GetLength() > 0)
	{
		nTotal += SubText( szParens, data1, data2 );
		// Use this for data2
		szParens = szParens.Trim();
		if (szParens.GetLength() > 0)
		{
			data2 = atoi( szParens );
		}
	}
	// Look up varname
	CString szVal;

	// Substitions are case-sensitive. Go through local map entries first
	if (m_mapVariableDictionary.Lookup( sz, szVal ))
	{
		sz = szVal;
		return nTotal + 1;
	}

	// Process global map entries
	if (MYAPP()->m_prefs.m_mapGlobalDict.Lookup( sz, szVal ))
	{
		sz = szVal;
		return nTotal + 1;
	}

	// Process predefined variable substitution
	// 1.0.287: new syntax '$' + varname [+ '(' + int_arg + ')'] + '$'
	PredefinedCallback cb;
	if (MYAPP()->m_prefs.m_mapPredefs.Lookup( sz, cb ))
	{
		sz = (*cb)(data1, data2);
		return nTotal + 1;
	}

	// If we got here, variable was not found anywhere - return empty
	sz = "<?unk>";
	return nTotal;
}

void CShape::RenameFace( CFace* pFace, LPCTSTR NewName )
{
	// Find reference in face map
	CString szOldName( pFace->m_szFaceName );
	CFace* pMapped;
	if (!this->m_mapFaces.Lookup( szOldName, pMapped ))
	{
		CDbg::Out( 0, "Warning: could not resolve name %s\n", (LPCTSTR)szOldName );
		return;
	} // Didn't find existing entry
	// Make sure new entry doesn't exist
	if (this->m_mapFaces.Lookup( NewName, pMapped ))
	{
		CDbg::Out( 0, "Warning: attempted to rename face %s to existing face %s\n", (LPCTSTR)szOldName, NewName );
		return;
	} // Found new entry
	// Now we can change it
	pFace->m_szFaceName = NewName;
	// Update map entry
	m_mapFaces.RemoveKey( szOldName );
	m_mapFaces.SetAt( NewName, pFace );
} // CShape::RenameFace()

// true if minimum stellation ratio has been exceeded
bool CShape::GetMinimumStellationRatio( double& dMinimum )
{
	if (!this->m_bMinStellationRatioExceeded)
	{
		return false;
	}
	dMinimum = this->m_dMinStellationRatio;
	return m_bMinStellationRatioExceeded;
}

// Set active division. New index returned if valid, -1 if division doesn't exist
int 
CShape::SetActiveDivision( LPCTSTR szNew, bool bAddNew /*=false*/ )
{
	// Use a linear search. As the maximum number of divisions is < 10 this does not pose a performance
	// issue...
	// Note that we don't enforce the division limit but the UI is currently limited to 9 divisions
	// in addition to the default.
	int nDivision = FindDivision( szNew );
	if (nDivision >= 0)
	{
		m_nActiveDivision = nDivision;
		return nDivision;
	} // Found it
	// If we made it here, we didn't find it
	if (bAddNew)
	{
		m_nActiveDivision = m_aDivisions.Add( szNew );
		return m_nActiveDivision;
	}
	// If we aren't adding a new one, that's an error
	return -1;
}

	// Set active division by index. New index returned if valid, -1 if out of range
int CShape::SetActiveDivision( int New )
{
	if (New < 0 || New >= m_aDivisions.GetSize())
		return -1;
	m_nActiveDivision = New;
	return m_nActiveDivision;
}

// Get division index
int CShape::GetDivisionIndex( LPCTSTR sz, bool bAddNew /*= false*/ )
{
	int nDivision = FindDivision( sz );
	if (nDivision >= 0)
	{
		return nDivision;
	} // Found it
	// If we made it here, we didn't find it
	if (bAddNew)
	{
		return m_aDivisions.Add( sz );
	}
	// Not found and not adding
	return -1;
}

// Find division by name. Return index or -1 if not found
int CShape::FindDivision( LPCTSTR sz )
{
	int nDivision;
	for (nDivision = 0; nDivision < this->GetDivisionCount(); nDivision++)
	{
		if (m_aDivisions[nDivision].CompareNoCase( sz ) == 0)
		{
			return nDivision;
		} // Found it
	}
	return -1; // Not found
}

// Write as definition. true if successful.
bool CShape::WriteAsDefinition(CString const& szPathName) const
{
	// We'll write facetype, faces, and joins,
	// and default text for menu, brief
	try
	{
		// Build facetype list and map of faces to facetype defs
		EDGEDEFDICT mapEdgeDefs;
		CMap<CString,LPCTSTR,CString,LPCTSTR> mapFaces;
		POSITION pos;
		CString szFaceName, szDefName;
		CFace* pFace;
		EDGEDEF *pEdgeDef1, *pEdgeDef2;
		for (pos = this->m_mapFaces.GetStartPosition(); pos != NULL; )
		{
			m_mapFaces.GetNextAssoc( pos, szFaceName, pFace );
			pEdgeDef1 = pFace->GetSafeEdgeDef();
			szDefName = CShape::GetPolygonDefName(pFace->m_pEdges.GetSize());
			if (mapEdgeDefs.Lookup( szDefName, pEdgeDef2 ))
			{
				// Does it match? If not, find an alternative
				if (!CShape::AreEdgeDefsEqual( pEdgeDef1, pEdgeDef2 ))
				{
					POSITION posDef;
					EDGEDEF *pDef;
					int nAlternate = 2;
					CString szAlternate, szDefName2;
					bool bFoundMatch = false;
					bool bAlternateIsUnique = false;
					for (posDef = mapEdgeDefs.GetStartPosition(); !bFoundMatch && posDef != NULL; )
					{
						mapEdgeDefs.GetNextAssoc( posDef, szDefName2, pDef );
						// Update alternate name in case we need to create one
						szAlternate.Format( "%s%d", (LPCTSTR)szDefName, nAlternate );
						while (nAlternate < 1000 && szAlternate <= szDefName2)
						{
							nAlternate++;
							szAlternate.Format( "%s%d", (LPCTSTR)szDefName, nAlternate );
						}
						// Check for the case where we did not find a unique numbered name
						bAlternateIsUnique = (szAlternate > szDefName2);
						// Check for a match
						if (CShape::AreEdgeDefsEqual( pEdgeDef1, pDef ))
						{
							bFoundMatch = true;
							szDefName = szDefName2;
						}
					}
					// If no match, create a new definition
					if (!bFoundMatch)
					{
						if (!bAlternateIsUnique)
						{
							szDefName += pFace->m_szFaceName;
						}
						else
						{
							szDefName = szAlternate;
						}
						mapEdgeDefs.SetAt( szDefName, pEdgeDef1 );
					}
				}
			}
			else
			{
				mapEdgeDefs.SetAt( szDefName, pEdgeDef1 );
			}
			// Save def used
			mapFaces.SetAt( pFace->m_szFaceName, szDefName );
		}
		// Open file for append
		CFile f( szPathName, CFile::modeCreate | CFile::modeNoTruncate | CFile::modeWrite );
		f.SeekToEnd();
		f.Write( "\r\n", 2 );
		// Construct data to write
		CString sz;
		sz += "shapedef ";
		sz += this->GetName();
		sz += "\r\n{\r\n";
		// Add menu stub
		sz += "  menu\r\n";
		sz += "  {\r\n";
		sz += "    &Other\r\n";
		sz += "    &";
		sz += GetName();
		sz += "\r\n";
		sz += "  }\r\n";
		// Add brief placeholder text
		sz += "  brief ";
		sz += GetName();
		sz += "\r\n";
		f.Write( sz, sz.GetLength() );
		sz.Empty();
		// Add facetype lists
		for (pos = mapEdgeDefs.GetStartPosition(); pos != NULL; )
		{
			mapEdgeDefs.GetNextAssoc( pos, szDefName, pEdgeDef1 );
			sz += "  facetype ";
			sz += szDefName;
			sz += "\r\n";
			sz += "  {\r\n";
			for (int n = 0; n < pEdgeDef1->nEdges; n++)
			{
				szDefName.Format( "    %s,%.8f", pEdgeDef1->aszName[n], pEdgeDef1->adLength[n] );
				sz += szDefName;
				if (pEdgeDef1->adEndAngles[n] > 0.0)
				{
					szDefName.Format( ",%.8f", pEdgeDef1->adEndAngles[n] );
					sz += szDefName;
				}
				sz += "\r\n";
			}
			sz += "  }\r\n";
		}
		f.Write( sz, sz.GetLength() );
		sz.Empty();
		// Add face definitions
		sz += "  faces\r\n";
		sz += "  {\r\n";
		f.Write( sz, sz.GetLength() );
		// Format is type,name,basename,orientation
		for (pos = mapFaces.GetStartPosition(); pos != NULL; )
		{
			mapFaces.GetNextAssoc( pos, szFaceName, szDefName );
			if (!this->m_mapFaces.Lookup( szFaceName, pFace ))
			{
				continue;
			}
			sz.Format( "    %s,%s,%s,%.2f\r\n", 
				(LPCTSTR)szDefName, 
				(LPCTSTR)szFaceName, 
				(LPCTSTR)pFace->GetBaseEdgeName(),
				pFace->m_fOrientation );
			f.Write( sz, sz.GetLength() );
		}
		sz = "  }\r\n";
		f.Write( sz, sz.GetLength() );
		sz.Empty();
		// Add join list
		CMap<CString,LPCTSTR,CString,LPCTSTR> mapJoins;
		// Keep track of usage on right side
		CMap<CString,LPCTSTR,int,int> mapJoinUsed;
		int nUsageCount;
		this->BuildJoinList( mapJoins );
		CArray<CString,LPCTSTR> aJoins;
		CString szEdge1, szEdge2;
		for (pos = mapJoins.GetStartPosition(); pos != NULL; )
		{
			mapJoins.GetNextAssoc( pos, szEdge1, szEdge2 );
			if (mapJoinUsed.Lookup( szEdge2, nUsageCount ))
			{
				continue;
			}
			nUsageCount = 1;
			mapJoinUsed.SetAt( szEdge1, nUsageCount );
			sz.Format( "%s-%s", (LPCTSTR)szEdge1, (LPCTSTR)szEdge2 );
			aJoins.Add( sz );
		}
		// Sort join list
		bool bDone = false;
		while (!bDone)
		{
			bDone = true;
			CString szSwap;
			for (int nElem = 0; nElem < aJoins.GetSize()-1; nElem++)
			{
				if (aJoins[nElem] > aJoins[nElem+1])
				{
					bDone = false;
					szSwap = aJoins[nElem+1];
					aJoins[nElem+1] = aJoins[nElem];
					aJoins[nElem] = szSwap;
				}
			}
		}
		// Dump sorted join list
		sz =  "  joins\r\n";
		sz += "  {\r\n";
		for (int nElem = 0; nElem < aJoins.GetSize(); nElem++)
		{
			sz += "    ";
			sz += aJoins[nElem];
			sz += "\r\n";
		}
		sz += "  }\r\n";
		// End shapedef
		sz += "}\r\n";
		// Write data to file
		f.Write( sz, sz.GetLength() );
		f.Close();
	}
	catch (char *str)
	{
		::AfxMessageBox( str, MB_OK, 0 );
		return false;
	}
	catch (...)
	{
		::AfxMessageBox( "Unhandled exception in CShape::WriteAsDefinition()\n" );
		return false;
	}
	return true;
}

// Create a definition name appropriate to a polygon with specified number of faces.
CString CShape::GetPolygonDefName( int nSides )
{
	// Handle default cases
	static char *aszPolyNames[] = {
		NULL,	// 0 sides
		NULL,	// 1 sides
		NULL,	// 2 sides
		"T",
		"S",
		"P",
		"H",
		NULL,	// 7 sides
		"O",
		"N",
		"D",
		"U"		// 11 sides = undecagon
	};
#define NUMPOLYNAMES sizeof(aszPolyNames)/sizeof(aszPolyNames[0])
	char *szName = NULL;
	if (nSides >= 0 && nSides < NUMPOLYNAMES)
	{
		szName = aszPolyNames[nSides];
	}
	CString sz;
	if (szName == NULL)
	{
		// 7A for 7-akis, 12A for 12-akis, etc
		sz.Format( "%dA", nSides );
	}
	else
	{
		sz = szName;
	}
	return sz;
}

// Compare edge defs
bool CShape::AreEdgeDefsEqual( EDGEDEF const *p1, EDGEDEF const *p2 )
{
	if (p1==NULL && p2==NULL)
	{
		return true;
	}
	if (p1==NULL || p2==NULL)
	{
		return false;
	}
	if (p1->nEdges != p2->nEdges)
	{
		return false;
	}
	for (int n = 0; n < p1->nEdges; n++)
	{
		if (p1->adLength[n] != p2->adLength[n])
		{
			return false;
		}
		if (p1->adEndAngles[n] != p2->adEndAngles[n])
		{
			return false;
		}
		if (p1->aszName[n] == NULL || p2->aszName[n] == NULL)
		{
			return false;
		}
		if (stricmp( p1->aszName[n], p2->aszName[n] ) != 0)
		{
			return false;
		}
	}
	// They are the same
	return true;
}

// Outdent a single face.  Negative indents. Return number of faces added.
int
CShape::OutdentFace( CFace* pFace, double dHeightRatio, double dSideRatio )
{
	int nAdded = 0;
	CString szName1, szName2, szName3, szName4;
	CString szFaceGroupName;
	bool IsSquare = (pFace->m_pEdges.GetSize() == 4);

	// Only supported for quadrangular faces
	if (!IsSquare)
	{
		return 0;
	}

	// Construct names for new faces by adding "d1", "d2", "d3" and "d4"
	// Resize current face
	// Make base edge the original base edges

	// If outdent is negative, it will only set concave attributes
	bool bConcave = (dHeightRatio < 0.0);
	dHeightRatio = fabs( dHeightRatio );

	// Make sure those new edge names are unique!
	szName1 = MakeUniqueFaceName( pFace->m_szFaceName, "d1" );
	szName2 = MakeUniqueFaceName( pFace->m_szFaceName, "d2" );
	szName3 = MakeUniqueFaceName( pFace->m_szFaceName, "d3" );
	szName4 = MakeUniqueFaceName( pFace->m_szFaceName, "d4" );

	szFaceGroupName = MakeUniqueFaceName( pFace->m_szFaceName, "_d" );

	// Get original sides starting with base
	double aOrgSides[4];
	CEdge* aEdges[4];
	aEdges[0] = pFace->GetBaseEdge();
	aOrgSides[0] = aEdges[0]->m_dLength;
	int n, nSides;
	nSides = pFace->m_pEdges.GetCount();
	double dMaxSide = aOrgSides[0];
	for (n = 1; n < nSides; n++)
	{
		aEdges[n] = aEdges[n-1]->GetClockwiseNext();
		aOrgSides[n] = aEdges[n]->m_dLength;
		if (dMaxSide < aOrgSides[n])
		{
			dMaxSide = aOrgSides[n];
		}
	}

	// Determine height
	double dHeight = dMaxSide * dHeightRatio;
	long double dHeight2 = dHeight * dHeight;

	// Determine side margins and vertical edge diagonals at the start of each side
	// Also get inside angles following a and b edges, which are identical to those
	// following d and c respectively.
	double aSideMargins[4];
	double aVertDiags[4];
	double aFollowA[4];
	double aFollowB[4];
	for (n = 0; n < nSides; n++)
	{
		aSideMargins[n] = (aOrgSides[n] - aOrgSides[n] * dSideRatio) / 2.0;
	}
	for (n = 0; n < nSides; n++)
	{
		long double mx, my, pdiag;
		mx = aSideMargins[(n+nSides-1)%nSides];
		my = aSideMargins[n];
		pdiag = sqrt( mx * mx + my * my );
		aVertDiags[n] = sqrt( pdiag * pdiag + dHeight2 );
		aFollowA[n] = 90.0 + CFace::RadansToDeg( asin( aSideMargins[n] / aVertDiags[n] ) );
		aFollowB[n] = 180.0 - aFollowA[n];
	}

	// Resize original edges. Angles won't change.
	for (n = 0; n < nSides; n++)
	{
		aEdges[n]->m_dLength *= dSideRatio;
	}


		// Construct new faces
		CFace* pName1Face = new CFace( this, szName1 );
		CFace* pName2Face = new CFace( this, szName2 );
		CFace* pName3Face = new CFace( this, szName3 );
		CFace* pName4Face = new CFace( this, szName4 );
		ASSERT( pName1Face != NULL );
		ASSERT( pName2Face != NULL );
		ASSERT( pName3Face != NULL );
		ASSERT( pName4Face != NULL );

        // All new faces will surround the existing face, going clockwise,
		// with the "a" edge connected to an edge of the original face.
		const char *apszNames[4] = { "a", "b", "c", "d" };
		double adLengths[4];
		// Angles are for the end of each edge going clockwise
		double adAngles[4];
		CFace* af[] = { pName1Face, pName2Face, pName3Face, pName4Face };
		const char *aBases[] = {"c","a","a","a"};
		for (n = 0; n < nSides; n++)
		{
			adLengths[0] = aEdges[0]->m_dLength;
			adLengths[1] = aVertDiags[n];
			adLengths[2] = aOrgSides[n];
			adLengths[3] = aVertDiags[(n+1)%nSides];
			adAngles[0] = aFollowA[n];
			adAngles[1] = aFollowB[n];
			adAngles[2] = aFollowB[n];
			adAngles[3] = aFollowA[n];
			af[n]->CreatePolygon( apszNames, adLengths, adAngles, 4 );
			af[n]->m_fOrientation = 90.0;
			af[n]->SetBaseEdgeName( aBases[n] );
		}

		// Set up joins
		for (n = 0; n < nSides; n++)
		{
			int nSucc = (n+1) % nSides;
			int nPred = (n-1+nSides) % nSides;

			// c edge always joins to outer join of original edge
			CEdge* pName1Face_c = af[n]->GetEdge( "c" );
			ASSERT( pName1Face_c != NULL );
			pName1Face_c->m_bConcave = false; // Outer join will usually be convex
			pName1Face_c->JoinOutward( aEdges[n]->m_pOutwardConnectedEdge );

			// Original edge joins to a edge
			CEdge* pName1Face_a = af[n]->GetEdge( "a" );
			ASSERT( pName1Face_a != NULL );
			pName1Face_a->JoinOutward( aEdges[n] );

			// b edge joins to predecessor's d
			CEdge* pName1Face_b = af[n]->GetEdge( "b" );
			ASSERT( pName1Face_b != NULL );
			pName1Face_b->JoinOutward( af[nPred]->GetEdge( "d" ) );

			// d edge will be taken care of in another iteration
		}

		// Make them all part of the new group
		pFace->AddToGroup( szFaceGroupName );
		// Newly created faces take the entire group collection from original face
		pName1Face->CopyGroups( pFace );
		pName2Face->CopyGroups( pFace );
		pName3Face->CopyGroups( pFace );
		pName4Face->CopyGroups( pFace );


		// Add new faces to our map
		m_mapFaces.SetAt( pName1Face->m_szFaceName, pName1Face );
		m_mapFaces.SetAt( pName2Face->m_szFaceName, pName2Face );
		m_mapFaces.SetAt( pName3Face->m_szFaceName, pName3Face );
		m_mapFaces.SetAt( pName4Face->m_szFaceName, pName4Face );

		return 4;

} // CShape::OutdentFace()

/*!

	@file	 ShapeLayout.cpp
	
	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: ShapeLayout.cpp 15 2006-03-30 15:34:42Z henry_groover $

  ShapeLayout.cpp: implementation of the CShapeLayout class.

*/

#include "stdafx.h"
#include "PaperCut.h"
#include "ShapeLayout.h"
#include "Vertex.h"
#include "MultiVertex.h"
#include "PageGroup.h"
#include "LogPoint.h"
#include "LogRect.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/// Defaults are used when options are not set in shape or global preferences
double CShapeLayout::m_dDefaultTabShoulderAngle = 27.0;
double CShapeLayout::m_dDefaultMinimalJoinAngle = 2 * CShapeLayout::m_dDefaultTabShoulderAngle;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CShapeLayout::CShapeLayout( CShape* pShape )
	:	m_pCutEdges(),
		m_mapFaceRotations(),
		m_adMultiVertexAddedAngles(),
		m_adLogHeight(),
		m_adLogWidth(),
		m_aLogMax(),
		m_aLogMin(),
		m_aLogOrigin(),
		m_aPages()
{
	m_dOrientation = 0.0; // Point north
	m_dScale = 1.0;
	m_bValid = false;
	// This value is ignored mostly - it is a logical place to store
	// this value from the view during serialization.
	m_dPageWidthInLogicalUnits = 3;
	m_pShape = NULL;
	Reset( pShape );
}

CShapeLayout::~CShapeLayout()
{
	if (m_pShape)
	{
		delete m_pShape;
		m_pShape = NULL;
	}
	FreeAllPages();
	m_pCutEdges.RemoveAll();
}

// Remove and free all page groups
void
CShapeLayout::ResetPageGroups()
{
	int nSize = m_aPages.GetSize();
	int nPage;
	for (nPage = 0; nPage < nSize; nPage++)
	{
		delete m_aPages[nPage];
		m_aPages[nPage] = NULL;
	}
	m_aPages.RemoveAll();
} // CShapeLayout::ResetPageGroups()


// Do a single join.  Return 1 if successful, 0 if not found, -1 if error
int
CShapeLayout::Join( const char *pszSourceFaceEdge )
{
	// Check for polygon collision on potential face joins within PopFaceEdgeJoin()
	if (m_pShape->PopFaceEdgeJoin( pszSourceFaceEdge, m_mapPopped, this ))
	{
		return 1;
	} // Success
	return 0;
} // CShapeLayout::Join()

// Automatically join laid out faces
int CShapeLayout::AutoJoin( int nMaxPerPage )
{
	// Find a face with 3 edges joined
	// (or the face with the maximum joined edges)

	m_bValid = false;

	// This takes a long time!
	HCURSOR hcurseOld = ::SetCursor( ::LoadCursor( NULL, IDC_WAIT ) );

	// Initialize data structures
	m_aDiscreteFaces.RemoveAll();
	m_mapFaceGroup.RemoveAll();
	m_mapOrphanCandidates.RemoveAll();

	// Get layout fit options
	int LayoutFit = m_pShape->GetLayoutFit();

	// Filter faces by layout division
	POSITION posFace;
	CFace* pFace;
	CString szFaceName;
	CMap<CString,LPCTSTR,CFace*,CFace*> mapFace;
	CString szActiveDivision = m_pShape->GetActiveDivision();
	for (posFace = m_pShape->m_mapFaces.GetStartPosition(); posFace != NULL; )
	{
		m_pShape->m_mapFaces.GetNextAssoc( posFace, szFaceName, pFace );
		if (pFace->GetLayoutDivision().CompareNoCase( szActiveDivision ) == 0)
		{
			mapFace.SetAt( szFaceName, pFace );
		}
	}

	int nReturn = 0;
	int nTotalFaces = mapFace.GetCount(); //m_pShape->GetNumFaces();
	int nMaxFacesPerPage = nMaxPerPage; //__max( 1, nTotalFaces / nTargetPages );
	//if (nMaxFacesPerPage < nTotalFaces - 1 && nTargetPages > 1)
	//{
	//	nMaxFacesPerPage++;
	//} // Account for truncation

	// d'Oh!!! We've cut all face joins so none are joined
	bool bFirstTime = true;
	/*****
	// First find all faces with no outer joins
	for (posFace = m_pShape->m_mapFaces.GetStartPosition(); posFace != NULL; )
	{
		m_pShape->m_mapFaces.GetNextAssoc( posFace, szFaceName, pFace );
		if (pFace->GetJoinCount() == 0)
		{
			CDbg::Out( "Isolated face %s\n", (LPCTSTR)szFaceName );
			m_aDiscreteFaces.Add( pFace );
		}
	}
	****/

	do {

		// Build a map of faces to number of occurrences
		CMap<CFace*,CFace*,int,int> mapFaceOccurrences;
		// Note that m_pShape->mapFaceEdge may reference faces in other divisions
		int nSize = m_pShape->m_mapFaceEdge.GetCount();
		POSITION pos;
		CString sz1, sz2;
		// If no joins that's ok...
		// Seed mapFaceOccurrences with identity value 0 for all faces
		for (posFace = mapFace.GetStartPosition(); posFace != NULL; )
		{
			mapFace.GetNextAssoc( posFace, szFaceName, pFace );
			mapFaceOccurrences.SetAt( pFace, 0 );
		}

		//CDbg::m_Level++;
		CDbg::Out( "\n----- %d face edge joins -----\n", nSize );

		for (pos = m_pShape->m_mapFaceEdge.GetStartPosition(); pos != NULL; )
		{
			// FIXME is this one-sided lookup the reason why we're not getting
			// outward join info in some cases?
			m_pShape->m_mapFaceEdge.GetNextAssoc( pos, sz1, sz2 );
			CDbg::Out( 2, "\t%s -> %s\n", (LPCTSTR)sz1, (LPCTSTR)sz2 );
			// Missing faces usually indicates that we're in another division
			CFace* pFace;
			if (!mapFace.Lookup( sz1.SpanExcluding( ":" ), pFace ))
			{
				continue;
			}
			//2006-02-26 superseded by above
			//CFace* pFace = m_pShape->FindFace( (LPCTSTR)(sz1.SpanExcluding( ":" )) );
			//if (!pFace)
			//{
			//	CDbg::Out( 0, "Warning: could not find %s\n", (LPCTSTR)sz1 );
			//	continue;
			//}

			// If first time through, add null associations
			if (bFirstTime && sz2.IsEmpty())
			{
				CDbg::Out( "Adding null association %s\n", (LPCTSTR)sz1 );
				m_aDiscreteFaces.Add( pFace );
				continue;
			}

			int nCount;
			if (mapFaceOccurrences.Lookup( pFace, nCount ))
			{
				nCount++;
			}
			else
			{
				nCount = 1;
			}
			// Update count
			mapFaceOccurrences.SetAt( pFace, nCount );
		} // for all joins
		//CDbg::m_Level--;

		// Find maximum join with lexically least name (which may be 0)
#define MAXSIDES	12
		CFace* pa[MAXSIDES] = {NULL,NULL,NULL,NULL,
						NULL,NULL,NULL,NULL,
						NULL,NULL,NULL,NULL};
		CFace* pf;
		int nCount;
		CString szLeastName[MAXSIDES];
		for (pos = mapFaceOccurrences.GetStartPosition(); pos != NULL; )
		{
			mapFaceOccurrences.GetNextAssoc( pos, pf, nCount );
			if (nCount > pf->m_pEdges.GetSize()) continue;
			if (!bFirstTime && nCount < 1) continue;
			if (nCount < 1)
			{
				//m_aDiscreteFaces.Add( pf );
				continue;
			} // First time, add all unconnected faces
			// Do we have a lexically lower name?
			if (pa[nCount]) 
			{
				if (pf->m_szFaceName.CompareNoCase( szLeastName[nCount] ) >= 0)
				{
					continue;
				} // Not lexically less
			}
			// Skip this one if already added
			// FIXME use a map for this!!!
			int nDiscrete;
			bool bExists = false;
			for (nDiscrete = m_aDiscreteFaces.GetUpperBound();
				nDiscrete >= 0 && !bExists;
				nDiscrete--)
			{
				bExists = (pf == m_aDiscreteFaces[nDiscrete]);
			}
			if (bExists)
			{
				continue;
			}
			szLeastName[nCount] = pf->m_szFaceName;
			pa[nCount] = pf;
		}
		// Clear first time flag
		bFirstTime = false;

		int n;
		CFace* pMax = NULL;
		// Find largest possible
		for (n = (sizeof(pa)/sizeof(pa[0]))-1; n >= 0; n--)
		{
			if (pMax = pa[n])
			{
				break;
			} // found one
		} // for all joins

		if (!pMax)
		{
			break;
		}

		// Add to list of discrete face groups
		CDbg::Out( "*** Adding face %s as root\n", (LPCTSTR)pMax->m_szFaceName );
		m_aDiscreteFaces.Add( pMax );

		// Join it - note that we already have one face
		int nMaxToJoin = nMaxFacesPerPage - 1;
		try {
			nReturn += RecursiveJoin( pMax, true, nMaxToJoin );
		}
		catch (...)
		{
			CDbg::Out( 0, "Caught exception in recursivejoin for %s!!!\n", (LPCTSTR)pMax->m_szFaceName );
		}

	} while (1);

	CDbg::Out( "\nDone with AutoJoin, checking for orphans (%d candidates)\n", m_mapOrphanCandidates.GetCount() );

	// Check for orphans
	int nRemovals;
	POSITION pos;
	for (pos = m_mapOrphanCandidates.GetStartPosition(); pos != NULL; )
	{
		m_mapOrphanCandidates.GetNextAssoc( pos, szFaceName, nRemovals );
		// Find face
		CFace* pFace; // = m_pShape->FindFace( szFaceName );
		if (mapFace.Lookup( szFaceName, pFace ))
		{
			// Make sure it is an orphan
			if (pFace->GetJoinCount() == 0 && nRemovals > 0)
			{
				CDbg::Out( "Adding orphan %s\n", (LPCTSTR)szFaceName );
				m_aDiscreteFaces.Add( pFace );
			}
		}
	} // for all candidates
	CDbg::Out( "\nAutoJoin(%08x) complete, %d faces\n---\n\n", (unsigned int)this, m_aDiscreteFaces.GetSize() );

	int nSize = m_aDiscreteFaces.GetSize();
	m_bValid = (nSize > 0);

	int nFaceGroup;

	// HGroover 19Jun06 - we're going to keep this attribute - tabs are always folded the same
	// way but concave planar joins are not always obvious.
	/**
	// Turn off concave attribute for all outer edges.  These may
	// be concave due to stellation but tabs are always folded inward.
	for (nFaceGroup = 0; nFaceGroup < nSize; nFaceGroup++)
	{
		m_aDiscreteFaces[nFaceGroup]->ClearOutwardEdgeConcavity( m_aDiscreteFaces[nFaceGroup] );
	} // for all face groups
	**/

	// Set up page groups
	ResetPageGroups();
	int nPage;
	for (nFaceGroup = 0; nFaceGroup < nSize; nFaceGroup++)
	{
		CPageGroup* pPage = NULL;
		for (nPage = 0; nPage < m_aPages.GetSize(); nPage++)
		{
			if (m_aPages[nPage]->AddFace( m_aDiscreteFaces[nFaceGroup], nFaceGroup, true, NULL, LayoutFit  ))
			{
				CDbg::Out( "Added fg %d to existing page %d\n", nFaceGroup, nPage );
				// Found a home for the face group
				pPage = m_aPages[nPage];
				break;
			}
		} // for all pages
		if (pPage == NULL)
		{
			CDbg::Out( "Adding new page %d for fg %d\n", nPage, nFaceGroup );
			pPage = new CPageGroup( m_dPageWidthInLogicalUnits, m_dPageWidthOverHeight );
			// This will happen if we don't have the right scale setting...
			bool bAdded = pPage->AddFace( m_aDiscreteFaces[nFaceGroup], nFaceGroup, true, NULL, LayoutFit );
			ASSERT( bAdded );
			// Save origin:0 page number
			pPage->SetPageNumber( nPage );
			m_aPages.Add( pPage );
		} // Add new page
	} // for all face groups

	// Set page group reference for all faces in each group
	for (nPage = 0; nPage < m_aPages.GetSize(); nPage++)
	{
		m_aPages[nPage]->SetSelfReference();
	}

	// Restore old cursor
	::SetCursor( hcurseOld );

	return nSize;
}

// Calculate all bounding rectangles enclosing laid out shape
int CShapeLayout::CalculateAllBoundingRectangles()
{
	m_adLogWidth.RemoveAll();
	m_adLogHeight.RemoveAll();
	m_aLogMin.RemoveAll();
	m_aLogMax.RemoveAll();
	m_aLogOrigin.RemoveAll();

	CArray<CLogPoint,CLogPoint const&> a;

	//m_dOrientation = 0.0;

	// Do all face groups

	// Get modular min, max, width and height
	// This goes across all pages and is useful for printing...
	m_dLogSummaryWidth = 0.0;
	m_dLogSummaryHeight = 0.0;
	m_LogSummaryMin.Set( 10000.0, 10000.0 );

	int nSize = m_aDiscreteFaces.GetSize();
	int n;
	for (n = 0; n < nSize; n++)
	{
		// Set origin for each face group
		m_adLogWidth.Add( 0.0 );
		m_adLogHeight.Add( 0.0 );
		m_aLogMin.Add( CLogPoint( 10000, 10000 ) );
		m_aLogMax.Add( CLogPoint( 0, 0 ) );
		m_aLogOrigin.Add( CLogPoint( 0, 0 ) );
		m_adOptimalOrientation.Add( 0.0 );
		CFace* pFace = m_aDiscreteFaces[n];

		a.RemoveAll();
		pFace->GetVertexCoordinates( NULL, 0, m_aLogOrigin[n],
				m_dOrientation, a );

		GetMinMaxWidth( a, m_adLogWidth[n], m_adLogHeight[n], m_aLogMin[n] );
		// Not sure if this is useful...
		m_aLogOrigin[n] = m_aLogMin[n];

		CDbg::Out( "For face %d, got w%f h%f minx %f miny %f\n",
			n, m_adLogWidth[n], m_adLogHeight[n], m_aLogMin[n].m_dX, m_aLogMin[n].m_dY );

		// If width exceeds height, swap and rotate orientation 90 degrees...
		if (false && m_adLogWidth[n] > m_adLogHeight[n])
		{
			m_adOptimalOrientation[n] += 90.0;
			double d;
			d = m_adLogWidth[n];
			m_adLogWidth[n] = m_adLogHeight[n];
			m_adLogHeight[n] = d;
			CLogRect R( m_aLogMin[n], m_aLogMax[n], m_aLogOrigin[n] );
			R.Rotate90();
			R.Normalize();
			R.GetMin( m_aLogMin[n] );
			R.GetMax( m_aLogMax[n] );
			R.GetOrg( m_aLogOrigin[n] );
		} // Rotate

		// Compare summary values
		if (m_adLogWidth[n] > m_dLogSummaryWidth)
		{
			CDbg::Out( "Max width of %f exceeded\n", m_dLogSummaryWidth );
			m_dLogSummaryWidth = m_adLogWidth[n];
		}
		if (m_adLogHeight[n] > m_dLogSummaryHeight)
		{
			CDbg::Out( "Max height of %f exceeded\n", m_dLogSummaryHeight );
			m_dLogSummaryHeight = m_adLogHeight[n];
		}
		// This is of dubious utility...
		if (m_aLogMin[n].SetToLesser( m_LogSummaryMin ))
		{
			CDbg::Out( "Min exceeded negatively, but ignoring\n" );
		}
	} // for all face groups


	// Restore debug level

	// Return number of face groups processed
	return nSize;
}

// Get bounding rectangle enclosing entire laid out shape
// for specified face group (origin:0)
void
CShapeLayout::GetBoundingRectangleFor( int nFaceGroup, double& dWidth, double& dHeight, double& dMinX, double& dMinY )
{
	if (nFaceGroup < 0 || nFaceGroup > m_aDiscreteFaces.GetUpperBound())
	{
		CDbg::Out( 0, "Error: tried to get bounds for face group %d which doesn't exist!\n", nFaceGroup );
		return;
	}
	ASSERT( nFaceGroup < m_adLogWidth.GetSize() );
	ASSERT( nFaceGroup < m_adLogHeight.GetSize() );
	ASSERT( nFaceGroup < m_aLogMin.GetSize() );

	dWidth = m_adLogWidth[nFaceGroup];
	dHeight = m_adLogHeight[nFaceGroup];
	dMinX = m_aLogMin[nFaceGroup].m_dX;
	dMinY = m_aLogMin[nFaceGroup].m_dY;
} // CShapeLayout::GetAllBoundingRectangles()

void CShapeLayout::GetBoundingRectangleForAllPages(double &dWidth, double &dHeight, double &dMinX, double &dMinY)
{
	dWidth = m_dLogSummaryWidth;
	dHeight = m_dLogSummaryHeight;
	dMinX = m_LogSummaryMin.m_dX;
	dMinY = m_LogSummaryMin.m_dY;
}

void CShapeLayout::GetBoundingMaxRectangleFor(int nFaceGroup, double &dWidth, double &dHeight, double &dMinX, double &dMinY)
{
	if (nFaceGroup < 0 || nFaceGroup > m_aDiscreteFaces.GetUpperBound())
	{
		CDbg::Out( 0, "Error: tried to get max bounds for face group %d which doesn't exist!\n", nFaceGroup );
		return;
	}
	dWidth = m_dLogSummaryWidth;
	dHeight = m_dLogSummaryHeight;
	dMinX = m_aLogMin[nFaceGroup].m_dX;
	dMinY = m_aLogMin[nFaceGroup].m_dY;
}

// Get minimum x and y as well as width
void
CShapeLayout::GetMinMaxWidth( CArray<CLogPoint,CLogPoint const&>& a,
				 double& dWidth, double& dHeight, CLogPoint& MinPoint )
{
	// Nothing to do if empty
	if (a.GetSize() < 1) return;

	// Find min and max
	MinPoint = a[0];

	// Nothing to do if unity
	if (a.GetSize() < 2) return;

	CLogPoint MaxPoint( a[0] );
	int n;
	for (n = 0; n < a.GetSize(); n++)
	{
		MinPoint.SetToLesser( a[n] );
		MaxPoint.SetToGreater( a[n] );
	} // find min/max
	CLogPoint Diff;
	Diff = MaxPoint - MinPoint;
	dWidth = Diff.m_dX;
	dHeight = Diff.m_dY;
	CDbg::Out( "Got %d vertex coords, min x,y = %f, %f  max x,y = %f, %f, size %f, %f\r\n",
			a.GetSize(), MinPoint.m_dX, MinPoint.m_dY, MaxPoint.m_dX, MaxPoint.m_dY, dWidth, dHeight );
} // CShapeLayout::GetMinMaxWidth()

// Access a face in m_aDiscreteFaces by ordinal
CFace* CShapeLayout::GetBaseFace(int nIndex)
{
	int nSize = m_aDiscreteFaces.GetSize();
	if (nIndex < 0 || nIndex >= nSize)
	{
		// Don't complain unless it's an attempt to get past end
		// or with a negative index
		if (nIndex < 0 || nIndex > nSize)
		{
			CDbg::Out( 0, "Error: attempted to get base face %d\n", nIndex );
		}
		return NULL;
	}
	return m_aDiscreteFaces[nIndex];
}

// Join a face and all connected faces
int CShapeLayout::RecursiveJoin( CFace* p, bool bDepthFirst /*= true*/, int& nMaxFaces /*=32767*/ )
{
	// Join all edges connected to this face
	// This is done by popping them from the "stack"
	// so we do not need to worry about violating the
	// planar layout.

	// Note that we also need to check for interior intersections as described at
	// http://paper-model.com/software/polygon-intersection.php but this will
	// happen several layers deep:
	/*
	RecursiveJoin()
	  Join()
	    CShape.PopFaceEdgeJoin()
		  CPageGroup.AddFace() - this is where we map out all the coordinates and check for intersections
	*/

	int nReturn = 0;

	POSITION pos;
	CString szEdgeName;
	CEdge* pEdge;
	for (pos = p->m_pEdges.GetStartPosition(); pos != NULL; )
	{
		p->m_pEdges.GetNextAssoc( pos, szEdgeName, pEdge );
		if (nMaxFaces <= 0)
		{
			if (pEdge->m_pOutwardConnectedEdge)
			{
				m_pShape->RemoveFaceAsTarget( pEdge->m_pOutwardConnectedEdge->m_pFace, true, &m_mapOrphanCandidates );
			}
			continue;
		} // Remove outer face as target

		// Check for possible joins, and if minimal join angle permits
		// join them and return number joined (always 1 or 0)
		// Join will also update m_loc for vertices of added faces
		int nJoined = Join( pEdge->GetFQName() );
		// If joined, remove this face as a target
		// The pool of joins reflects the state of the solid polyhedron.
		// They cannot all apply to a net layout.
		nMaxFaces -= nJoined;
		if (nJoined && pEdge->m_pOutwardConnectedEdge)
		{
			m_pShape->RemoveFaceAsTarget( pEdge->m_pOutwardConnectedEdge->m_pFace, false, &m_mapOrphanCandidates );
		}
		nReturn += nJoined;
	} // for all edges

	// Early out if nothing new
	if (!bDepthFirst || !nReturn)
	{
		CDbg::Out( "Nothing joined\n" );
		return nReturn;
	}

	// If limit exceeded, make sure connecting faces are
	// cut off from all others as potential targets.
	if (!nMaxFaces && !nReturn)
	{
		CDbg::Out( 0, "Error: unexpected condition in AutoJoin!\n" );
		return nReturn;
	} // This should not be possible

	// Go through again and hook up connected faces
	for (pos = p->m_pEdges.GetStartPosition(); pos != NULL; )
	{
		p->m_pEdges.GetNextAssoc( pos, szEdgeName, pEdge );
		if (pEdge->m_pOutwardConnectedEdge)
		{
			if (!nMaxFaces)
			{
				m_pShape->RemoveFaceAsTarget( pEdge->m_pOutwardConnectedEdge->m_pFace, true, &m_mapOrphanCandidates );
				continue;
			} // Cut it off
			// Make sure we don't get hooked up as a target ourselves
			// Don't add ourselves to the orphan candidate list
			m_pShape->RemoveFaceAsTarget( p );
			// Filter out faces not in the current division
			if (pEdge->m_pOutwardConnectedEdge->m_pFace->GetLayoutDivision().CompareNoCase( m_pShape->GetActiveDivision() ) == 0)
			{
				int nJoin = RecursiveJoin( pEdge->m_pOutwardConnectedEdge->m_pFace, true, nMaxFaces );
				nReturn += nJoin;
			}
			// Now remove joined face as a target, even if it's not in this layout division.
			// This handles the case where we ran out of maximum faces to join
			// in a deeper level of recursion - we might have a face that is connected
			// on other edges we could not pursue. Don't add to orphan candidate list.
			m_pShape->RemoveFaceAsTarget( pEdge->m_pOutwardConnectedEdge->m_pFace, true );
		}
	}

	return nReturn;
}

int CShapeLayout::GetFaceGroupCount()
{
	return m_aDiscreteFaces.GetSize();
}


void CShapeLayout::Reset( CShape* pShape )
{
	// Won't be valid until we do an AutoJoin()
	m_bValid = false;
	// Delete existing shape
	if (m_pShape)
	{
		delete m_pShape;
		m_pShape = NULL;
	}
	m_pShape = new CShape( *pShape );
	// Use shape's attributes
	m_pShape->SetParent( pShape );
	m_pShape->ClearFaceEdgeMap();
	m_pShape->CutAllFaceJoins();
	pShape->BuildJoinList( m_pShape->m_mapFaceEdge );

	// Pick any face for the base orientation
	// As faces are cut at the edges and unfolded,
	// their orientation deltas are set in m_mapFaceRotations
	// except the base orientation face, which remains constant.
	// In other words, all faces will be unfolded into the plane of
	// m_pBaseOrientationFace.  This means that the intrinsic
	// orientation of the base face must be added to the delta for all
	// other rotations.
	m_pBaseOrientationFace = m_pShape->GetFirstFace();
	//m_pBaseOrientationFace->m_fOrientation;

	// Create map of unified vertices
	m_pShape->MapVertices();
}

// Get specified page group
CPageGroup*
CShapeLayout::GetPage( int nPage ) const
{
	if (nPage < 0 || nPage >= m_aPages.GetSize())
	{
		return NULL;
	}
	return m_aPages.GetAt( nPage );
} // CShapeLayout::GetPage()

// Free all page groups
void CShapeLayout::FreeAllPages()
{
	int n;
	for (n = 0; n < m_aPages.GetSize(); n++)
	{
		delete m_aPages[n];
	}
	m_aPages.RemoveAll();
}

bool CShapeLayout::IsValid()
{
	return m_bValid;
}

// Save layout properties in symbolic form
CString
CShapeLayout::SaveSymbolic()
{
	CString sz;
	sz.Format( "layout\n{\n  pagewidth %lf\n  widthratio %lf\n}\n",
			m_dPageWidthInLogicalUnits, m_dPageWidthOverHeight );
	return sz;
} // CShapeLayout::SaveSymbolic()

// Restore layout properties
void
CShapeLayout::RestoreSymbolic( LPCTSTR lp, double& dPageWidthInLogicalUnits, double& dPageWidthOverHeight )
{
	dPageWidthInLogicalUnits = 3;
	dPageWidthOverHeight = 0.762;
	CString sz = lp;
	if (sz.SpanExcluding( "\r\n" ) != "layout")
	{
		return;
	}
	LPCTSTR lpTag;
	if (lpTag = strstr( lp, "pagewidth" ))
	{
		// Skip over non-whitespace
		lpTag += strcspn( lpTag, " \t" );
		// Skip whitespace
		lpTag += strspn( lpTag, " \t" );
		sz = lpTag;
		dPageWidthInLogicalUnits = atof( sz.SpanExcluding( "\r\n" ) );
	}
	if (lpTag = strstr( lp, "widthratio" ))
	{
		lpTag += strcspn( lpTag, " \t" );
		lpTag += strspn( lpTag, " \t" );
		sz = lpTag;
		dPageWidthOverHeight = atof( sz.SpanExcluding( "\r\n" ) );
	}
} // CShapeLayout::RestoreSymbolic()

double CShapeLayout::GetMinimalJoinAngle() const
{
	double dRet = CShapeLayout::m_dDefaultMinimalJoinAngle;
	if (this->m_pShape != NULL)
	{
		// Get fit angle specified in shape
		if (m_pShape->HasMinFitAngle())
		{
			dRet = m_pShape->GetMinFitAngle();
		}
		// If no tabs, minimum fit angle is 0
		if (m_pShape->HasUseTabs() && m_pShape->GetUseTabs() == CGlobalPreferences::UTB_NONE)
		{
			dRet = 0.0;
		}
	}
	return dRet;
}

double CShapeLayout::GetTabShoulderAngle() const
{
	double dRet = CShapeLayout::m_dDefaultTabShoulderAngle;
	if (this->m_pShape != NULL && this->m_pShape->HasTabShoulderAngle())
	{
		dRet = m_pShape->GetTabShoulderAngle();
	}
	return dRet;
}


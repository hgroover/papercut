/*!

	@file	 PageGroup.cpp
	
	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: PageGroup.cpp 9 2006-03-08 14:41:10Z henry_groover $

  Page group represents a physical print page which has one more more face groups placed on it.
  This is used to fit layouts to pages. The page group maintains a free list of unused regions
  available for placing shapes. 
  TODO: Currently this object does NOT implement convex / concave fitting, which would provide
  more efficient use of paper.

*/

#include "stdafx.h"
#include "PaperCut.h"
#include "PageGroup.h"
#include "MainFrm.h"

#include <afxtempl.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPageGroup::CPageGroup( double dPageWidthInLogicalUnits, double dWidthOverHeight )
	: CLogRect()
{
	// Set our size initially
	//CMainFrame* pMain = (CMainFrame*)AfxGetMainWnd();
	SetOrigin( 0, 0 );
	m_Min.Set( 0, 0 );
	m_Max.SetX( dPageWidthInLogicalUnits );
	m_Max.SetY( m_Max.m_dX / dWidthOverHeight );
	m_dOrientation = 0;
	m_PageNumber = -1;

	// Initially populate free list with ourselves
	m_aFreeList.Add( *this );
}

CPageGroup::~CPageGroup()
{
	// Go through list of faces and reset page group
	// This code executes after the scalar deleting destructor, so it's pointless and
	// references freed heap space... bad idea!
#if 0
	for (int n = 0; n < m_pFaces.GetSize(); n++)
	{
		CFace* pFace = m_pFaces[n];
		if (pFace != NULL)
		{
			pFace->SetPageGroup( NULL );
		}
	}
#endif
}

// Will this face fit?
//bool
//CPageGroup::WillFit( CFace* pFace ) const
//{
//	return true;
//} // CPageGroup::WillFit()

// Add a face to this page
bool
CPageGroup::AddFace( CFace* pFace, int nFaceGroupIndex, bool bAlwaysAddFirst /*= true*/, CFace* pUnjoinedFace /* = NULL */, int LayoutFit /* = LFIT_FULL */ )
{

	// Get bounding rectangle for face group
	CArray<CLogPoint,CLogPoint const&> a;
	CArray<int> idx;
#define	NUMORIENTS	37
#define	STARTORIENT	18
	CLogRect fgRect[NUMORIENTS];
	double dTrials[NUMORIENTS] = { -90, -85, -80, -75, -70, -65, -60, -55, -50,
									-45, -40, -35, -30, -25, -20, -15, -10, -5,
									0,
									5, 10, 15, 20, 25, 30, 35, 40, 45,
									50, 55, 60, 65, 70, 75, 80, 85, 90 };
	int nTrial;
	int nMaxTrial = STARTORIENT;
	int nMinTrial = nMaxTrial;
	int nTrialDirection;
	bool FirstPass = true;

	// Check for other options
	switch (LayoutFit)
	{
	case CPreferenceSet::LFIT_BY90:
		nTrialDirection = 18;
		break;
	case CPreferenceSet::LFIT_NEG90:
		nTrialDirection = -18;
		nMaxTrial = NUMORIENTS-1;
		break;
	case CPreferenceSet::LFIT_BY45:
		nTrialDirection = 9;
		break;
	case CPreferenceSet::LFIT_NEG45:
		nMaxTrial = STARTORIENT-9;
		nTrialDirection = 9;
		break;
	case CPreferenceSet::LFIT_BY15:
		nTrialDirection = 3;
		break;
	case CPreferenceSet::LFIT_NEG15:
		nTrialDirection = 3;
		nMaxTrial = STARTORIENT-3;
		break;
	case CPreferenceSet::LFIT_FULL:
	default:
		nTrialDirection = 1;
		break;
	}

	// Brute force method: step through an entire quadrant and see what
	// fits best.  Yuck.  Gotta be a better way to do this...
	// We define optimal orientation as that where width / height is minimum
	for (nTrial = nMaxTrial; nTrial >= 0 && nTrial < NUMORIENTS; nTrial += nTrialDirection)
	{
		// Get coordinates for this rotation
		a.RemoveAll();
		pFace->GetVertexCoordinates( NULL, 0, CLogPoint( 0, 0 ),
			dTrials[nTrial],
			a );

		// If the first pass, check for intersections
		if (FirstPass && pUnjoinedFace != NULL)
		{
			CDbg::Out( "AddFace(%s) - checking for collisions\n", (LPCTSTR)pFace->m_szFaceName );

			// If any collisions, this layout is a no-go
			int count = pUnjoinedFace->CollisionCount( this->m_pFaces );
			if (count > 0)
			{
				CDbg::Out( "Rejecting proposed layout - %d polygon collisions detected\n", count );
				return false;
			}
			CDbg::Out( "No collisions, adding face...\n" );
		}
		FirstPass = false;

		// Get orthogonal bounding rectangle
		GetMinMaxWidth( a, fgRect[nTrial] );

		// Save index entry
		idx.Add( nTrial );

		// Save orientation
		fgRect[nTrial].m_dOrientation = dTrials[nTrial];

		// Define post-rotation offset which will avoid any negative
		// x or y values
		fgRect[nTrial].SetOrigin( -fgRect[nTrial].GetMinX(), -fgRect[nTrial].GetMinY() );

		// Save start and end indices
		if (nMinTrial > nTrial)
		{
			nMinTrial = nTrial;
		}
		if (nMaxTrial < nTrial)
		{
			nMaxTrial = nTrial;
		}
		// If this is the second one, see if we're going in the wrong direction
		if (nTrial == STARTORIENT+nTrialDirection && nTrialDirection > 0)
		{
			if (fgRect[nTrial-1].GetWidthToHeight() < fgRect[nTrial].GetWidthToHeight())
			{
				nTrialDirection = -nTrialDirection;
				nTrial = STARTORIENT;
			}
		}

		// If no rotation specified, we're done
		if (!FirstPass && (LayoutFit == CPreferenceSet::LFIT_NOROTATE || LayoutFit == CPreferenceSet::LFIT_NEG45))
		{
			break;
		}

	}

	// Now find minimum and maximum
	double dMinRatio, dMaxRatio;
	int nMinRatio, nMaxRatio;
	nMinRatio = 0;
	nMaxRatio = 0;
	dMinRatio = 99999999999;
	dMaxRatio = 0;
	int nidx;
	for (nidx = 0; nidx < idx.GetSize(); nidx++)
	{
		nTrial = idx[nidx];
		double d = fgRect[nTrial].GetWidthToHeight();
		if (d < dMinRatio)
		{
			dMinRatio = d;
			nMinRatio = nTrial;
		}
		if (d > dMaxRatio)
		{
			dMaxRatio = d;
			nMaxRatio = nTrial;
		}
	}

	// Traverse free blocks until we find a fit
	int nSelected = nMinRatio;

	//CDbg::m_Level++;

	int nFreeBlock;
	for (nFreeBlock = 0; nFreeBlock < m_aFreeList.GetSize(); nFreeBlock++)
	{
		bool bFits = (fgRect[nSelected] <= m_aFreeList[nFreeBlock]);
		if (!bFits)
		{
			// Try rotating
			bFits = (fgRect[nMaxRatio] <= m_aFreeList[nFreeBlock]);
			if (bFits)
			{
				CDbg::Out( "Would fit rotated in free block %d, using %d\n", nFreeBlock, nMaxRatio );
				nSelected = nMaxRatio;
			}
			else
			{
				// Reject trial fit from layout if the page is blank and no fit
				if (!bAlwaysAddFirst || GetNumFaceGroups() > 0)
				{
					continue;
				}
				// else force this to be added!!!
				// Find best fit, which is not necessarily the smallest ratio
				int nTryFirst = nMinTrial;
				int nTryLast = nMaxTrial;
				if (nTryFirst > nTryLast)
				{
					nTryFirst = nMaxTrial;
					nTryLast = nMinTrial;
				}
				double dLeastHeightDelta = fgRect[nSelected].GetHeight() - m_aFreeList[nFreeBlock].GetHeight();
				double dLeastWidthDelta = fgRect[nSelected].GetWidth() - m_aFreeList[nFreeBlock].GetWidth();
				if (dLeastHeightDelta < 0) dLeastHeightDelta = 0;
				if (dLeastWidthDelta < 0) dLeastWidthDelta = 0;
				for (nTrial = nTryFirst; nTrial <= nTryLast; nTrial++)
				{
					double dHeightDelta = fgRect[nTrial].GetHeight() - m_aFreeList[nFreeBlock].GetHeight();
					double dWidthDelta = fgRect[nTrial].GetWidth() - m_aFreeList[nFreeBlock].GetWidth();
					if (dHeightDelta < 0) dHeightDelta = 0;
					if (dWidthDelta < 0) dWidthDelta = 0;
					if (dHeightDelta > dLeastHeightDelta || dWidthDelta > dLeastWidthDelta) continue;
					nSelected = nTrial;
					dLeastHeightDelta = dHeightDelta;
					dLeastWidthDelta = dWidthDelta;
				}
				CDbg::Out( "First time, forcing add of %s\n", (LPCTSTR)pFace->m_szFaceName );
			}
		}
		// Move our rectangle.  Free list entries always have org 0
		//CDbg::m_Level++;
		fgRect[nSelected].m_Org.m_dX += m_aFreeList[nFreeBlock].GetMin().m_dX;
		fgRect[nSelected].m_Org.m_dY += m_aFreeList[nFreeBlock].GetMin().m_dY;
		CDbg::Out( "Moved rect for face %s abs(%d) +%f, +%f\n",
			pFace->m_szFaceName, nFaceGroupIndex,
			m_aFreeList[nFreeBlock].GetMin().m_dX,
			m_aFreeList[nFreeBlock].GetMin().m_dY );
		CDbg::Out( "Face rect is h%f w%f rotated %f, free entry is h%f w%f\n",
			fgRect[nSelected].GetHeight(), fgRect[nSelected].GetWidth(), fgRect[nSelected].GetOrientation(),
			m_aFreeList[nFreeBlock].GetHeight(), m_aFreeList[nFreeBlock].GetWidth() );
		// Split free block
		CLogRect NewBlock( m_aFreeList[nFreeBlock] );
		m_aFreeList[nFreeBlock].m_Min.m_dX += fgRect[nSelected].GetWidth();
		m_aFreeList[nFreeBlock].m_Max.m_dY = m_aFreeList[nFreeBlock].m_Min.m_dY + fgRect[nSelected].GetHeight();
		CDbg::Out( "Split block %d: was [%f, %f - %f, %f],\n\t now [%f, %f - %f, %f] h%f w%f\n",
			nFreeBlock,
			NewBlock.m_Min.m_dX, NewBlock.m_Min.m_dY,
			NewBlock.m_Max.m_dX, NewBlock.m_Max.m_dY,
			m_aFreeList[nFreeBlock].m_Min.m_dX, m_aFreeList[nFreeBlock].m_Min.m_dY,
			m_aFreeList[nFreeBlock].m_Max.m_dX, m_aFreeList[nFreeBlock].m_Max.m_dY,
			m_aFreeList[nFreeBlock].GetHeight(), m_aFreeList[nFreeBlock].GetWidth()
			);
		NewBlock.m_Min.m_dY += fgRect[nSelected].GetHeight();
		m_aFreeList.Add( NewBlock );
		CDbg::Out( "Remainder block %d [%f, %f - %f, %f] h%f w%f\n",
			nFreeBlock+1,
			NewBlock.m_Min.m_dX, NewBlock.m_Min.m_dY,
			NewBlock.m_Max.m_dX, NewBlock.m_Max.m_dY,
			NewBlock.GetHeight(), NewBlock.GetWidth()
			);
		//CDbg::m_Level--;
		// Added successfully
		m_pFaces.Add( pFace );
		m_aFaceRect.Add( fgRect[nSelected] );
		m_aFaceIndices.Add( nFaceGroupIndex );
		// Get list of faces
		CMap<CFace*,CFace*,int,int> mapFaces;
		pFace->BuildConnectedFaceMap( mapFaces, TRUE );
		// Save reference to ourselves
		CFace* pf;
		int junk;
		for (POSITION pos = mapFaces.GetStartPosition(); pos != NULL; )
		{
			mapFaces.GetNextAssoc( pos, pf, junk );
			pf->SetPageGroup( this );
		}
		return true;
	} // Found a match

	// No room at the inn, sorry...
	//CDbg::m_Level--;
	return false;
	// Subtract from ourselves wherever there's room, rotating if need be
} // CPageGroup::AddFace()

// Get face with specified index
CFace*
CPageGroup::GetFaceGroup( int nGroupIndex ) const
{
	if (nGroupIndex < 0 || nGroupIndex >= m_pFaces.GetSize())
	{
		return NULL;
	}
	return m_pFaces[nGroupIndex];
} // CPageGroup::GetFaceGroup()

// Get layout rectangle for face group
bool
CPageGroup::GetFaceRect( int nGroupIndex, CLogRect& FaceRect ) const
{
	if (nGroupIndex < 0 || nGroupIndex >= m_aFaceRect.GetSize())
	{
		return false;
	}
	FaceRect = m_aFaceRect[nGroupIndex];
	return true;
} // CPageGroup::GetFaceRect()

// Get minimum x and y as well as width
void
CPageGroup::GetMinMaxWidth( CArray<CLogPoint,CLogPoint const&>& a, CLogRect& r )
{
	// Nothing to do if empty
	if (a.GetSize() < 1) return;

	// Find min and max
	r.m_Min = a[0];
	r.m_Max = a[0];

	// Nothing to do if singular
	if (a.GetSize() < 2) return;

	int n;
	for (n = 1; n < a.GetSize(); n++)
	{
		r.m_Min.SetToLesser( a[n] );
		r.m_Max.SetToGreater( a[n] );
	} // find min/max
	//dWidth = dMaxX - dMinX;
	//dHeight = dMaxY - dMinY;
	//CDbg::Out( "Got %d vertex coords, min x,y = %f, %f  max x,y = %f, %f, size %f, %f\r\n",
	//		ax.GetSize(), dMinX, dMinY, dMaxX, dMaxY, dWidth, dHeight );
} // CPageGroup::GetMinMaxWidth()

// Get absolute face group index
int
CPageGroup::GetAbsoluteIndexFor( int nGroupIndex ) const
{
	if (nGroupIndex < m_aFaceIndices.GetSize() && nGroupIndex >= 0)
	{
		return m_aFaceIndices[nGroupIndex];
	}
	ASSERT( false );
	return 0;
} // CPageGroup::GetAbsoluteIndexFor()

// Set self-reference for all faces in this group
void CPageGroup::SetSelfReference()
{
	int n;
	for (n = 0; n < this->m_pFaces.GetSize(); n++)
	{
		CFace* pFace = m_pFaces[n];
		pFace->SetPageGroup( this );
	}
}

// Set page number
void CPageGroup::SetPageNumber( int nPage )
{
	CDbg::Out( "CPageGroup::SetPageNumber(this=%08x)(%d) - old value = %d\n", (DWORD)this, nPage, m_PageNumber );
	m_PageNumber = nPage;
}

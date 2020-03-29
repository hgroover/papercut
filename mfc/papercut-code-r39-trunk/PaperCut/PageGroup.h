/*!

	@file	 PageGroup.h
	
	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: PageGroup.h 9 2006-03-08 14:41:10Z henry_groover $

  Page group represents a physical print page which has one more more face groups placed on it.
  This is used to fit layouts to pages. The page group maintains a free list of unused regions
  available for placing shapes. 
  TODO: Currently this object does NOT implement convex / concave fitting, which would provide
  more efficient use of paper.

*/

#if !defined(AFX_PAGEGROUP_H__7E443202_4471_11D6_A858_0040F4309CCE__INCLUDED_)
#define AFX_PAGEGROUP_H__7E443202_4471_11D6_A858_0040F4309CCE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxtempl.h>
#include "LogRect.h"
#include "Face.h"
#include "GlobalPreferences.h"

class CLogGroup;

class CPageGroup : public CLogRect  
{
public:
	CPageGroup(double dPageWidthInLogicalUnits, double dWidthOverHeight);
	virtual ~CPageGroup();

	// Add a face to this page along with absolute face group index.
	// Returns false if no room.  If bAlwaysAddFirst == false, don't add
	// if no room, even if the page is otherwise empty.
	bool AddFace( CFace* pFace, int nFaceGroupIndex, bool bAlwaysAddFirst = true, CFace* pUnjoinedFace = NULL, int LayoutFit = CPreferenceSet::LFIT_FULL );

	// Get face count
	int GetNumFaceGroups() const { return m_pFaces.GetSize(); }

	// Get face with specified index
	CFace* GetFaceGroup( int nGroupIndex ) const;

	// Get layout rectangle for face group
	bool GetFaceRect( int nGroupIndex, CLogRect& FaceRect ) const;

	// Get absolute face group index
	int GetAbsoluteIndexFor( int nGroupIndex ) const;

	// Get page number
	int GetPageNumber() const { return m_PageNumber; }

	// Set page number
	void SetPageNumber( int nPage );

	// Set self-reference for all faces in this group
	void SetSelfReference();

	// Will this face fit?
	//bool WillFit( CFace* pFace ) const;

protected:
	// Get minimum x and y as well as width
	static void GetMinMaxWidth( CArray<CLogPoint,CLogPoint const&>& a, CLogRect& r );

	// Page number from last layout
	int m_PageNumber;

	// Our faces - these are faces which may be connected to others.
	CArray<CFace*,CFace*> m_pFaces;

	// Original face group indices of our faces
	CArray<int,int> m_aFaceIndices;

	// Bounding rectangles and orientation with minimum
	// width / height ratio for each face group
	CArray<CLogRect,CLogRect const&> m_aFaceRect;

	// Free list for this page
	// We use a simple method for managing free blocks
	// The list starts out with a single entry (the entire page)
	// and is continuously split.
	CArray<CLogRect,CLogRect const&> m_aFreeList;
};

#endif // !defined(AFX_PAGEGROUP_H__7E443202_4471_11D6_A858_0040F4309CCE__INCLUDED_)

/*!

	@file	 MultiVertex.cpp
	
	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: MultiVertex.cpp 9 2006-03-08 14:41:10Z henry_groover $

  A multi-vertex object collects colocated vertices. The object hierarchy is 
  CShape{1} : CFace{many} : CVertex{many} : CEdge{2}
  where each vertex of a face points to two edges, and therefore each edge has
  exactly two references from vertices (we do not support unterminated edges).

*/

#include "stdafx.h"
#include "PaperCut.h"
#include "MultiVertex.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMultiVertex::CMultiVertex()
	:	m_aVertices(),
		m_szName()
{

}

CMultiVertex::~CMultiVertex()
{

}

// Congruency test for two vertices
bool
CMultiVertex::AreCongruent( CVertex* pv1, CVertex* pv2 )
{
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

	// First sanity check: do both have valid faces
	if (!pv1 || !pv2 || !pv1->m_pFace || !pv2->m_pFace)
	{
		return false;
	} // Error

	// [1] Do they belong to different faces?
	CFace* pf1 = pv1->m_pFace;
	CFace* pf2 = pv2->m_pFace;
	if (pf1 == pf2 ||
		pf1->m_szFaceName.Compare( pf2->m_szFaceName ) == 0)
	{
		return false;
	} // Same face

	// [2] Are faces joined?
	CEdge* pe = pf1->GetCommonEdge( pf2 );
	if (!pe)
	{
		return false;
	}

	// [3] Are both vertices connected?
	// [4] And are the orientations different?
	int nVRelation1 = pe->GetVertexRelation( pv1 );
	if (!nVRelation1)
	{
		nVRelation1 = pe->m_pOutwardConnectedEdge->GetVertexRelation( pv1 );
	}
	int nVRelation2 = pe->GetVertexRelation( pv2 );
	if (!nVRelation2)
	{
		nVRelation2 = pe->m_pOutwardConnectedEdge->GetVertexRelation( pv2 );
	}
	// Check for the case where neither is connected (fails [3])
	if (!(nVRelation1 && nVRelation2))
	{
		return false;
	}

	// Are orientations complimentary?
	if (nVRelation1 + nVRelation2 != 0)
	{
		return false;
	} // not complimentary

	// Passed all tests
	return true;

} // CMultiVertex::AreCongruent()

// Is vertex congruent with any in our list?
bool
CMultiVertex::IsCongruent( CVertex* pv )
{
	// First check for existing membership
	// Strictly speaking this is the identity function for
	// congruency - any object is congruent with itself -
	// but in our case we are checking for non-identity congruency.
	if (IsMember( pv ))
	{
		return false;
	} // If already a member then NOT congruent

	int n;
	int nSize = m_aVertices.GetSize();
	for (n = 0; n < nSize; n++)
	{
		if (AreCongruent( pv, m_aVertices[n] ))
		{
			return true;
		}
	} // for all members

	return false;
} // CMultiVertex::IsCongruent()

// Is vertex a member of our list?
bool
CMultiVertex::IsMember( CVertex* pv )
{
	// FIXME use a map for this
	int n;
	int nSize = m_aVertices.GetSize();
	for (n = 0; n < nSize; n++)
	{
		if (m_aVertices[n] == pv)
		{
			return true;
		} // Found a match
	} // for all members
	return false;
} // CMultiVertex::IsMember()

// Add vertex to our list
void
CMultiVertex::AddMember( CVertex* pv )
{
	if (IsMember( pv ))
	{
		return;
	} // Already present
	m_aVertices.Add( pv );
} // CMultiVertex::AddMember()

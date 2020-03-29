/*!

	@file	 MultiVertex.h
	
	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: MultiVertex.h 9 2006-03-08 14:41:10Z henry_groover $

  A multi-vertex object collects colocated vertices. The object hierarchy is 
  CShape{1} : CFace{many} : CVertex{many} : CEdge{2}
  where each vertex of a face points to two edges, and therefore each edge has
  exactly two references from vertices (we do not support unterminated edges).

*/

#if !defined(AFX_MULTIVERTEX_H__622AC241_3973_11D6_A858_0040F4309CCE__INCLUDED_)
#define AFX_MULTIVERTEX_H__622AC241_3973_11D6_A858_0040F4309CCE__INCLUDED_

#include "Vertex.h"

#include <afxtempl.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CVertex;

class CMultiVertex  
{
public:
	CMultiVertex();
	virtual ~CMultiVertex();

	// Congruency test for two vertices
	static bool AreCongruent( CVertex* pv1, CVertex* pv2 );

	// Is vertex congruent with any in our list?
	bool IsCongruent( CVertex* pv );

	// Is vertex a member of our list?
	bool IsMember( CVertex* pv );

	// Add vertex to our list
	void AddMember( CVertex* pv );

	CArray<CVertex*,CVertex*> m_aVertices;

	// Not currently used
	CString m_szName;
};

#endif // !defined(AFX_MULTIVERTEX_H__622AC241_3973_11D6_A858_0040F4309CCE__INCLUDED_)

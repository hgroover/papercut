/*!

	@file	 Edge.cpp

	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: Edge.cpp 16 2006-04-15 06:39:12Z henry_groover $

	Edge object. Edges are attached to vertices and belong to faces.

*/

#include "stdafx.h"
#include "PaperCut.h"
#include "Edge.h"
#include "Shape.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEdge::CEdge( CFace* pOwner )
	: m_szName()
{
	m_dLength = 1.0;
	m_dOrientation = 0.0; // North
	m_pEndPoints[0] = NULL;
	m_pEndPoints[1] = NULL;
	m_pOutwardConnectedEdge = NULL;
	m_bConcave = false;
	m_pFace = pOwner;
	m_forcedSplit = false;
}

CEdge::CEdge( CFace* pOwner, const char *pszName )
	: m_szName( pszName )
{
	m_dLength = 1.0;
	m_dOrientation = 0.0; // North
	m_pEndPoints[0] = NULL;
	m_pEndPoints[1] = NULL;
	m_pOutwardConnectedEdge = NULL;
	m_bConcave = false;
	m_pFace = pOwner;
	m_forcedSplit = false;
}

CEdge::CEdge( CEdge const& src )
{
	CDbg::Out( 2, "    CEdge::CEdge(CEdge&): creating edge %08x from %08x->%08x %s(%f)\n",
			(void*)this,
			(void*)src.m_pFace,
			(void*)&src,
			(LPCTSTR)src.GetFQName(),
			src.m_dLength );
	(*this) = src;
}

CEdge::~CEdge()
{

}

CEdge& CEdge::operator =(const CEdge &src)
{
	m_szName = src.m_szName;
	m_dLength = src.m_dLength;
	m_dOrientation = src.m_dOrientation;
	m_pEndPoints[0] = NULL; //src.m_pEndPoints[0];
	m_pEndPoints[1] = NULL; //src.m_pEndPoints[1];
	m_pOutwardConnectedEdge = NULL; //src.m_pOutwardConnectedEdge;
	m_szSavedOutwardEdge = src.m_szSavedOutwardEdge;
	m_pFace = src.m_pFace;
	m_szTabText = src.m_szTabText;
	m_bConcave = src.m_bConcave;
	m_forcedSplit = src.m_forcedSplit;
	return *this;
}

void CEdge::JoinOutward(CEdge *pOtherEdge, bool bSetReference /*= false*/)
{
	m_pOutwardConnectedEdge = pOtherEdge;
	m_szTabText.Empty();
	m_szSavedOutwardEdge.Empty();
	if (bSetReference) m_szOutwardConnectedEdge.Empty();
	if (pOtherEdge)
	{
		// Check for errors
		ASSERT( pOtherEdge->m_dLength == m_dLength );
		// Connect with this page group
		pOtherEdge->m_pFace->SetPageGroup( this->m_pFace->GetPageGroup() );
		// Use the same tab text for both - it is meant
		// to aid in assembly ;-)
		m_szTabText = pOtherEdge->GetFQName();
		// Always use lexically inferior value
		if (GetFQName() < m_szTabText)
		{
			m_szTabText = GetFQName();
		}
		m_szSavedOutwardEdge = pOtherEdge->GetFQName();
		pOtherEdge->m_pOutwardConnectedEdge = this;
		pOtherEdge->m_szTabText = m_szTabText;
		pOtherEdge->m_szSavedOutwardEdge = GetFQName();
		// Concave attribute takes precedence over non-concave
		if (m_bConcave) pOtherEdge->m_bConcave = m_bConcave;
		else if (pOtherEdge->m_bConcave) m_bConcave = true;
		if (bSetReference)
		{
			m_szOutwardConnectedEdge = pOtherEdge->GetFQName();
			// We only need one reference, and the other may be bogus, so remove it
			pOtherEdge->m_szOutwardConnectedEdge.Empty();
		}
	}
	CDbg::Out( "JoinOutward: Joined %s(%f) -> %s(%f), using tab name \"%s\"\n",
		(LPCTSTR)GetFQName(), m_dLength,
		pOtherEdge == NULL ? "NULL" : (LPCTSTR)pOtherEdge->GetFQName(),
		pOtherEdge == NULL ? 0.0 : pOtherEdge->m_dLength,
		(LPCTSTR)m_szTabText );
}

int CEdge::GetVertexRelation(CVertex *pv)
{

	if (pv == m_pEndPoints[0])
	{
		return -1;
	}
	if (pv == m_pEndPoints[1])
	{
		return 1;
	}
	// No relation
	return 0;
}

// Create symbolic representation
// "edge", "{", "name x", "owner x", "length x", "orient x", "concave 1|0",
// "outward face:edge", "start vertexname", "end vertexname"
// Returns comma-delimited list for debugging
CString
CEdge::SaveSymbolic( CArray<CString,const char*>& a ) const
{
	CString szReturn;
	CString szCvt;
	CString* pstr;

	a.Add( "object edge" );
	a.Add( "{" );

	MAKESYMBOLIC( "name", GetName() );
	MAKESYMBOLIC( "owner", m_pFace->m_szFaceName );
	MAKESYMBOLIC_CVT( "length", m_dLength );
	MAKESYMBOLIC_CVT( "orient", m_dOrientation );
	MAKESYMBOLIC_CVT( "concave", m_bConcave );
	// Write this only if non-default
	if (m_forcedSplit)
	{
		MAKESYMBOLIC_CVT( "forced-split", m_forcedSplit );
	}
	if (!m_pOutwardConnectedEdge)
	{
		MAKESYMBOLIC( "outward", CFace::m_strNull );
	}
	else
	{
		MAKESYMBOLIC( "outward", m_pOutwardConnectedEdge->GetFQName() );
	}
	MAKESYMBOLIC_NOTNULL( "start", m_pEndPoints[0], m_szVertexName );
	MAKESYMBOLIC_NOTNULL( "end", m_pEndPoints[1], m_szVertexName );

	// Close edge
	a.Add( "}" );

	return szReturn;

} // CEdge::SaveSymbolic()

CString
CEdge::GetFQName() const
{
	CString sz;
	sz.Format( "%s:%s",
		(const char*)m_pFace->m_szFaceName,
		(const char*)m_szName );
	return sz;
}

// Detach from outward join.  Return 1 if previously attached,
// 0 if not.
int
CEdge::Detach( bool bSaveSymbolic /*= false*/ )
{
	if (!m_pOutwardConnectedEdge)
	{
		return 0;
	} // Nothing to disconnect from
	if (bSaveSymbolic)
	{
		m_pOutwardConnectedEdge->m_szOutwardConnectedEdge = GetFQName();
		m_szOutwardConnectedEdge = m_pOutwardConnectedEdge->GetFQName();
	}
	// Always save this reference for tab connections
	m_szSavedOutwardEdge = m_pOutwardConnectedEdge->GetFQName();
	m_pOutwardConnectedEdge->m_szTabText.Empty();
	m_pOutwardConnectedEdge->m_pOutwardConnectedEdge = NULL;
	m_pOutwardConnectedEdge = NULL;
	return 1;
} // CEdge::Detach()

// Get fully qualified name of outward join if connected
// Return false if not
bool
CEdge::GetOuterJoinFQName( CString& szName ) const
{
	szName.Empty();
	if (m_pOutwardConnectedEdge == NULL)
	{
		return false;
	}
	szName = m_pOutwardConnectedEdge->GetFQName();
	return true;
} // CEdge::GetOuterJoinFQName()

// Parse a single element.  If bAllowExternalJoins, hook up
// outward connections (should be false on pass 1)
int CEdge::ParseElement(LPCTSTR lpElement, bool bAllowExternalJoins /*=true*/)
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
		this->m_szName = lpElement;
		return PE_ATTRIBUTE;
	}
	else if (!sz.CompareNoCase( "owner" ))
	{
		// Quietly ignore
		return PE_ATTRIBUTE;
	}
	else if (!sz.CompareNoCase( "length" ))
	{
		this->m_dLength = atof( lpElement );
		return PE_ATTRIBUTE;
	}
	else if (!sz.CompareNoCase( "orient" ))
	{
		this->m_dOrientation = atof( lpElement );
		return PE_ATTRIBUTE;
	}
	else if (!sz.CompareNoCase( "concave" ))
	{
		this->m_bConcave = (atoi( lpElement ) != 0);
		return PE_ATTRIBUTE;
	}
	else if (!sz.CompareNoCase( "forced-split" ))
	{
		this->m_forcedSplit = (atoi( lpElement ) != 0);
		return PE_ATTRIBUTE;
	}
	else if (!sz.CompareNoCase( "outward" ))
	{
		this->m_szOutwardConnectedEdge = lpElement;
		return PE_ATTRIBUTE;
	}
	else if (!sz.CompareNoCase( "start" ))
	{
		this->m_aszEndPoints[0] = lpElement;
		return PE_ATTRIBUTE;
	}
	else if (!sz.CompareNoCase( "end" ))
	{
		this->m_aszEndPoints[1] = lpElement;
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
		CDbg::Out( "Got unexpected object in %s:%u - %s %s\n",
			__FILE__,
			__LINE__,
			(LPCTSTR)sz,
			lpElement );
		return PE_OBJECT_UNKNOWN;
	}

	CDbg::Out( 0, "Got unexpected element in %s:%u - %s %s\n",
			__FILE__, __LINE__,
			(LPCTSTR)sz,
			lpElement );
	return PE_UNKNOWN;

}

// Restore from symbolic representation
int
CEdge::CreateFromSymbolic1( CArray<CString,const char*> const& a )
{
	// What we have:
	/****
    object edge
    {
      name c
      owner A
      length 1.000000
      orient 0.000000
      outward D:a
      start b.c
      end c.a
    }
	****/
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
		// Ignore all others
	}
	// Return number of elements processed
	return nLimit;
} // CEdge::CreateFromSymbolic1()

// Fixup symbolic names
void
CEdge::FixupSymbolic( CMap<CString,const char*,CVertex*,CVertex*> const& mapVertex )
{
	int n;
	for (n = 0; n < sizeof(m_aszEndPoints)/sizeof(m_aszEndPoints[0]); n++)
	{
		//m_pEndPoints[n] = NULL;
		if (m_aszEndPoints[n].IsEmpty())
		{
			continue;
		}
		if (!mapVertex.Lookup( m_aszEndPoints[n], m_pEndPoints[n] ))
		{
			CDbg::Out( 0, "Error: cannot find vertex %s\n", (LPCTSTR)m_aszEndPoints[n] );
			continue;
		}
		// Free it - it is no longer needed
		m_aszEndPoints[n].Empty();
	}
	// Fix up outward connected edge
	//this->m_pOutwardConnectedEdge = NULL;
	if (!m_szOutwardConnectedEdge.IsEmpty())
	{
		m_pOutwardConnectedEdge = m_pFace->m_pOwner->FindFQEdge( m_szOutwardConnectedEdge );
		// If a face has been deleted, the outward connecting edge may become empty
		//ASSERT( m_pOutwardConnectedEdge != NULL );
		if (m_pOutwardConnectedEdge == NULL)
		{
			m_szTabText.Empty();
		}
		else
		{
			if (m_pOutwardConnectedEdge->m_szOutwardConnectedEdge.IsEmpty())
			{
				m_pOutwardConnectedEdge->m_pOutwardConnectedEdge = this;
			}
			// Save as tab text iff other edge doesn't already have it
			if (m_pOutwardConnectedEdge->m_szTabText.IsEmpty())
			{
				m_szTabText = m_szOutwardConnectedEdge;
				m_pOutwardConnectedEdge->m_szTabText = m_szTabText;
			}
			else
			{
				m_szTabText = m_pOutwardConnectedEdge->m_szTabText;
			}
		} // Got outward connection
		// Free it - no longer needed
		m_szOutwardConnectedEdge.Empty();
	} // we have outward edge
} // CEdge::FixupSymbolic()

// Get next edge going clockwise
CEdge* 
CEdge::GetClockwiseNext() const {
	return m_pEndPoints[1]->m_pSides[1];
}

// Get next edge going counterclockwise
CEdge* 
CEdge::GetClockwisePrev() const {
	return m_pEndPoints[0]->m_pSides[0];
}

// Is this edge the base edge of its face?
bool CEdge::IsBaseEdge() const 
{ 
	return (this == m_pFace->GetBaseEdge()); 
}

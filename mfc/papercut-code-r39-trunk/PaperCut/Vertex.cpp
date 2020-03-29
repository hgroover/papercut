/*!

	@file	 Vertex.cpp
	
	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: Vertex.cpp 9 2006-03-08 14:41:10Z henry_groover $
  Vertex.cpp: implementation of the CVertex class.
*/

#include "stdafx.h"
#include "PaperCut.h"
#include "Vertex.h"
#include "Face.h"
#include "LogPoint.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CVertex::CVertex( CFace* pOwner )
	: m_szVertexName()
{
	m_pSides[0] = NULL;
	m_pSides[1] = NULL;
	m_dInsideAngle = 0.0;
	m_bPivot = false;
	m_pFace = pOwner;
	m_locSet = false;
	m_loc = new CLogPoint();
}

CVertex::CVertex( CFace* pOwner, const char *pszName )
	: m_szVertexName( pszName )
{
	m_pSides[0] = NULL;
	m_pSides[1] = NULL;
	m_dInsideAngle = 0.0;
	m_bPivot = false;
	m_pFace = pOwner;
	m_locSet = false;
	m_loc = new CLogPoint();
}

CVertex::CVertex( CVertex const& src )
{
	CDbg::Out( 2, "    CVertex::CVertex(CVertex&): creating vertex %08x from %08x->%08x\n",
			(void*)this,
			(void*)src.m_pFace,
			(void*)&src );
	m_loc = NULL;
	(*this) = src;
}

CVertex::~CVertex()
{
	if (m_loc != NULL)
	{
		delete m_loc;
		m_loc = NULL;
	}
}

CVertex& CVertex::operator =(const CVertex &src)
{
	m_pSides[0] = NULL; //src.m_pSides[0];
	m_pSides[1] = NULL; //src.m_pSides[1];
	m_dInsideAngle = src.m_dInsideAngle;
	m_bPivot = src.m_bPivot;
	m_szVertexName = src.m_szVertexName;
	m_pFace = src.m_pFace;
	if (m_loc != NULL)
	{
		delete m_loc;
		m_loc = NULL;
	}
	if (src.m_loc != NULL)
	{
		m_loc = new CLogPoint( *src.m_loc );
	}
	m_locSet = src.m_locSet;
	return *this;
}

// Create symbolic representation
// "vertex", "{", "name x", "owner facename", "pivot 0", "inside x",
// "leg0 edgename", "leg1 edgename"
// Returns comma-delimited list for debugging
CString
CVertex::SaveSymbolic( CArray<CString,const char*>& a ) const
{
	CString szReturn;
	CString szCvt;
	CString* pstr;

	a.Add( "object vertex" );
	a.Add( "{" );

	MAKESYMBOLIC( "name", m_szVertexName );
	MAKESYMBOLIC( "owner", m_pFace->m_szFaceName );
	MAKESYMBOLIC_CVT( "pivot", m_bPivot );
	MAKESYMBOLIC_CVT( "inside", m_dInsideAngle );
	MAKESYMBOLIC_NOTNULL( "leg0", m_pSides[0], m_szName );
	MAKESYMBOLIC_NOTNULL( "leg1", m_pSides[1], m_szName );

	a.Add( "}" );

	return szReturn;

} // CVertex::SaveSymbolic()

// Parse a single element.  If bAllowExternalJoins, hook up
// outward connections (should be false on pass 1)
int CVertex::ParseElement(LPCTSTR lpElement, bool bAllowExternalJoins /*=true*/)
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
		this->m_szVertexName = lpElement;
		return PE_ATTRIBUTE;
	}
	else if (!sz.CompareNoCase( "owner" ))
	{
		// Quietly ignore
		return PE_ATTRIBUTE;
	}
	else if (!sz.CompareNoCase( "pivot" ))
	{
		this->m_bPivot = (atoi( lpElement ) != 0);
		return PE_ATTRIBUTE;
	}
	else if (!sz.CompareNoCase( "inside" ))
	{
		this->m_dInsideAngle = atof( lpElement );
		return PE_ATTRIBUTE;
	}
	else if (!sz.Left(3).CompareNoCase( "leg" ))
	{
		int nLegIndex = atoi( sz.Mid(3) );
		if (nLegIndex < 0 || nLegIndex > 1)
		{
			return PE_ERROR;
		}
		this->m_aszEdge[nLegIndex] = lpElement;
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
			__FILE__,
			__LINE__,
			(LPCTSTR)sz, lpElement );
	return PE_UNKNOWN;

}

// Restore from symbolic representation
int
CVertex::CreateFromSymbolic1( CArray<CString,const char*> const& a )
{
	// What we have:
	/****
	object vertex
	{
		name a.b
		owner A
		pivot 0
		inside 60.000000
		leg0 a
		leg1 b
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
} // CVertex::CreateFromSymbolic1()

// Fixup symbolic names
void
CVertex::FixupSymbolic( CMap<CString,const char*,CEdge*,CEdge*> const& mapEdge )
{
	int n;
	for (n = 0; n < sizeof(m_aszEdge)/sizeof(m_aszEdge[0]); n++)
	{
		//m_pSides[n] = NULL;
		if (m_aszEdge[n].IsEmpty())
		{
			continue;
		}
		if (!mapEdge.Lookup( m_aszEdge[n], m_pSides[n] ))
		{
			CDbg::Out( 0, "Error: cannot find edge %s\n", (LPCTSTR)m_aszEdge[n] );
			continue;
		}
		// Free it - it is no longer needed
		m_aszEdge[n].Empty();
	}
} // CVertex::FixupSymbolic()


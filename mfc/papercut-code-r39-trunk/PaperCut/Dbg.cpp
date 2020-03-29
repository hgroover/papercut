/*!

	@file	 Dbg.cpp

	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: Dbg.cpp 9 2006-03-08 14:41:10Z henry_groover $

	Global object for debugging settings. Formats debug messages and outputs them based on
	debug level. We need a macro-based debug output method to eliminate unnecessary evaluation
	for release versions. Mostly the really time-consuming evaluations are done inside 
	if (CDbg::m_Level > 0)
	{
		...
	}
	constructs.

*/

#include "stdafx.h"
#include "PaperCut.h"
#include "Dbg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// Static initialization
int CDbg::m_Level = 0;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDbg::CDbg()
{

}

CDbg::~CDbg()
{

}

void CDbg::Out(const char *pszFmt, ... )
{
	if (m_Level < 1) return;
	char sz[4096];
	va_list marker;
	va_start( marker, pszFmt );
	_vsnprintf( sz, sizeof( sz ), pszFmt, marker );
	va_end( marker );
	::OutputDebugString( sz );
}

void CDbg::Out( int nMinLevel, const char *pszFmt, ...)
{
	if (m_Level < nMinLevel) return;
	char sz[4096];
	va_list marker;
	va_start( marker, pszFmt );
	_vsnprintf( sz, sizeof( sz ), pszFmt, marker );
	va_end( marker );
	::OutputDebugString( sz );
}

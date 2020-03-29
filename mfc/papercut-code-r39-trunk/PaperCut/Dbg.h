/*!

	@file	 Dbg.h

	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: Dbg.h 9 2006-03-08 14:41:10Z henry_groover $

	Global object for debugging settings. Formats debug messages and outputs them based on
	debug level. We need a macro-based debug output method to eliminate unnecessary evaluation
	for release versions. Mostly the really time-consuming evaluations are done inside 
	if (CDbg::m_Level > 0)
	{
		...
	}
	constructs.

*/

#if !defined(AFX_DBG_H__7902E181_3CFE_11D6_A858_0040F4309CCE__INCLUDED_)
#define AFX_DBG_H__7902E181_3CFE_11D6_A858_0040F4309CCE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CDbg  
{
public:
	CDbg();
	virtual ~CDbg();

	static void Out( const char *pszFmt, ... );
	static void Out( int nMinLevel /* default is 1 */, const char *pszFmt, ... );
	static int m_Level;
};

#endif // !defined(AFX_DBG_H__7902E181_3CFE_11D6_A858_0040F4309CCE__INCLUDED_)

/*!

	@file	 stdafx.h
	
	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: StdAfx.h 9 2006-03-08 14:41:10Z henry_groover $
  
  stdafx.h : include file for standard system include files,
  or project specific include files that are used frequently, but
      are changed infrequently
*/

#if !defined(AFX_STDAFX_H__9837ABAB_3851_11D6_A858_0040F4309CCE__INCLUDED_)
#define AFX_STDAFX_H__9837ABAB_3851_11D6_A858_0040F4309CCE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#include <pixielib.h>		// PixieLib

//#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes

#ifndef _AFX_NO_DB_SUPPORT
#include <afxdb.h>			// MFC ODBC database classes
#endif // _AFX_NO_DB_SUPPORT

#ifndef _AFX_NO_DAO_SUPPORT
#include <afxdao.h>			// MFC DAO database classes
#endif // _AFX_NO_DAO_SUPPORT

#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxsock.h>		// MFC socket extensions
#include <afxtempl.h>		// MFC collection classes

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

// Everybody uses this
#include "Dbg.h"

#endif // !defined(AFX_STDAFX_H__9837ABAB_3851_11D6_A858_0040F4309CCE__INCLUDED_)

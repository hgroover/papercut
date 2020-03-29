/*!

	@file	 PaperCut.h
	
	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: PaperCut.h 12 2006-03-10 06:10:30Z henry_groover $

  PaperCut application MFC execution class. Some global functionality which requires
  access to files present at startup in the same directory as the exe are also
  encapsulated here (such as the papercut.shapedef access functions).

*/

#if !defined(AFX_PAPERCUT_H__9837ABA8_3851_11D6_A858_0040F4309CCE__INCLUDED_)
#define AFX_PAPERCUT_H__9837ABA8_3851_11D6_A858_0040F4309CCE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include "GlobalPreferences.h"	// Added by ClassView

/////////////////////////////////////////////////////////////////////////////
// CPaperCutApp:
// See PaperCut.cpp for the implementation of this class
//

class CShape;
class CShapeDef;

class CPaperCutApp : public CWinApp
{
public:
	CGlobalPreferences m_prefs;


	CPaperCutApp();

	// Initialize shape from definitions or return false if not found
	bool InitShape( CShape& shp, LPCTSTR ShapeName );

	// Release shape definitions
	void ReleaseShapeDefs();

	// Re-read shape definitions
	void ReloadShapedefs();

	// Given an origin:0 ordinal, return the shape name
	CString GetShapeByOrdinal( int MenuOrdinal );

	// Put shape defs in menu
	void AddShapedefsToMenu( CMenu* pMenu, unsigned int CommandIDBase );

	// Get shape definition record from origin:0 ordinal
	CShapeDef* GetShapeDef( int Ordinal );

	CString GetAppVersion() const;
	//time_t GetAppVersionDate() const;
	CString GetAppVersionDate() const;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPaperCutApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	COleTemplateServer m_server;

		// Server object for document creation
	//{{AFX_MSG(CPaperCutApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	// Definitions read from papercut.shapedef at startup time
	CMap<CString,LPCTSTR,CShapeDef*,CShapeDef*> m_mapShapeDefs;

	// Submenus created via papercut.shapedefs. Key is path relative to Shape / Create
	CMap<CString,LPCTSTR,CMenu*,CMenu*> m_mapSubmenus;

	// Path from which shapedefs were actually read
	CString m_ShapeDefPath;

	// Array defining ordinal position of shapes in m_mapShapeDefs
	CArray<CString,LPCTSTR> m_aShapeNames;

	// Create ordinal list of shape names
	void CreateOrdinalList();

	// Create or find containing menu
	CMenu* FindOrCreateContainingMenu( CMenu* pMenu, CArray<CString,LPCTSTR>& aMenuNames );
};


#define MYAPP() ((CPaperCutApp*)::AfxGetApp())
#define MYPREFS() (MYAPP()->m_prefs)

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PAPERCUT_H__9837ABA8_3851_11D6_A858_0040F4309CCE__INCLUDED_)

/*!

	@file	 MainFrm.h
	
	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: MainFrm.h 15 2006-03-30 15:34:42Z henry_groover $

  The main MFC MDI frame window class handles all shape creation and global preference
  commands.

*/

#if !defined(AFX_MAINFRM_H__9837ABAD_3851_11D6_A858_0040F4309CCE__INCLUDED_)
#define AFX_MAINFRM_H__9837ABAD_3851_11D6_A858_0040F4309CCE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxext.h>

#include "DockDlg.h"

class CPaperCutDoc;

class CMainFrame : public CMDIFrameWnd
{
	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame();

// Attributes
public:

// Operations
public:

// Overrides
	// From CFrameWnd
	virtual void GetMessageString(   UINT nID,   CString& rMessage ) const;
// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void RecalcLayout(BOOL bNotify = TRUE);
	//}}AFX_VIRTUAL

// Implementation
public:
	double m_dLastWidthHeight;
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Get maximum number of shapes / page as set in spin control
	int GetMaxPage();
	
	// Set the maximum number of shapes per page to specified value
	int SetMaxPage( int nMax );

	// Get the current width / height ratio from page aspect dropdown
	double GetWidthOverHeight();

	// Get page width in logical units from slider
	double GetPageWidthInLogicalUnits();

	// Set page width in logical units
	void SetPageWidthInLogicalUnits( double d );

	// Get size ratio from slider. Size ratio is the percentage of a page's smallest dimension
	// (typically width) occupied by a unit.
	double GetSizeRatio();

	// Set size ratio
	void SetSizeRatio( double d );

	// Set width over height (and automatically set landscape)
	void SetPageWidthOverHeight( double d );

	// Load user-defined menu selections for window menu. Returns number of items added or replaced.
	static int LoadUserDefined( CWnd* wnd, bool& bLoaded, LPCTSTR szFile, bool bForceLoad = false );

protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;
	CToolBar    m_wndToolBar;
	CDockDlg	m_wndViewControlBar;

// Generated message map functions
protected:
	int m_nUntitledTetra;
	int m_nUntitledIcosa;
	int m_nUntitledPoly;
	bool m_bFireEvents;
	bool m_bMenuLoaded;
	CPaperCutDoc * OnShapeCreate( int& nUntitled, LPCTSTR lpszName, bool bStellate = false, double dRatio = 0.0, BOOL bKepler = FALSE, LPCTSTR lpSetupName = NULL );
	CPaperCutDoc * OnPolyShapeCreate( char cType, LPCTSTR lpszName );
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnShapeCreateTetra();
	afx_msg void OnShapeCreateIcosa();
	afx_msg void OnDbgOn();
	afx_msg void OnReleasedcaptureRotate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnLandscape();
	afx_msg void OnSelchangePageaspect();
	afx_msg void OnReleasedcaptureSizeratio(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnShapeCreateHex();
	afx_msg void OnShapeCreateOcta();
	afx_msg void OnGlobalPreferences();
	//}}AFX_MSG
	afx_msg BOOL OnToolTipNotify( UINT id, NMHDR * pNMHDR, LRESULT * pResult );
	afx_msg void OnClose();
	DECLARE_MESSAGE_MAP()
	// This did not do the trick
	//afx_msg void OnCustomdrawRotate(NMHDR* pNMHDR, LRESULT* pResult);
public:
	afx_msg void OnGlobalDictionary();
	afx_msg void OnShapeCreateKepler();
	afx_msg void OnCreateCube();
	afx_msg void OnShapeCreateDodeca();
	afx_msg void OnArchimedeanCuboctahedron();
	afx_msg void OnCompoundCubeOctahedron();
	afx_msg void OnArchimedeanTruncatedoctahedron();
	afx_msg void OnArchimedeanBuckyball();
	afx_msg void OnArchimedeanSnubcube();
	afx_msg void OnCompoundsCubeOctahedron();
	afx_msg void OnCompoundsIcosaDodeca();
	afx_msg void OnArchimedeanTruncatedtetrahedron();
	afx_msg void OnUpdateGlobalPreferences(CCmdUI *pCmdUI);
	afx_msg void OnShapedefCreate( UINT nID );
	afx_msg void OnShapeCreateTriangle();
	afx_msg void OnShapeCreateSquare();
	afx_msg void OnShapeCreatePentagon();
	afx_msg void OnShapeCreateHexagon();
	afx_msg void OnShapeCreateOctagon();
	afx_msg void OnShapeCreateDecagon();
	afx_msg void OnCreateEscher();
	afx_msg void OnImportOff();
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__9837ABAD_3851_11D6_A858_0040F4309CCE__INCLUDED_)

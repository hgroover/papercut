/*!

	@file	 PrefTabsDrawing.h
	
	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: PrefTabsDrawing.h 9 2006-03-08 14:41:10Z henry_groover $

  Drawing options property page from preferences (tabbed) dialog

*/

#if !defined(AFX_PREFTABSDRAWING_H__987E9C44_5638_11D6_A858_0040F4459482__INCLUDED_)
#define AFX_PREFTABSDRAWING_H__987E9C44_5638_11D6_A858_0040F4459482__INCLUDED_

#include "GlobalPreferences.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PrefTabsDrawing.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPrefTabsDrawing dialog

class CPrefTabsDrawing : public CPropertyPage
{
	DECLARE_DYNCREATE(CPrefTabsDrawing)

// Construction
public:
	void ResetAll();
	void SaveAll( CPreferenceSet* pSaveTo = NULL );
	void SetPrefs( CPreferenceSet* pPrefs );
	CPrefTabsDrawing();
	~CPrefTabsDrawing();

// Dialog Data
	//{{AFX_DATA(CPrefTabsDrawing)
	enum { IDD = IDD_PREFTABS_DRAWING };
	CStatic	m_lblScaleByOriginal;
	CStatic	m_lblScaleByHeight;
	CStatic	m_lblScaleByBase;
	CStatic	m_lblScaleByArea;
	CString	m_szDefaultText;
	int		m_ScaleBy;
	CString	m_szScaleByArea;
	CString	m_szScaleByBase;
	CString	m_szScaleByHeight;
	CString	m_szScaleByOriginal;
	BOOL	m_bFlowAdjoining;
	BOOL	m_bEnableRotation;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPrefTabsDrawing)
	public:
	virtual BOOL OnApply();
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CString m_szFontTypeface;
	unsigned long m_uFontAttributes;
	int m_nFontPointsize;
	COLORREF m_rgbTabEdge;
	COLORREF m_rgbInwdEdge;
	COLORREF m_rgbBgTab;
	COLORREF m_rgbBgFace;
	void EditColor( COLORREF& clr, int nControl );
	CPreferenceSet * m_pPrefs;
	COLORREF m_rgbEdge;
	COLORREF m_rgbFace;
	// Generated message map functions
	//{{AFX_MSG(CPrefTabsDrawing)
	afx_msg void OnColorFace();
	afx_msg void OnColorEdge();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnScalebyarea();
	virtual BOOL OnInitDialog();
	afx_msg void OnColorBgface();
	afx_msg void OnColorBgtab();
	afx_msg void OnColorInwdedge();
	afx_msg void OnColorTabedge();
	afx_msg void OnTextFont();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PREFTABSDRAWING_H__987E9C44_5638_11D6_A858_0040F4459482__INCLUDED_)

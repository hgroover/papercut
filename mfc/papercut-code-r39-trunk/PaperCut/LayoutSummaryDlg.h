/*!

	@file	 LayoutSummaryDlg.h

	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: LayoutSummaryDlg.h 9 2006-03-08 14:41:10Z henry_groover $

	Simple dialog to display a layout summary. This has become mostly irrelevant with the
	tab link hints, but could be useful for very large projects.
	TODO: Add other tabs and a division view as well.

*/

#if !defined(AFX_LAYOUTSUMMARYDLG_H__1CDD8761_4B2D_11D6_A858_0040F4309CCE__INCLUDED_)
#define AFX_LAYOUTSUMMARYDLG_H__1CDD8761_4B2D_11D6_A858_0040F4309CCE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LayoutSummaryDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLayoutSummaryDlg dialog

class CShapeLayout;

class CLayoutSummaryDlg : public CDialog
{
// Construction
public:
	void MakeTabSummary();
	void MakeFaceSummary();
	void MakePageSummary();
	void SetLayout( CShapeLayout* pLayout );
	CLayoutSummaryDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CLayoutSummaryDlg)
	enum { IDD = IDD_LAYOUT_SUMMARY };
	CEdit	m_txtSummary;
	int		m_ByFacePageTab;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLayoutSummaryDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void CreateSummary( int nWhich );
	CShapeLayout* m_pLayout;

	// Generated message map functions
	//{{AFX_MSG(CLayoutSummaryDlg)
	virtual void OnOK();
	afx_msg void OnByface();
	afx_msg void OnBypage();
	afx_msg void OnBytab();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LAYOUTSUMMARYDLG_H__1CDD8761_4B2D_11D6_A858_0040F4309CCE__INCLUDED_)

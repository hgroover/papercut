/*!

	@file	 DlgSelectStellation.h

	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: DlgSelectStellation.h 9 2006-03-08 14:41:10Z henry_groover $

	Dialog to select a custom stellation factor.

*/

#if !defined(AFX_DLGSELECTSTELLATION_H__0410EFE1_543F_11D6_A858_0040F4459482__INCLUDED_)
#define AFX_DLGSELECTSTELLATION_H__0410EFE1_543F_11D6_A858_0040F4459482__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgSelectStellation.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgSelectStellation dialog

class CDlgSelectStellation : public CDialog
{
// Construction
public:
	CDlgSelectStellation(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgSelectStellation)
	enum { IDD = IDD_SELECT_STELLATION };
	CString	m_Percentage;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgSelectStellation)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Persistent last selected percentage
	static CString m_szLastPercentage;

	// Generated message map functions
	//{{AFX_MSG(CDlgSelectStellation)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGSELECTSTELLATION_H__0410EFE1_543F_11D6_A858_0040F4459482__INCLUDED_)

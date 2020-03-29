/*!

	@file	 EditFace.h

	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: EditFace.h 9 2006-03-08 14:41:10Z henry_groover $

	Dialog to allow editing face text.

*/

#if !defined(AFX_EDITFACE_H__70E680C1_50B7_11D6_A858_0040F4459482__INCLUDED_)
#define AFX_EDITFACE_H__70E680C1_50B7_11D6_A858_0040F4459482__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CEditFace dialog

class CEditFace : public CDialog
{
// Construction
public:
	CEditFace(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEditFace)
	enum { IDD = IDD_EDIT_FACE };
	CString	m_szFaceText;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditFace)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEditFace)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITFACE_H__70E680C1_50B7_11D6_A858_0040F4459482__INCLUDED_)

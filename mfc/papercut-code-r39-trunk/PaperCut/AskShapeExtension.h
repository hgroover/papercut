/*!

	@file	 AskShapeExtension.h

	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: AskShapeExtension.h 9 2006-03-08 14:41:10Z henry_groover $

	Dialog to pop up at startup asking whether we should associate PaperCut with the .shp
	file extension.

*/

#if !defined(AFX_ASKSHAPEEXTENSION_H__CD30F241_428C_11D6_A858_0040F4309CCE__INCLUDED_)
#define AFX_ASKSHAPEEXTENSION_H__CD30F241_428C_11D6_A858_0040F4309CCE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AskShapeExtension.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAskShapeExtension dialog

class CAskShapeExtension : public CDialog
{
// Construction
public:
	CString m_szAppPath;
	CString m_szShortAppPath;
	CString m_szInstalledPath;
	bool ShouldAskToRegister();
	bool IsExtensionRegistered();
	CAskShapeExtension(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAskShapeExtension)
	enum { IDD = IDD_ASKSHAPE_EXTENSION };
	CButton	m_chkAlways;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAskShapeExtension)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAskShapeExtension)
	afx_msg void OnYes();
	afx_msg void OnNo();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ASKSHAPEEXTENSION_H__CD30F241_428C_11D6_A858_0040F4309CCE__INCLUDED_)

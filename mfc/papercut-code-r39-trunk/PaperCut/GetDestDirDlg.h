/*!

	@file	 GetDestDirDlg.h

	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: GetDestDirDlg.h 9 2006-03-08 14:41:10Z henry_groover $

	Get destination dir. This was never finished, and we use DirDialog.h instead.

*/

#pragma once
#include "afxwin.h"


// CGetDestDirDlg dialog

class CGetDestDirDlg : public CDialog
{
	DECLARE_DYNAMIC(CGetDestDirDlg)

public:
	CGetDestDirDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGetDestDirDlg();

	CString m_szPrompt;
	CString m_szDir;
	BOOL m_bOverwriteExisting;

// Dialog Data
	enum { IDD = IDD_GET_DESTINATION_DIR };

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CStatic m_lblPrompt;
	afx_msg void OnBnClickedBrowseDir();
	CEdit m_txtDir;
	afx_msg void OnBnClickedOk();
	CButton m_chkOverwriteExisting;
};

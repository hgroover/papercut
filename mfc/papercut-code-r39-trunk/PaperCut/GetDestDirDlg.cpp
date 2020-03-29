/*!

	@file	 GetDestDirDlg.cpp

	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: GetDestDirDlg.cpp 9 2006-03-08 14:41:10Z henry_groover $

	Get destination dir. This was never finished, and we use DirDialog.h instead.

*/

#include "stdafx.h"
#include "PaperCut.h"
#include "GetDestDirDlg.h"
#include "DirDialog.h"

#include ".\getdestdirdlg.h"


// CGetDestDirDlg dialog

IMPLEMENT_DYNAMIC(CGetDestDirDlg, CDialog)
CGetDestDirDlg::CGetDestDirDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGetDestDirDlg::IDD, pParent)
{
	m_bOverwriteExisting = FALSE;
}

CGetDestDirDlg::~CGetDestDirDlg()
{
}

void CGetDestDirDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROMPT, m_lblPrompt);
	DDX_Control(pDX, IDC_DESTINATION_DIR, m_txtDir);
	DDX_Control(pDX, IDC_OVERWRITE_EXISTING, m_chkOverwriteExisting);
}


BEGIN_MESSAGE_MAP(CGetDestDirDlg, CDialog)
	ON_BN_CLICKED(IDC_BROWSE_DIR, OnBnClickedBrowseDir)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()

BOOL CGetDestDirDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_lblPrompt.SetWindowText( m_szPrompt );
	m_txtDir.SetWindowText( m_szDir );
	m_chkOverwriteExisting.SetCheck( m_bOverwriteExisting ? 1 : 0 );

	return TRUE;
}

// CGetDestDirDlg message handlers

void CGetDestDirDlg::OnBnClickedBrowseDir()
{
	// Browse for dir - assume shape path
	CString szDir;
	m_txtDir.GetWindowText( szDir );
	CDirDialog dlg( szDir, NULL, this );
	if (dlg.DoModal() == IDOK)
	{
		m_txtDir.SetWindowText( dlg.GetPath() );
	}
}

void CGetDestDirDlg::OnBnClickedOk()
{
	// Save selection
	m_txtDir.GetWindowText( m_szDir );
	// Save checkbox state
	m_bOverwriteExisting = m_chkOverwriteExisting.GetCheck() != 0;
	OnOK();
}

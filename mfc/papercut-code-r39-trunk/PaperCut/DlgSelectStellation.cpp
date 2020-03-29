/*!

	@file	 DlgSelectStellation.cpp

	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: DlgSelectStellation.cpp 9 2006-03-08 14:41:10Z henry_groover $

	Dialog to select a custom stellation factor.

*/

#include "stdafx.h"
#include "PaperCut.h"
#include "DlgSelectStellation.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgSelectStellation dialog

CString CDlgSelectStellation::m_szLastPercentage = "100%";

CDlgSelectStellation::CDlgSelectStellation(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgSelectStellation::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgSelectStellation)
	m_Percentage = _T("");
	//}}AFX_DATA_INIT
	m_Percentage = m_szLastPercentage;
}


void CDlgSelectStellation::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgSelectStellation)
	DDX_CBString(pDX, IDC_STELLATION_PERCENTAGE, m_Percentage);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgSelectStellation, CDialog)
	//{{AFX_MSG_MAP(CDlgSelectStellation)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgSelectStellation message handlers

BOOL CDlgSelectStellation::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Set initial value
	UpdateData( FALSE );
	CWnd* pHelp = GetDlgItem( IDC_HELPBOX );
	if (pHelp)
	{
		CString szHelpText;
		szHelpText.LoadString( IDC_STELLATE_HELPBOX );
		pHelp->SetWindowText( szHelpText );
	}

	// If percentage value doesn't exist in dropdown, add it

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgSelectStellation::OnOK() 
{
	if (UpdateData())
	{
		// Save last percentage as next default
		CDlgSelectStellation::m_szLastPercentage = this->m_Percentage;
		CDialog::OnOK();
	}
}

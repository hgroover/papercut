/*!

	@file	 DlgInputScale.cpp
	
	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id$

  Implementation of simple input dialog

*/

#include "stdafx.h"
#include "PaperCut.h"
#include "DlgInputScale.h"


// DlgInputScale dialog

IMPLEMENT_DYNAMIC(DlgInputScale, CDialog)

DlgInputScale::DlgInputScale(CWnd* pParent /*=NULL*/)
	: CDialog(DlgInputScale::IDD, pParent)
{

}

DlgInputScale::~DlgInputScale()
{
}

void DlgInputScale::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(DlgInputScale, CDialog)
END_MESSAGE_MAP()


// DlgInputScale message handlers
BOOL DlgInputScale::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	// Set scale text
	CWnd *wndText = GetDlgItem( IDC_SCALE_TEXT );
	if (wndText) 
	{
		wndText->SetWindowTextA( m_scaleText );
		wndText->SetFocus();
		return FALSE;
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE

}

void DlgInputScale::OnOK()
{
	// Set scale text
	CWnd *wndText = GetDlgItem( IDC_SCALE_TEXT );
	if (wndText) 
	{
		wndText->GetWindowTextA( m_scaleText );
	}
}


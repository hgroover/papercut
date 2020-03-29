/*!

	@file	 DlgGetIndent.h
	
	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id$

  Implementation of dialog class to get parameters for face indent/outdent

*/
// DlgGetIndent.cpp : implementation file
//

#include "stdafx.h"
#include "PaperCut.h"
#include "DlgGetIndent.h"
#include ".\dlggetindent.h"


// CDlgGetIndent dialog

IMPLEMENT_DYNAMIC(CDlgGetIndent, CDialog)
CDlgGetIndent::CDlgGetIndent(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgGetIndent::IDD, pParent)
	, m_SideProportion(0)
	, m_HeightProportion(0)
{
}

CDlgGetIndent::~CDlgGetIndent()
{
}

void CDlgGetIndent::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SIDE_PROPORTION, m_ctlSide);
	DDX_Control(pDX, IDC_HEIGHT_PROPORTION, m_ctlHeight);
}

BOOL CDlgGetIndent::OnInitDialog()
{
	CDialog::OnInitDialog();

	UpdateData( FALSE );

	CString sz;
	sz.Format( "%7.5f", this->m_SideProportion );
	m_ctlSide.SetWindowText( sz );
	sz.Format( "%7.5f", this->m_HeightProportion );
	m_ctlHeight.SetWindowText( sz );

	return FALSE;
}

BEGIN_MESSAGE_MAP(CDlgGetIndent, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


// CDlgGetIndent message handlers

void CDlgGetIndent::OnBnClickedOk()
{
	// Get values
	this->UpdateData();
	// That doesn't work... get them manually
	CString sz;
	m_ctlSide.GetWindowText( sz );
	m_SideProportion = atof( sz );
	m_ctlHeight.GetWindowText( sz );
	m_HeightProportion = atof( sz );
	// Dismiss dialog
	OnOK();
	this->EndDialog( IDOK );
}

/*!

	@file	 GenericSummary.cpp
	
	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id$

  Implementation of generic summary dialog

*/

#include "stdafx.h"
#include "PaperCut.h"
#include "GenericSummary.h"


// GenericSummary dialog

IMPLEMENT_DYNAMIC(GenericSummary, CDialog)

GenericSummary::GenericSummary(CWnd* pParent /*=NULL*/)
	: CDialog(GenericSummary::IDD, pParent)
{

}

GenericSummary::~GenericSummary()
{
}

void GenericSummary::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(GenericSummary, CDialog)
END_MESSAGE_MAP()


// Set specified caption, title and text and return modal status
INT_PTR GenericSummary::ShowDialog( LPCTSTR caption, LPCTSTR title, LPCTSTR text )
{
	m_dlgCaption = caption;
	m_title = title;
	m_text = text;
	return DoModal();
}

BOOL GenericSummary::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Set caption
	SetWindowText( m_dlgCaption );

	// Set title text
	CWnd *wndTitle = GetDlgItem( IDC_SUMMARY_TITLE );
	if (wndTitle) wndTitle->SetWindowTextA( m_title );

	// Set main text
	CWnd *wndText = GetDlgItem( IDC_SUMMARY_TEXT );
	if (wndText) 
	{
		wndText->SetWindowTextA( m_text );
		wndText->SetFocus();
		return FALSE;
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

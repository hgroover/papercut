/*!

	@file	 EditFace.cpp

	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: EditFace.cpp 9 2006-03-08 14:41:10Z henry_groover $

	Dialog to allow editing face text.

*/

#include "stdafx.h"
#include "PaperCut.h"
#include "EditFace.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditFace dialog


CEditFace::CEditFace(CWnd* pParent /*=NULL*/)
	: CDialog(CEditFace::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditFace)
	m_szFaceText = _T("");
	//}}AFX_DATA_INIT
}


void CEditFace::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditFace)
	DDX_Text(pDX, IDC_FACETEXT, m_szFaceText);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditFace, CDialog)
	//{{AFX_MSG_MAP(CEditFace)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditFace message handlers

BOOL CEditFace::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Remove all ^
	m_szFaceText.Replace( "^\r\n", "\r\n" );

	// Initialize control
	UpdateData( FALSE );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEditFace::OnOK() 
{
	// Add ^ at end of lines
	int n, nStart = 0;
	UpdateData();
	while ((n = m_szFaceText.Find( "\r\n", nStart )) >= nStart)
	{
		m_szFaceText.Insert( n, '^' );
		nStart = n + 1 /* newly inserted character */
				   + 2 /* CR/LF */;
	}

	// We are overriding default behavior
	EndDialog( IDOK );
}

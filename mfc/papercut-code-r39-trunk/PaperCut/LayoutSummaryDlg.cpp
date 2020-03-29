/*!

	@file	 LayoutSummaryDlg.cpp

	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: LayoutSummaryDlg.cpp 9 2006-03-08 14:41:10Z henry_groover $

	Simple dialog to display a layout summary. This has become mostly irrelevant with the
	tab link hints, but could be useful for very large projects.
	TODO: Add other tabs and a division view as well.

*/

#include "stdafx.h"
#include "PaperCut.h"
#include "LayoutSummaryDlg.h"
#include "ShapeLayout.h"
#include "PageGroup.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLayoutSummaryDlg dialog


CLayoutSummaryDlg::CLayoutSummaryDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLayoutSummaryDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLayoutSummaryDlg)
	m_ByFacePageTab = 0;
	//}}AFX_DATA_INIT
	m_pLayout = NULL;
}


void CLayoutSummaryDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLayoutSummaryDlg)
	DDX_Control(pDX, IDC_SUMMARYTEXT, m_txtSummary);
	DDX_Radio(pDX, IDC_BYFACE, m_ByFacePageTab);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLayoutSummaryDlg, CDialog)
	//{{AFX_MSG_MAP(CLayoutSummaryDlg)
	ON_BN_CLICKED(IDC_BYFACE, OnByface)
	ON_BN_CLICKED(IDC_BYPAGE, OnBypage)
	ON_BN_CLICKED(IDC_BYTAB, OnBytab)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLayoutSummaryDlg message handlers

void CLayoutSummaryDlg::OnOK() 
{
	
	CDialog::OnOK();
}

void CLayoutSummaryDlg::OnByface() 
{
	if (!UpdateData()) return;
	CreateSummary( m_ByFacePageTab );
}

void CLayoutSummaryDlg::OnBypage() 
{
	if (!UpdateData()) return;
	CreateSummary( m_ByFacePageTab );
}

void CLayoutSummaryDlg::OnBytab() 
{
	if (!UpdateData()) return;
	CreateSummary( m_ByFacePageTab );
}

void CLayoutSummaryDlg::SetLayout(CShapeLayout *pLayout)
{
	m_pLayout = pLayout;
}

BOOL CLayoutSummaryDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Make sure we have a layout
	if (!m_pLayout)
	{
		AfxMessageBox( "No layout!" );
		EndDialog( IDCANCEL );
		return TRUE;
	}

	// Set up face summary by default
	CreateSummary( 0 );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CLayoutSummaryDlg::CreateSummary(int nWhich)
{

	switch (nWhich)
	{
	case 0:
		MakeFaceSummary();
		break;
	case 1:
		MakePageSummary();
		break;
	case 2:
		MakeTabSummary();
		break;
	}
}

void CLayoutSummaryDlg::MakePageSummary()
{
	CString sz;
	int nTotalPages = m_pLayout->GetPageCount();
	if (nTotalPages <= 0)
	{
		sz = "No pages to show!";
		m_txtSummary.SetWindowText( sz );
		return;
	}

	sz.Format( "Page summary for %d pages:\r\n-----------\r\n", nTotalPages );

	int nPage;
	CPageGroup* pPage;
	CString szTemp;
	for (nPage = 0; nPage < nTotalPages; nPage++)
	{
		pPage = m_pLayout->GetPage( nPage );
		ASSERT( pPage != NULL );
		szTemp.Format( "Page %d", nPage + 1 );
		sz += szTemp;
		int nGroupsInPage = pPage->GetNumFaceGroups();
		if (nGroupsInPage > 1)
		{
			sz += ":\r\n  ";
		}
		else
		{
			sz += ", ";
		}
		szTemp.Format( "face group %d:", nGroupsInPage );
		sz += szTemp;
		int nFaceGroup;
		for (nFaceGroup = 0; nFaceGroup < nGroupsInPage; nFaceGroup++)
		{
			CFace* pFace = pPage->GetFaceGroup( nFaceGroup );
			szTemp.Format( "    %s joins %s\r\n", (LPCTSTR)pFace->m_szFaceName, (LPCTSTR)pFace->GetTextJoinList() );
			sz += szTemp;
		}

	} // for all pages

	sz += "==============\r\n";
	m_txtSummary.SetWindowText( sz );

}

void CLayoutSummaryDlg::MakeFaceSummary()
{
	CString sz;
	int nTotalPages = m_pLayout->GetPageCount();
	if (nTotalPages <= 0)
	{
		sz = "No pages to show!";
		m_txtSummary.SetWindowText( sz );
		return;
	}

	int nTotalFaces = m_pLayout->m_pShape->GetNumFaces();
	sz.Format( "Face summary for %d faces:\r\n-----------\r\n", nTotalFaces );

	typedef CMap<CFace const*,CFace const*,int,int> FCMap_t;
	CArray<FCMap_t*,FCMap_t*> aMaps;
	CArray<int,int> aFaceGroupToPage;
	CArray<int,int> aGroupWithinPage;

	// Allocate array of maps
	int nTotalFaceGroups = m_pLayout->GetFaceGroupCount();
	int nPage;
	for (nPage = 0; nPage < nTotalFaceGroups; nPage++)
	{
		aMaps.Add( new FCMap_t );
	}

	CPageGroup* pPage;
	CString szTemp;
	int nFaceGroup = 0;
	for (nPage = 0; nPage < nTotalPages; nPage++)
	{
		pPage = m_pLayout->GetPage( nPage );
		ASSERT( pPage != NULL );

		int nGroupsInPage = pPage->GetNumFaceGroups();
		int nGroupInPage;
		for (nGroupInPage = 0; nGroupInPage < nGroupsInPage; nGroupInPage++, nFaceGroup++)
		{
			aFaceGroupToPage.Add( nPage + 1 );
			aGroupWithinPage.Add( nGroupInPage + 1 );
			CFace* pFace = pPage->GetFaceGroup( nGroupInPage );
			// Seed list with identity for current face to prevent looping
			aMaps[nFaceGroup]->SetAt( pFace, 1 );
			// Get count of faces
			pFace->GetJoinedFaces( *(aMaps[nFaceGroup]) );
		} // For all face groups in page

	} // for all pages

	// Now go through all faces in lexical order
	CArray<CFace*,CFace*> aFaces;
	nTotalFaces = m_pLayout->m_pShape->GetSortedFaces( aFaces );

	int nFace;
	for (nFace = 0; nFace < nTotalFaces; nFace++)
	{
		szTemp.Format( "%s\t", (LPCTSTR)aFaces[nFace]->m_szFaceName );
		sz += szTemp;
		for (nFaceGroup = 0; nFaceGroup < nTotalFaceGroups; nFaceGroup++)
		{
			int nCount;
			if (!aMaps[nFaceGroup]->Lookup( aFaces[nFace], nCount ))
			{
				continue;
			}
			// Get page
			szTemp.Format( " p%d.%d", aFaceGroupToPage[nFaceGroup], aGroupWithinPage[nFaceGroup] );
			sz += szTemp;
		} // for all face groups
		sz += "\r\n";
	} // for all faces

	// Free array of maps
	for (nPage = 0; nPage < nTotalFaceGroups; nPage++)
	{
		delete aMaps[nPage];
	}

	sz += "==============\r\n";
	m_txtSummary.SetWindowText( sz );

}

void CLayoutSummaryDlg::MakeTabSummary()
{
	m_txtSummary.SetWindowText( "Tab summary not implemented yet" );
}

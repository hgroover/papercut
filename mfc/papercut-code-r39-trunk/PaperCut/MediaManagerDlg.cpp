/*!

	@file	 MediaManagerDlg.cpp
	
	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: MediaManagerDlg.cpp 9 2006-03-08 14:41:10Z henry_groover $

  The media manager basically allows display and en masse copying of content (i.e. pictures)
  used in a shape to another location. This is useful when preparing shape files for upload.

*/

#include "stdafx.h"
#include "PaperCut.h"
#include "Shape.h"
#include "FaceContent.h"
#include "GetDestDirDlg.h"
#include ".\mediamanagerdlg.h"


// CMediaManagerDlg dialog

IMPLEMENT_DYNAMIC(CMediaManagerDlg, CDialog)
CMediaManagerDlg::CMediaManagerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMediaManagerDlg::IDD, pParent)
{
	m_pShape = NULL;
}

CMediaManagerDlg::~CMediaManagerDlg()
{
}

BOOL CMediaManagerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Populate list from shape
	if (m_pShape != NULL)
	{
		POSITION pos;
		CFace* pFace;
		CString szFaceName;
		CString szImagePath;
		CString szContentKey;
		CMap<CString,LPCTSTR,CFaceContent*,CFaceContent*> mapContent;
		CMap<CString,LPCTSTR,int,int> mapContentInstance;
		int UniqueContent = 0;
		for (pos = m_pShape->m_mapFaces.GetStartPosition(); pos != NULL; )
		{
			m_pShape->m_mapFaces.GetNextAssoc( pos, szFaceName, pFace );
			CFaceContent* pContent = pFace->m_pContent;
			if (pContent != NULL)
			{
				szImagePath = pContent->GetPath();
				if (!szImagePath.IsEmpty())
				{
					int Index = this->m_lstMedia.AddString( szImagePath );
					this->m_lstMedia.SetItemData( Index, (DWORD_PTR)pFace );
					szContentKey.Format( "%08x", (DWORD)pContent );
					if (mapContent.Lookup( szContentKey, pContent ))
					{
						mapContentInstance[szContentKey]++;
					}
					else
					{
						mapContent.SetAt( szContentKey, pFace->m_pContent );
						mapContentInstance.SetAt( szContentKey, 1 );
						UniqueContent++;
					}
				}
			}
		}
		if (UniqueContent > 0)
		{
			CString szSummary;
			szSummary.Format( "%d unique content entries", UniqueContent );
			this->m_txtNotes.SetWindowText( szSummary );
		}
	}

	return FALSE;
}

void CMediaManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MEDIA_LIST, m_lstMedia);
	DDX_Control(pDX, IDC_MEDIALIST_NOTES, m_txtNotes);
}


BEGIN_MESSAGE_MAP(CMediaManagerDlg, CDialog)
	ON_BN_CLICKED(IDC_COPY_CONTENT, OnBnClickedCopyContent)
END_MESSAGE_MAP()


// CMediaManagerDlg message handlers

void CMediaManagerDlg::OnBnClickedCopyContent()
{
	// Copy selected
	int NumSelected = m_lstMedia.GetSelCount();
	if (NumSelected < 1)
	{
		::AfxMessageBox( "No entries selected" );
		return;
	}
	// Get destination
	CGetDestDirDlg dlgGetDest;
	dlgGetDest.m_szPrompt = "Enter destination directory for media files:";
	char szDrive[1024], szDir[1024];
	_splitpath( m_pShape->m_SavedPath, szDrive, szDir, NULL, NULL );
	dlgGetDest.m_szDir = szDrive;
	dlgGetDest.m_szDir += szDir;
	if (dlgGetDest.DoModal() != IDOK)
		return;
	if (dlgGetDest.m_szDir.IsEmpty())
		return;
	int *aSelected = new int[NumSelected];
	int TotalFilesCopied = 0, TotalBytesCopied = 0;
	m_lstMedia.GetSelItems( NumSelected, aSelected );
	CString szErrorSummary;
	int ErrorCount = 0;
	for (int n = 0; n < NumSelected; n++)
	{
		int Selected = aSelected[n];
		CFace* pFace = (CFace*)m_lstMedia.GetItemData( Selected );
		if (pFace != NULL)
		{
			// Copy actual file
			// Reset path in content object
			// ...iff path is different
			int BytesCopied = pFace->m_pContent->CopyContentFile( dlgGetDest.m_szDir, dlgGetDest.m_bOverwriteExisting );
			if (BytesCopied <= 0)
			{
				CString szMsg;
				szMsg.Format( "Error: Failed to copy %s to %s\r\n", (LPCTSTR)pFace->m_pContent->GetPath(), (LPCTSTR)dlgGetDest.m_szDir );
				szErrorSummary += szMsg;
				ErrorCount++;
				//::AfxMessageBox( szMsg );
			}
			else if (BytesCopied > 0)
			{
				TotalFilesCopied++;
				TotalBytesCopied += BytesCopied;
			}
		}
	}
	delete [] aSelected;
	if (ErrorCount > 0)
	{
		::AfxMessageBox( szErrorSummary );
	}
	CString szSummary;
	szSummary.Format( "%d files copied - total bytes = %u", TotalFilesCopied, TotalBytesCopied );
	this->m_txtNotes.SetWindowText( szSummary );
}

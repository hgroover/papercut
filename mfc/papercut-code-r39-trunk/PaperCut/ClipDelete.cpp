/*!

	@file	 ClipDelete.cpp

	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: ClipDelete.cpp 9 2006-03-08 14:41:10Z henry_groover $

	Cut or delete dialog used by division split dialog.

*/

#include "stdafx.h"
#include "PaperCut.h"
#include "ClipDelete.h"
#include ".\clipdelete.h"
#include "Shape.h"
#include "Face.h"


// CClipDelete dialog

IMPLEMENT_DYNAMIC(CClipDelete, CDialog)
CClipDelete::CClipDelete(CWnd* pParent /*=NULL*/)
	: CDialog(CClipDelete::IDD, pParent)
{
	m_bClip = false;
	m_pShape = NULL;
}

CClipDelete::~CClipDelete()
{
}

BOOL CClipDelete::OnInitDialog()
{
	CDialog::OnInitDialog();
	if (m_pShape == NULL)
	{
		this->EndDialog(IDCANCEL);
		return FALSE;
	}
	CString sz;
	this->m_lblIntro.GetWindowText( sz );
	m_pShape->Substitute( sz, (int)m_pShape->GetFirstFace(), 0 );
	this->m_lblIntro.SetWindowText( sz );

	// Update controls with data
	UpdateData( FALSE );
	// Update control vars
	UpdateData( TRUE );

	this->InitTree();

	// Give tree the focus
	m_treeFaces.SetFocus();

	// Redraw select list to show nothing selected
	RedrawSelectList();

	return TRUE; // Yes, we set focus to one of our controls...
}

void CClipDelete::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FACE_TREE, m_treeFaces);
	DDX_Control(pDX, IDC_SHAPE_CLIP, m_btnClip);
	DDX_Control(pDX, IDC_SHAPE_DELETE, m_btnDelete);
	DDX_Control(pDX, IDC_INTRO, m_lblIntro);
	DDX_Control(pDX, IDC_GROUP_LIST, m_lblGroupList);
	DDX_Control(pDX, IDC_RESET_SELECT, m_btnReset);
}


BEGIN_MESSAGE_MAP(CClipDelete, CDialog)
	ON_BN_CLICKED(IDC_SHAPE_CLIP, OnBnClickedShapeClip)
	ON_BN_CLICKED(IDC_SHAPE_DELETE, OnBnClickedShapeDelete)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_NOTIFY(TVN_SELCHANGED, IDC_FACE_TREE, OnTvnSelchangedFaceTree)
	ON_BN_CLICKED(IDC_RESET_SELECT, OnBnClickedResetSelect)
END_MESSAGE_MAP()


// CClipDelete message handlers

void CClipDelete::OnBnClickedShapeClip()
{
	// If previous sense was to delete, remove selections
	if (!m_bClip) m_aFaces.RemoveAll();
	// Clip to selected faces
	m_bClip = true;
	HTREEITEM htSel = m_treeFaces.GetSelectedItem();
	AddItemToRemoveList( htSel );
	RedrawSelectList();
}

void CClipDelete::OnBnClickedShapeDelete()
{
	// If previous sense was to clip, remove selections
	if (m_bClip) m_aFaces.RemoveAll();
	// Delete selected faces
	m_bClip = false;
	HTREEITEM htSel = m_treeFaces.GetSelectedItem();
	AddItemToRemoveList( htSel );
	RedrawSelectList();
}

void CClipDelete::AddItemToRemoveList( HTREEITEM htSel )
{
	if (htSel != NULL)
	{
		if (m_treeFaces.ItemHasChildren( htSel ))
		{
			// Group - get first child
			HTREEITEM htChild = m_treeFaces.GetChildItem( htSel );
			HTREEITEM htNext;

			while (htChild != NULL)
			{
				htNext = m_treeFaces.GetNextItem( htChild, TVGN_NEXT );
				AddItemToRemoveList( htChild );
				htChild = htNext;
			}
		}
		else
		{
			// Face
			CFace* pFace = (CFace*)m_treeFaces.GetItemData( htSel );
			if (pFace != NULL)
			{
				m_aFaces.Add( pFace );
			}
		}
	}
}

void CClipDelete::OnBnClickedCancel()
{
	// Just close
	OnCancel();
}

void CClipDelete::OnBnClickedOk()
{
	if (this->m_aFaces.GetSize() > 0)
	{
		if (m_bClip)
		{
			// Build hash map of faces to preserve
			CMap<CString,LPCTSTR,int,int> mapSave;
			int n;
			for (n = 0; n < m_aFaces.GetSize(); n++)
			{
				mapSave.SetAt(m_aFaces[n]->m_szFaceName, n);
			}
			// Build inverse list
			CArray<CFace*,CFace*> aInverse;
			CString szName;
			CFace* pFace;
			for (POSITION pos = m_pShape->m_mapFaces.GetStartPosition(); pos != NULL; )
			{
				m_pShape->m_mapFaces.GetNextAssoc( pos, szName, pFace );
				if (!mapSave.Lookup( szName, n ))
				{
					aInverse.Add(pFace);
				}
			}
			m_aFaces.RemoveAll();
			// Remove faces
			RemoveFaces( aInverse );
		}
		else
		{
			RemoveFaces( m_aFaces );
		}
	}
	OnOK();
}

// Remove faces in array
void CClipDelete::RemoveFaces( CArray<CFace*,CFace*>& aDeleteList )
{
	// Commit actual changes to shape
	m_pShape->CutAllFaceJoins( true );
	for (int n = 0; n < aDeleteList.GetSize(); n++)
	{
		m_pShape->DeleteFace( aDeleteList[n]->m_szFaceName );
		//delete aDeleteList[n];
		aDeleteList[n] = NULL;
	}
	aDeleteList.RemoveAll();
	m_pShape->FixupSymbolic();
}

void CClipDelete::OnTvnSelchangedFaceTree(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// Enable clip and delete
	this->m_btnClip.EnableWindow();
	this->m_btnDelete.EnableWindow();
	*pResult = 0;
}

// Set up tree from shape
void CClipDelete::InitTree()
{
	//this->m_treeFaces.DeleteAllItems();
	// Every face should have the same number of groups.
	// Groups should be in correct hierarchical order from left to right.
	POSITION pos;
	CString szName;
	CFace *pFace;
	HTREEITEM htRoot = m_treeFaces.InsertItem( m_pShape->GetName() );
	for (pos = m_pShape->m_mapFaces.GetStartPosition(); pos != NULL; )
	{
		m_pShape->m_mapFaces.GetNextAssoc( pos, szName, pFace );
		CString szDbg, szAdd;
		CArray<CString,LPCTSTR> aGroups;
		pFace->GetGroups(aGroups);
		HTREEITEM htGroup = htRoot;
		// Traverse tree from root on down to insert everything in order
		for (int n = 0; n < aGroups.GetSize(); n++)
		{
			// If group has children, traverse until we find one lexically greater or end
			if (m_treeFaces.ItemHasChildren( htGroup ))
			{
				HTREEITEM htSeek, htNext;
				htSeek = m_treeFaces.GetChildItem( htGroup );
				while (htSeek != NULL)
				{
					// Get next
					htNext = m_treeFaces.GetNextItem( htSeek, TVGN_NEXT );
					// If lexically inferior, insert before htSeek
					// If found, go on to next level
					CString szCurrent = m_treeFaces.GetItemText( htSeek );
					int nCompare = aGroups[n].CompareNoCase( szCurrent );
					if (nCompare == 0) 
					{
						htGroup = htSeek;
						break;
					}
					if (nCompare < 0)
					{
						htGroup = m_treeFaces.InsertItem( aGroups[n], htGroup, TVI_FIRST );
						break;
					}
					// If next is null, insert here
					if (htNext == NULL)
					{
						htGroup = m_treeFaces.InsertItem( aGroups[n], htGroup, TVI_LAST );
						break;
					}
					// If next is lexically greater, insert here
					szCurrent = m_treeFaces.GetItemText( htNext );
					nCompare = aGroups[n].CompareNoCase( szCurrent );
					if (nCompare < 0)
					{
						htGroup = m_treeFaces.InsertItem( aGroups[n], htGroup, htSeek );
						break;
					}
					// else continue
					htSeek = htNext;
				}
			}
			else
			{
				htGroup = m_treeFaces.InsertItem( aGroups[n], htGroup, TVI_LAST );
			}
			szAdd.Format( "%s/", (LPCTSTR)aGroups[n] );
			szDbg += szAdd;
		}
		szDbg += szName;
		// htGroup should be the correct parent
		HTREEITEM ht = m_treeFaces.InsertItem( szName, htGroup, TVI_SORT );
		m_treeFaces.SetItemData( ht, (DWORD_PTR)pFace );
		CDbg::Out( 1, "Face group: %s\n", (LPCTSTR)szDbg );
	}
}


void CClipDelete::OnBnClickedResetSelect()
{
	this->m_aFaces.RemoveAll();
	RedrawSelectList();
}

// Redraw selected list
void CClipDelete::RedrawSelectList()
{
	CString szText;
	if (m_aFaces.GetSize() == 0)
	{
		szText = "Nothing selected";
		m_btnReset.EnableWindow( FALSE );
	}
	else
	{
		if (this->m_bClip)
			szText = "Deleting all EXCEPT: ";
		else
			szText = "Deleting: ";
		for (int n = 0; n < 100 && n < m_aFaces.GetSize(); n++)
		{
			if (n > 0) szText += ", ";
			szText += m_aFaces[n]->m_szFaceName;
		}
		m_btnReset.EnableWindow( TRUE );
	}
	this->m_lblGroupList.SetWindowText( szText );
}


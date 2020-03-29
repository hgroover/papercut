/*!

	@file	 LayoutSplit.cpp

	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: LayoutSplit.cpp 9 2006-03-08 14:41:10Z henry_groover $

	Layout splitting dialog. This allows creation of new divisions and moving faces and
	groups of faces between divisions.

*/

#include "stdafx.h"
#include "PaperCut.h"
#include "ShapeLayout.h"
#include "LayoutSplit.h"
#include ".\layoutsplit.h"


// CLayoutSplit dialog

IMPLEMENT_DYNAMIC(CLayoutSplit, CDialog)
CLayoutSplit::CLayoutSplit(CWnd* pParent /*=NULL*/)
	: CDialog(CLayoutSplit::IDD, pParent)
{
	m_pShapeLayout = NULL;
	m_szNextDivision = "B";
}

CLayoutSplit::~CLayoutSplit()
{
}

void CLayoutSplit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FACE_TREE, m_tree);
	DDX_Control(pDX, IDC_INTRO, m_lblIntro);
	DDX_Control(pDX, IDC_GROUP_LIST, m_lblList);
	DDX_Control(pDX, IDC_SET_DESTINATION, m_btnSetDest);
	DDX_Control(pDX, IDC_MOVE_NODE, m_btnMove);
}


BEGIN_MESSAGE_MAP(CLayoutSplit, CDialog)
	ON_BN_CLICKED(IDC_NEW_DIVISION, OnBnClickedNewDivision)
	ON_NOTIFY(TVN_BEGINDRAG, IDC_FACE_TREE, OnTvnBegindragFaceTree)
	ON_NOTIFY(TVN_SELCHANGED, IDC_FACE_TREE, OnTvnSelchangedFaceTree)
	ON_BN_CLICKED(IDC_SET_DESTINATION, OnBnClickedSetDestination)
	ON_BN_CLICKED(IDC_MOVE_NODE, OnBnClickedMoveNode)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


// CLayoutSplit message handlers

void CLayoutSplit::OnBnClickedNewDivision()
{
	// Add new root division
	HTREEITEM htNew = m_tree.InsertItem( this->m_szNextDivision );
	this->IncrementDefaultDivision();
	// Set focus to tree control and highlight new item
	m_tree.SetFocus();
	m_tree.SelectItem( htNew );
}

// Increment default division name
void CLayoutSplit::IncrementDefaultDivision()
{
	// If < "Z" get ordinal successor
	if (m_szNextDivision < "Z")
	{
		m_szNextDivision.Format( "%c", m_szNextDivision[0] + 1 );
	}
	else
	{
		int Numeric = atoi( m_szNextDivision.Mid( 1 ) ) + 1;
		m_szNextDivision.Format( "Z%03d", Numeric );
	}
}

BOOL CLayoutSplit::OnInitDialog()
{
	CDialog::OnInitDialog();
	if (this->m_pShapeLayout == NULL)
	{
		this->EndDialog(IDCANCEL);
		return FALSE;
	}
	CString sz;
	CShape* pShape = m_pShapeLayout->m_pShape;
	this->m_lblIntro.GetWindowText( sz );
	pShape->Substitute( sz, (int)pShape->GetFirstFace(), 0 );
	this->m_lblIntro.SetWindowText( sz );

	// Update controls with data
	UpdateData( FALSE );
	// Update control vars
	UpdateData( TRUE );

	this->InitTree();

	// Give tree the focus
	m_tree.SetFocus();

	return TRUE; // Yes, we set focus to one of our controls...
}

// Set up tree from shape
void CLayoutSplit::InitTree()
{
	//this->m_treeFaces.DeleteAllItems();
	// Every face should have the same number of groups.
	// Groups should be in correct hierarchical order from left to right.
	POSITION pos;
	CString szName;
	CFace *pFace;
	HTREEITEM htRoot = m_tree.InsertItem( "<default>" );
	this->mapDivisionItems.SetAt( "<default>", htRoot );
	this->mapDivisionItems.SetAt( "", htRoot );
	CShape* pShape = m_pShapeLayout->m_pShape;
	for (pos = pShape->m_mapFaces.GetStartPosition(); pos != NULL; )
	{
		pShape->m_mapFaces.GetNextAssoc( pos, szName, pFace );
		CString szDbg, szAdd;
		CArray<CString,LPCTSTR> aGroups;
		pFace->GetGroups(aGroups);
		HTREEITEM htGroup = htRoot;
		// Check division
		CString szDivision = pFace->GetLayoutDivision();
		if (!szDivision.IsEmpty())
		{
			if (!mapDivisionItems.Lookup( szDivision, htGroup ))
			{
				htGroup = m_tree.InsertItem( szDivision );
				mapDivisionItems.SetAt( szDivision, htGroup );
			}
		}
		// Traverse tree from division root on down to insert everything in order
		for (int n = 0; n < aGroups.GetSize(); n++)
		{
			// If group has children, traverse until we find one lexically greater or end
			if (m_tree.ItemHasChildren( htGroup ))
			{
				HTREEITEM htSeek, htNext;
				htSeek = m_tree.GetChildItem( htGroup );
				while (htSeek != NULL)
				{
					// Get next
					htNext = m_tree.GetNextItem( htSeek, TVGN_NEXT );
					// If lexically inferior, insert before htSeek
					// If found, go on to next level
					CString szCurrent = m_tree.GetItemText( htSeek );
					int nCompare = aGroups[n].CompareNoCase( szCurrent );
					if (nCompare == 0) 
					{
						htGroup = htSeek;
						break;
					}
					if (nCompare < 0)
					{
						htGroup = m_tree.InsertItem( aGroups[n], htGroup, TVI_FIRST );
						break;
					}
					// If next is null, insert here
					if (htNext == NULL)
					{
						htGroup = m_tree.InsertItem( aGroups[n], htGroup, TVI_LAST );
						break;
					}
					// If next is lexically greater, insert here
					szCurrent = m_tree.GetItemText( htNext );
					nCompare = aGroups[n].CompareNoCase( szCurrent );
					if (nCompare < 0)
					{
						htGroup = m_tree.InsertItem( aGroups[n], htGroup, htSeek );
						break;
					}
					// else continue
					htSeek = htNext;
				}
			}
			else
			{
				htGroup = m_tree.InsertItem( aGroups[n], htGroup, TVI_LAST );
			}
			szAdd.Format( "%s/", (LPCTSTR)aGroups[n] );
			szDbg += szAdd;
		}
		szDbg += szName;
		// htGroup should be the correct parent
		HTREEITEM ht = m_tree.InsertItem( szName, htGroup, TVI_SORT );
		m_tree.SetItemData( ht, (DWORD_PTR)pFace );
		CDbg::Out( 1, "Face group: %s\n", (LPCTSTR)szDbg );
	}
}

void CLayoutSplit::OnTvnBegindragFaceTree(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
}

void CLayoutSplit::OnTvnSelchangedFaceTree(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// Get current item
	HTREEITEM htCurrent = pNMTreeView->itemNew.hItem;
	// Is it a root entry?
	BOOL IsRoot = (m_tree.GetParentItem( htCurrent ) == NULL);
	// Enable select destination button
	this->m_btnSetDest.EnableWindow( IsRoot );
	// If not root, destination set, and either a face or has children, enable move button
	BOOL CanMove = (m_htMoveDest != NULL && !IsRoot && 
		(m_tree.GetItemData( htCurrent ) != 0 || m_tree.ItemHasChildren( htCurrent )));
	this->m_btnMove.EnableWindow( CanMove );
	*pResult = 0;
}

void CLayoutSplit::OnBnClickedSetDestination()
{
	m_htMoveDest = m_tree.GetSelectedItem();
	// Only root is allowed as a destination
	if (m_tree.GetParentItem( m_htMoveDest ) != NULL)
	{
		m_htMoveDest = NULL;
		m_btnMove.EnableWindow( FALSE );
	}
}

void CLayoutSplit::OnBnClickedMoveNode()
{
	// Get selected item
	HTREEITEM htCurrent = m_tree.GetSelectedItem();
	if (htCurrent == NULL || m_htMoveDest == NULL)
	{
		m_btnMove.EnableWindow( FALSE );
		return;
	}
	HTREEITEM htOldParent = m_tree.GetParentItem( htCurrent );
	// Get next item to highlihgt
	HTREEITEM htNextItem = m_tree.GetNextSiblingItem( htCurrent );
	if (htNextItem == NULL)
	{
		htNextItem = htOldParent;
	}
	CString szNewParent = m_tree.GetItemText( m_htMoveDest );
	if (szNewParent == "<default>") szNewParent.Empty();
	// Move recursively
	RecursiveMove( htCurrent, m_htMoveDest, szNewParent );
	// Take focus back from button
	m_tree.SetFocus();
	m_tree.SelectItem( htNextItem );
}

// Recursive move of node and its children, creating parallel structure
void CLayoutSplit::RecursiveMove( HTREEITEM htNode, HTREEITEM htDest, LPCTSTR szNewParent )
{
	if (m_tree.ItemHasChildren( htNode ))
	{
		// Create this node in destination
		CString szText = m_tree.GetItemText( htNode );
		htDest = m_tree.InsertItem( szText, htDest );
		HTREEITEM htChild = m_tree.GetChildItem( htNode );
		HTREEITEM htNext;

		while (htChild != NULL)
		{
			htNext = m_tree.GetNextItem( htChild, TVGN_NEXT );
			RecursiveMove( htChild, htDest, szNewParent );
			htChild = htNext;
		}

		// Delete this node
		m_tree.DeleteItem( htNode );

		return;
	}
	// Move to destination
	CFace* pFace = (CFace*)m_tree.GetItemData( htNode );
	if (pFace == NULL)
	{
		m_btnMove.EnableWindow( FALSE );
		return;
	}
	m_tree.DeleteItem( htNode );
	htNode = m_tree.InsertItem( pFace->m_szFaceName, htDest );
	m_tree.SetItemData( htNode, (DWORD_PTR)pFace );
	CString szExisting;
	if (!this->mapDivisionChange.Lookup( pFace->m_szFaceName, szExisting ))
	{
		mapDivisionChange.SetAt( pFace->m_szFaceName, szNewParent );
	}
	else
	{
		mapDivisionChange[pFace->m_szFaceName] = szNewParent;
	}
}

void CLayoutSplit::OnBnClickedOk()
{
	// Commit division changes
	POSITION pos;
	CString szFaceName, szDivision;
	for (pos = mapDivisionChange.GetStartPosition(); pos != NULL; )
	{
		mapDivisionChange.GetNextAssoc( pos, szFaceName, szDivision );
		CFace* pFace = m_pShapeLayout->m_pShape->FindFace( szFaceName );
		if (pFace != NULL)
		{
			pFace->SetLayoutDivision( szDivision );
		}
	}
	// Whatever was set as the move destination, make that the active layout division
	if (this->m_htMoveDest != NULL)
	{
		CString szDivision;
		szDivision = m_tree.GetItemText( m_htMoveDest );
		if (szDivision == "<default>")
		{
			szDivision.Empty();
		}
		m_pShapeLayout->m_pShape->SetActiveDivision( szDivision );
	}
	OnOK();
}

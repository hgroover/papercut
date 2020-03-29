/*!

	@file	 ShapeDictionary.cpp
	
	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: ShapeDictionary.cpp 9 2006-03-08 14:41:10Z henry_groover $

  ShapeDictionary.cpp : implementation of shape dictionary class

*/

#include "stdafx.h"
#include "PaperCut.h"
#include "ShapeDictionary.h"
#include "Dbg.h"
#include ".\shapedictionary.h"


// CShapeDictionary dialog

IMPLEMENT_DYNAMIC(CShapeDictionary, CDialog)
CShapeDictionary::CShapeDictionary(CWnd* pParent /*=NULL*/, CMap<CString,const char*,CString,const char*>* p )
	: CDialog(CShapeDictionary::IDD, pParent)
	, m_szNewEntry(_T(""))
{
	this->map = p;
	this->mapCB = NULL;
	m_szCaption = "Edit shape dictionary";
	a.RemoveAll();
}

CShapeDictionary::~CShapeDictionary()
{
}

BOOL CShapeDictionary::OnInitDialog()
{
	// Call default
	CDialog::OnInitDialog();

	// Set caption
	this->SetWindowText( m_szCaption );

	// Initialize controls
	UpdateData( FALSE );

	// Set up list
	POSITION pos;
	BOOL bFirst = TRUE;
	CString szName, szValue;
	for (pos = this->map->GetStartPosition(); pos != NULL; )
	{
		map->GetNextAssoc( pos, szName, szValue );
		this->m_lstEntries.AddString( szName );
		a.Add( szValue );
		if (bFirst)
		{
			this->m_txtValue.SetWindowText( szValue );
			bFirst = FALSE;
		}
	}

	// If predefined values exist, populate list
	if (this->mapCB != NULL)
	{
		PredefinedCallback cb;
		for (pos = mapCB->GetStartPosition(); pos != NULL; )
		{
			mapCB->GetNextAssoc( pos, szName, cb );
			this->m_lstPredefined.AddString( szName );
		}
	}

	// Update from values
	UpdateData( FALSE );

	// Set focus to first
	return TRUE;
} // CShapeDictionary::OnInitDialog()

void CShapeDictionary::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DICTENTRIES, m_lstEntries);
	DDX_Text(pDX, IDC_NEW_ENTRY, m_szNewEntry);
	DDV_MaxChars(pDX, m_szNewEntry, 32);
	DDX_Control(pDX, IDC_EDIT_VALUE, m_txtValue);
	DDX_Control(pDX, IDC_ADD_NEW, m_btnAddNew);
	DDX_Control(pDX, IDC_PREDEFINED, m_lstPredefined);
	DDX_Control(pDX, IDC_PREDEFINED_TEXT, m_txtPredefinedValue);
}


BEGIN_MESSAGE_MAP(CShapeDictionary, CDialog)
	ON_LBN_SELCHANGE(IDC_DICTENTRIES, OnLbnSelchangeDictentries)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_EN_CHANGE(IDC_NEW_ENTRY, OnEnChangeNewEntry)
	ON_BN_CLICKED(IDC_ADD_NEW, OnBnClickedAddNew)
	ON_LBN_SETFOCUS(IDC_DICTENTRIES, OnLbnSetfocusDictentries)
	ON_EN_KILLFOCUS(IDC_EDIT_VALUE, OnEnKillfocusEditValue)
	ON_BN_CLICKED(IDC_DELETE, OnBnClickedDelete)
	ON_LBN_SELCHANGE(IDC_PREDEFINED, OnLbnSelchangePredefined)
END_MESSAGE_MAP()


// CShapeDictionary message handlers

void CShapeDictionary::OnLbnSelchangeDictentries()
{
	// Selection has changed
	int nSel = m_lstEntries.GetCurSel();
	CDbg::Out( "selchange: new sel %d\n", nSel );
	if (nSel >= 0 && nSel < a.GetCount())
	{
		m_txtValue.SetWindowText( a[nSel] );
	}
}

void CShapeDictionary::OnBnClickedOk()
{
	// Save dictionary entries to map
	UpdateData( TRUE );
	int nEntry;
	CString szName, szValue;
	map->RemoveAll();
	for (nEntry = 0; nEntry < m_lstEntries.GetCount(); nEntry++)
	{
		m_lstEntries.GetText( nEntry, szName );
		szValue = a[nEntry];
		map->SetAt( szName, szValue );
	}
	OnOK();
}

void CShapeDictionary::OnEnChangeNewEntry()
{
	// If empty, disable add button
	UpdateData( TRUE );
	this->m_btnAddNew.EnableWindow(!this->m_szNewEntry.IsEmpty());
}

void CShapeDictionary::OnBnClickedAddNew()
{
	// Add new entry to list
	int nEntryIndex = m_lstEntries.AddString( m_szNewEntry );
	// Add empty value to array
	while (a.GetCount() < m_lstEntries.GetCount()) a.Add("");
	// Make it the current selection
	m_lstEntries.SetCurSel( nEntryIndex );
	// Set focus to edit box
	this->m_txtValue.SetFocus();
}

void CShapeDictionary::OnLbnSetfocusDictentries()
{
	// Got focus - get current selection
	int nSel = m_lstEntries.GetCurSel();
	CDbg::Out( "OnLbnSetfocus(): new sel = %d\n", nSel );
	if (nSel < 0) return;
	m_txtValue.SetWindowText("");
	if (nSel >= a.GetCount()) return;
	m_txtValue.SetWindowText(a[nSel]);
}

void CShapeDictionary::OnEnKillfocusEditValue()
{
	// Save in array
	int nSel = m_lstEntries.GetCurSel();
	if (nSel >= 0 && nSel < a.GetCount())
	{
		m_txtValue.GetWindowText( a[nSel] );
	}
}

void CShapeDictionary::OnBnClickedDelete()
{
	// Get selection
	int nSel = m_lstEntries.GetCurSel();
	if (nSel >= 0 && nSel < a.GetCount())
	{
		// Confirm action
		if (::AfxMessageBox( "Delete selected item?", MB_YESNO | MB_DEFBUTTON2, 0 ) == IDYES)
		{
			a.RemoveAt( nSel );
			m_lstEntries.DeleteString( nSel );
			if (nSel >= m_lstEntries.GetCount()) nSel--;
			m_lstEntries.SetCurSel( nSel );
		}
	}

}

void CShapeDictionary::OnLbnSelchangePredefined()
{
	// Get current selection
	int nSel = m_lstPredefined.GetCurSel();
	if (nSel >= 0 && this->mapCB != NULL)
	{
		CString szName, szValue;
		m_lstPredefined.GetText( nSel, szName );
		PredefinedCallback cb;
		if (mapCB->Lookup( szName, cb )) 
		{
			szValue = (*cb)(0,0);
		}
		else
		{
			szValue = "?undefined";
		}
		this->m_txtPredefinedValue.SetWindowText( szValue );
	}
}

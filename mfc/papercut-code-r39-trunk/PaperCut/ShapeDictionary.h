/*!

	@file	 ShapeDictionary.h
	
	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: ShapeDictionary.h 9 2006-03-08 14:41:10Z henry_groover $


*/

#pragma once
#include "afxwin.h"
#include "afxtempl.h"

#include "GlobalPreferences.h"

// CShapeDictionary dialog

class CShapeDictionary : public CDialog
{
	DECLARE_DYNAMIC(CShapeDictionary)

public:
	CShapeDictionary(CWnd* pParent, CMap<CString,const char*,CString,const char*>* p );   // standard constructor
	virtual ~CShapeDictionary();

	// Dictionary to edit
	CMap<CString,const char*,CString,const char*>* map;

	CMap<CString,const char*,PredefinedCallback,PredefinedCallback>* mapCB;

	// Local copy of values
	CArray<CString,const char*> a;

	// Caption to display
	CString m_szCaption;

// Dialog Data
	enum { IDD = IDD_EDIT_DICTIONARY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLbnSelchangeDictentries();
	afx_msg void OnBnClickedOk();
	afx_msg void OnEnChangeNewEntry();
	// Dictionary entries
	CListBox m_lstEntries;
	// New entry to be added
	CString m_szNewEntry;
	// Edit value control
	CEdit m_txtValue;
	CButton m_btnAddNew;
	afx_msg void OnBnClickedAddNew();
	afx_msg void OnLbnSetfocusDictentries();
	afx_msg void OnEnKillfocusEditValue();
	// Predefined variables for substitution
	CListBox m_lstPredefined;
	afx_msg void OnBnClickedDelete();
	afx_msg void OnLbnSelchangePredefined();
	// Value to display for predefined variable
	CEdit m_txtPredefinedValue;
};

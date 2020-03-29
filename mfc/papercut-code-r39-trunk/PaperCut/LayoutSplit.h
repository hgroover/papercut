/*!

	@file	 LayoutSplit.h

	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: LayoutSplit.h 9 2006-03-08 14:41:10Z henry_groover $

	Layout splitting dialog. This allows creation of new divisions and moving faces and
	groups of faces between divisions.

*/

#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// CLayoutSplit dialog

class CShapeLayout;

class CLayoutSplit : public CDialog
{
	DECLARE_DYNAMIC(CLayoutSplit)

public:
	CLayoutSplit(CWnd* pParent = NULL);   // standard constructor
	virtual ~CLayoutSplit();

// Dialog Data
	enum { IDD = IDD_LAYOUT_SPLIT };

	CShapeLayout *m_pShapeLayout;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// Default division name
	CString m_szNextDivision;

	// Increment default division name
	void IncrementDefaultDivision();

	// Set up tree from shape
	void InitTree();

	// Move destination
	HTREEITEM m_htMoveDest;

	// Recursive move of node and its children, creating parallel structure
	void RecursiveMove( HTREEITEM htNode, HTREEITEM htDest, LPCTSTR szNewParent );

	virtual BOOL OnInitDialog();

	// Map divisions to HTREEITEM
	CMap<CString,LPCTSTR,HTREEITEM,HTREEITEM> mapDivisionItems;

	// Map face name to new division
	CMap<CString,LPCTSTR,CString,LPCTSTR> mapDivisionChange;

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedNewDivision();
	CTreeCtrl m_tree;
	CStatic m_lblIntro;
	afx_msg void OnTvnBegindragFaceTree(NMHDR *pNMHDR, LRESULT *pResult);
	CStatic m_lblList;
	afx_msg void OnTvnSelchangedFaceTree(NMHDR *pNMHDR, LRESULT *pResult);
	CButton m_btnSetDest;
	CButton m_btnMove;
	afx_msg void OnBnClickedSetDestination();
	afx_msg void OnBnClickedMoveNode();
	afx_msg void OnBnClickedOk();
};

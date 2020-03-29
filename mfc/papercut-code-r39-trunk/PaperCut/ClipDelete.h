/*!

	@file	 ClipDelete.h

	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: ClipDelete.h 9 2006-03-08 14:41:10Z henry_groover $

	Cut or delete dialog used by division split dialog.

*/

#pragma once
#include "afxcmn.h"
#include "afxwin.h"


class CShape;
class CFace;

// CClipDelete dialog

class CClipDelete : public CDialog
{
	DECLARE_DYNAMIC(CClipDelete)

public:
	CClipDelete(CWnd* pParent = NULL);   // standard constructor
	virtual ~CClipDelete();

// Dialog Data
	enum { IDD = IDD_CLIP_DELETE };

	CShape* m_pShape;
	// Faces to delete
	CArray<CFace*,CFace*> m_aFaces;
	bool m_bClip; // If true, clipping to m_aFaces
protected:
	// Set up tree from shape
	void InitTree();

	// Recursively add faces to remove list
	void AddItemToRemoveList( HTREEITEM ht );

	// Remove faces in array
	void RemoveFaces( CArray<CFace*,CFace*>& aDeleteList );

	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// Redraw selected list
	void RedrawSelectList();

	DECLARE_MESSAGE_MAP()
public:
	// Hierarchical face groups and faces contained
	CTreeCtrl m_treeFaces;
	afx_msg void OnBnClickedShapeClip();
	afx_msg void OnBnClickedShapeDelete();
	CButton m_btnClip;
	CButton m_btnDelete;
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();
	afx_msg void OnTvnSelchangedFaceTree(NMHDR *pNMHDR, LRESULT *pResult);
	CStatic m_lblIntro;
	CStatic m_lblGroupList;
	afx_msg void OnBnClickedResetSelect();
	CButton m_btnReset;
};

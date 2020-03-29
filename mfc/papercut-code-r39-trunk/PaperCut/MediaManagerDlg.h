/*!

	@file	 MediaManagerDlg.h
	
	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: MediaManagerDlg.h 9 2006-03-08 14:41:10Z henry_groover $

  The media manager basically allows display and en masse copying of content (i.e. pictures)
  used in a shape to another location. This is useful when preparing shape files for upload.

*/

#pragma once
#include "afxwin.h"


// CMediaManagerDlg dialog

class CShape;

class CMediaManagerDlg : public CDialog
{
	DECLARE_DYNAMIC(CMediaManagerDlg)

public:
	CMediaManagerDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMediaManagerDlg();

	// Associated shape
	CShape* m_pShape;

// Dialog Data
	enum { IDD = IDD_MEDIA_MANAGER };

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	// Media used in shape
	CListBox m_lstMedia;
	// Notes for list, e.g. "No media defined", "4 images defined", etc.
	CStatic m_txtNotes;
	afx_msg void OnBnClickedCopyContent();
};

/*!

	@file	 DockDlg.h

	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: DockDlg.h 9 2006-03-08 14:41:10Z henry_groover $

	Docking dialog which is displayed as part of main frame window.

*/

#ifndef _DOCKDLG_H_INCLUDED_
#define	_DOCKDLG_H_INCLUDED_

#include "stdafx.h"
#include "resource.h"

#include <afxext.h>

class CDockDlg: public CDialogBar
{
public:
	CDockDlg();
	~CDockDlg();
	enum { IDD = IDD_DOCKABLE_VIEWCONTROL };
	BOOL InitDialog();

protected:

	//{{AFX_MSG(CDockDlg)
		// No message handlers
	//}}AFX_MSG
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnNMCustomdrawSizeratio(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnEnChangeSizetext();
};


#endif

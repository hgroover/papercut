/*!

	@file	 DlgGetIndent.h
	
	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id$

  Declaration of dialog class to get parameters for face indent/outdent

*/
#pragma once
#include "afxwin.h"


// CDlgGetIndent dialog

class CDlgGetIndent : public CDialog
{
	DECLARE_DYNAMIC(CDlgGetIndent)

public:
	CDlgGetIndent(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgGetIndent();

// Dialog Data
	enum { IDD = IDD_GET_INDENT };

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	// Side proportion, usually in the range 0.1 - 0.9
	double m_SideProportion;
	// Side proportion edit control
	CEdit m_ctlSide;
	// Height proportion relative to longest side
	double m_HeightProportion;
	// Control for height proportion entry
	CEdit m_ctlHeight;
};

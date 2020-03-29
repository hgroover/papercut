/*!

	@file	 DlgInputScale.h
	
	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id$

  Simple input dialog for scale to use when importing OFF format

*/

#pragma once


// DlgInputScale dialog

class DlgInputScale : public CDialog
{
	DECLARE_DYNAMIC(DlgInputScale)

public:
	DlgInputScale(CWnd* pParent = NULL);   // standard constructor
	virtual ~DlgInputScale();

// Dialog Data
	enum { IDD = IDD_DLGINPUTSCALE };

	void SetText( LPCTSTR text ) { m_scaleText = text; }
	CString const& GetText() const { return m_scaleText; }

protected:
	CString m_scaleText;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	virtual void OnOK();

};

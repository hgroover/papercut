/*!

	@file	 GenericSummary.h
	
	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id$

  Generic summary dialog for displaying title text with (potentially long) message display

*/

#pragma once


// GenericSummary dialog

class GenericSummary : public CDialog
{
	DECLARE_DYNAMIC(GenericSummary)

public:
	GenericSummary(CWnd* pParent = NULL);   // standard constructor
	virtual ~GenericSummary();

// Dialog Data
	enum { IDD = IDD_GENERIC_SUMMARY };

	// Set caption for dialog title bar
	void SetCaption( LPCTSTR captionText ) { m_dlgCaption = captionText; }

	// Set header title text
	void SetTitle( LPCTSTR title ) { m_title = title; }

	// Set main (scrollable) text
	void SetText( LPCTSTR text ) { m_text = text; }

	// Set specified caption, title and text and return modal status
	INT_PTR ShowDialog( LPCTSTR caption, LPCTSTR title, LPCTSTR text );

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	CString m_dlgCaption;
	CString m_title;
	CString m_text;

	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();

};

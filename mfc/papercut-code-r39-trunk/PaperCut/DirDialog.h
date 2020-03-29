/*!

	@file	 DirDialog.h

	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: DirDialog.h 9 2006-03-08 14:41:10Z henry_groover $

	Dialog to select a destination directory. Ripped off from somewhere...
	TODO: add url to origin

*/

#if !defined(DIRDIALOG_INCLUDED_)
#define DIRDIALOG_INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// DirDialog.h - header for CDirDialog class

// Recursively create directory path. Return non-zero if successful. lpDir must end with backslash
int AssertDirectoryPathExists( LPCTSTR lpDir );

// CDlgWnd - intercepts messages from child controls
class CDlgWnd : public CWnd
{
public:
    void CheckDir(const CString &ss);   // Display directory contents
	int editctl;						// Subclassed edit control ID

protected:
    afx_msg void OnOpen();              // Open button clicked

    DECLARE_MESSAGE_MAP()
};

// CDirEdit - edit control class
class CDirEdit : public CEdit
{
protected:
    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg UINT OnGetDlgCode();

    DECLARE_MESSAGE_MAP()
};

// CDirDialog - directory selection dialog
class CDirDialog : public CFileDialog
{
public:
    CDirDialog(LPCTSTR initial = NULL,
               LPCTSTR filter = NULL, CWnd* pParentWnd = NULL);

    CString GetPath() { return m_strPath; }

    // Overriden members of CFileDialog
    virtual void OnInitDone();
    virtual void OnFolderChange();

    // Disallow selection of files (since we're only selecting directories)
    virtual BOOL OnFileNameOK() { return TRUE; }

private:
    CString m_strPath;                  // Current directory
    CString m_strFilter;                // The current file filters string (used for string storage
                                        // for internal use of File Open dialog)

    CDlgWnd m_DlgWnd;                   // Subclassed dialog window (parent of CDirDialog window)
    CDirEdit m_Edit;                    // Edit control where you can type in a dir. name
    CButton m_Open;                     // "Open" button (replaces OK button)
};

#endif // DIRDIALOG_INCLUDED_

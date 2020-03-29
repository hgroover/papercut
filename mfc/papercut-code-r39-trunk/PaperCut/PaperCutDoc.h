/*!

	@file	 PaperCutDoc.h
	
	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: PaperCutDoc.h 9 2006-03-08 14:41:10Z henry_groover $

  The document containing the actual CShape and serialization wrapper functionality.

*/

#if !defined(AFX_PAPERCUTDOC_H__9837ABB1_3851_11D6_A858_0040F4309CCE__INCLUDED_)
#define AFX_PAPERCUTDOC_H__9837ABB1_3851_11D6_A858_0040F4309CCE__INCLUDED_

#include "Shape.h"	// Added by ClassView
#include "ShapeLayout.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CPaperCutDoc : public CDocument
{
protected: // create from serialization only
	CPaperCutDoc();
	DECLARE_DYNCREATE(CPaperCutDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPaperCutDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	// Recalculate layout at specified page width and width/height ratio
	int UpdateLayout( double dPageWidthInLogicalUnits, double dPageWidthOverHeight );

	// Read file into an array. Return elements read or -1 if error
	static int ReadFileIntoArray( CFile* pf, CArray<CString,LPCTSTR>& a );

	// Import shape from OFF. Returns number of faces in shape or 0 if error, with errors logged to errorLog
	int ImportOFF( LPCTSTR file, CString& errorLog, int& warningCount, int& errorCount );

	CShapeLayout* m_pShapeLayout;
	CShape m_Shape;
	virtual ~CPaperCutDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CPaperCutDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	// Generated OLE dispatch map functions
	//{{AFX_DISPATCH(CPaperCutDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PAPERCUTDOC_H__9837ABB1_3851_11D6_A858_0040F4309CCE__INCLUDED_)

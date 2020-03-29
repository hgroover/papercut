/*!

	@file	 PrefTabsLayout.h
	
	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: PrefTabsLayout.h 16 2006-04-15 06:39:12Z henry_groover $

  Layout property page from preferences (tabbed) dialog

*/

#if !defined(AFX_PREFTABSLAYOUT_H__987E9C45_5638_11D6_A858_0040F4459482__INCLUDED_)
#define AFX_PREFTABSLAYOUT_H__987E9C45_5638_11D6_A858_0040F4459482__INCLUDED_

#include "GlobalPreferences.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PrefTabsLayout.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPrefTabsLayout dialog

class CPrefTabsLayout : public CPropertyPage
{
	DECLARE_DYNCREATE(CPrefTabsLayout)

// Construction
public:
	void ResetAll();
	void SaveAll( CPreferenceSet* pSaveTo = NULL );
	void SetPrefs( CPreferenceSet* pPrefs );
	CPrefTabsLayout();
	~CPrefTabsLayout();

// Dialog Data
	//{{AFX_DATA(CPrefTabsLayout)
	enum { IDD = IDD_PREFTABS_LAYOUT };
	double	m_dMinJoinAngle;
	double	m_dTabShoulderAngle;
	int m_nTabUsage;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPrefTabsLayout)
	public:
	virtual BOOL OnApply();
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CPreferenceSet * m_pPrefs;
	// Generated message map functions
	//{{AFX_MSG(CPrefTabsLayout)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PREFTABSLAYOUT_H__987E9C45_5638_11D6_A858_0040F4459482__INCLUDED_)

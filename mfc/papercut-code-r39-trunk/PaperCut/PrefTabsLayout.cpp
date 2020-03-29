/*!

	@file	 PrefTabsLayout.cpp
	
	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: PrefTabsLayout.cpp 16 2006-04-15 06:39:12Z henry_groover $

  Layout property page from preferences (tabbed) dialog

*/

#include "stdafx.h"
#include "PaperCut.h"
#include "GlobalPreferences.h"
#include "PrefTabsLayout.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPrefTabsLayout property page

IMPLEMENT_DYNCREATE(CPrefTabsLayout, CPropertyPage)

CPrefTabsLayout::CPrefTabsLayout() : CPropertyPage(CPrefTabsLayout::IDD)
{
	//{{AFX_DATA_INIT(CPrefTabsLayout)
	m_dMinJoinAngle = 0.0;
	m_dTabShoulderAngle = 0.0;
	m_nTabUsage = CGlobalPreferences::UTB_FULL;
	//}}AFX_DATA_INIT
	m_pPrefs = NULL;
}

CPrefTabsLayout::~CPrefTabsLayout()
{
}

void CPrefTabsLayout::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefTabsLayout)
	DDX_Text(pDX, IDC_MINJOINANGLE, m_dMinJoinAngle);
	DDV_MinMaxDouble(pDX, m_dMinJoinAngle, 0., 360.);
	DDX_Text(pDX, IDC_TABSHOULDERANGLE, m_dTabShoulderAngle);
	DDV_MinMaxDouble(pDX, m_dTabShoulderAngle, 0., 360.);
	DDX_Radio(pDX, IDC_TAB_NONE, m_nTabUsage);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefTabsLayout, CPropertyPage)
	//{{AFX_MSG_MAP(CPrefTabsLayout)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CPrefTabsLayout::SetPrefs( CPreferenceSet* pPrefs )
{
	this->m_pPrefs = pPrefs;
	ResetAll();
}

/////////////////////////////////////////////////////////////////////////////
// CPrefTabsLayout message handlers

BOOL CPrefTabsLayout::OnApply() 
{
	// Save values
	SaveAll();
	return CPropertyPage::OnApply();
}

void CPrefTabsLayout::OnOK() 
{
	// Save values
	SaveAll();
	CPropertyPage::OnOK();
}

void CPrefTabsLayout::SaveAll(CPreferenceSet* pSaveTo /*= NULL*/)
{
	if (!pSaveTo) pSaveTo = m_pPrefs;
	if (!pSaveTo) return;
	if (m_hWnd != NULL && !UpdateData())
	{
		return;
	}
	pSaveTo->SetMinFitAngle( m_dMinJoinAngle );
	pSaveTo->SetTabShoulderAngle( m_dTabShoulderAngle );
	pSaveTo->SetUseTabs( m_nTabUsage );
}

void CPrefTabsLayout::ResetAll()
{
	if (!m_pPrefs) return;
	if (this->m_hWnd)
	{
		UpdateData();
	}
	m_dMinJoinAngle = m_pPrefs->GetMinFitAngle();
	m_dTabShoulderAngle = m_pPrefs->GetTabShoulderAngle();
	m_nTabUsage = m_pPrefs->GetUseTabs();
	if (this->m_hWnd)
	{
		UpdateData( FALSE );
	}
}

BOOL CPrefTabsLayout::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// Set initial values

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

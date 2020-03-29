/*!

	@file	 DockDlg.cpp

	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: DockDlg.cpp 15 2006-03-30 15:34:42Z henry_groover $

	Docking dialog which is displayed as part of main frame window.

*/

#include "stdafx.h"

#include "MainFrm.h"
#include "PaperCutDoc.h"
#include "PaperCutView.h"
#include "DockDlg.h"
#include ".\dockdlg.h"

CDockDlg::CDockDlg()
{
}

CDockDlg::~CDockDlg()
{
}


BEGIN_MESSAGE_MAP(CDockDlg, CDialogBar)
	//{{AFX_MSG_MAP(CDockDlg)
		// No message handlers
	//}}AFX_MSG_MAP
	ON_WM_HSCROLL()
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SIZERATIO, OnNMCustomdrawSizeratio)
	ON_EN_CHANGE(IDC_SIZETEXT, OnEnChangeSizetext)
END_MESSAGE_MAP()

// Ain't it wonderful how MFC gives you all this GUI-based app builder
// shit and they let you down on things like this that should be
// fairly obvious...
void
CDockDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// If this is from our slider, handle separately
	
	CMainFrame* pMain = (CMainFrame*)::AfxGetMainWnd();
	if (pMain /*&& pScrollBar->m_hWnd == pMain->m_wndViewControlBar.GetDlgItem( IDC_ROTATE )->m_hWnd*/)
	{
		CSliderCtrl *pSlider = (CSliderCtrl*)pScrollBar;
		CPaperCutView* pView = (CPaperCutView*)pMain->GetActiveFrame()->GetActiveView();
		if (pView)
		{
			// Do nothing... dropped rotate control
		}
		CDbg::Out( 2, "Got horizontal scroll from slider pos = %d\n", pSlider->GetPos() );
		return;
	} // It's the slider
	CDialogBar::OnHScroll(nSBCode, nPos, pScrollBar);
} // CDockDlg::OnHScroll()

BOOL
CDockDlg::InitDialog() 
{
	// Select 0.762 initially (8.5X11)
	// or A4 if non-US,Canada
	LCID userlocale = ::GetUserDefaultLCID();
	char szPaperSize[256] = "us";
	::GetLocaleInfo( userlocale, LOCALE_SABBREVCTRYNAME, szPaperSize, sizeof( szPaperSize ) );
	char *pszFind = "0.762";
	strcat( szPaperSize, "|" );
	strlwr( szPaperSize );
	if (!strstr( "usa|us|ca|mx|", szPaperSize ))
	{
		// A4's nominal width/height ratio is 1/1.41414, but we account
		// for margins...
		pszFind = "0.694";
	}
	CComboBox* pPageAspect = (CComboBox*)GetDlgItem( IDC_PAGEASPECT );
	if (pPageAspect)
	{
		int nSel = pPageAspect->FindString( 0, pszFind );
		nSel = __max( 0, nSel );
		pPageAspect->SetCurSel( nSel );
		CString sz;
		pPageAspect->GetLBText( nSel, sz );
		pPageAspect->SetWindowText( sz );
	}

	CSliderCtrl slide;
	HWND hSlider;

	GetDlgItem( IDC_SIZERATIO, &hSlider );
	slide.Attach( hSlider );
	slide.EnableToolTips();
	// Units are 0.10 to 40.0
	int nSlideMax = (int)(CPaperCutView::VIEWSCALE_MAX * 10.0 + 0.5);
	slide.SetRange( 1, nSlideMax );
	slide.EnableToolTips();
	// Default to 30%
	slide.SetPos( 7 * (nSlideMax / 20) );
	slide.Detach();

	EnableToolTips();
	return TRUE;

} // CDockDlg::InitDialog()

void CDockDlg::OnNMCustomdrawSizeratio(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// Display ratio in text box
	if (pNMCD->dwDrawStage == CDDS_PREPAINT /*CDDS_POSTPAINT*/)
	{
		CSliderCtrl slide;
		HWND hSlider;
		GetDlgItem( IDC_SIZERATIO, &hSlider );
		slide.Attach( hSlider );
		// Range is 1 to 400, representing 0.10 to 40.0
		int nPos = slide.GetPos();
		double dPos = nPos;
		dPos /= 10.0;
		CString szPos;
		szPos.Format( "%.3f", dPos );
		CWnd *wndSizeText = this->GetDlgItem( IDC_SIZETEXT );
		if (wndSizeText != NULL)
		{
			wndSizeText->SetWindowText( szPos );
		}
		slide.Detach();
	}
	*pResult = 0;
}

void CDockDlg::OnEnChangeSizetext()
{
	// Get text

	// Compare to slider
}

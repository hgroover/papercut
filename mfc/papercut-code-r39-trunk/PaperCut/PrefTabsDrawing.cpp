/*!

	@file	 PrefTabsDrawing.cpp
	
	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: PrefTabsDrawing.cpp 9 2006-03-08 14:41:10Z henry_groover $

  Drawing options property page from preferences (tabbed) dialog

*/

#include "stdafx.h"
#include "PaperCut.h"
#include "PrefTabsDrawing.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPrefTabsDrawing property page

IMPLEMENT_DYNCREATE(CPrefTabsDrawing, CPropertyPage)

CPrefTabsDrawing::CPrefTabsDrawing() : CPropertyPage(CPrefTabsDrawing::IDD)
{
	//{{AFX_DATA_INIT(CPrefTabsDrawing)
	m_szDefaultText = _T("");
	m_ScaleBy = 0;
	m_szScaleByArea = _T("");
	m_szScaleByBase = _T("");
	m_szScaleByHeight = _T("");
	m_szScaleByOriginal = _T("");
	m_bFlowAdjoining = FALSE;
	m_bEnableRotation = FALSE;
	//}}AFX_DATA_INIT
	m_pPrefs = NULL;
}

CPrefTabsDrawing::~CPrefTabsDrawing()
{
}

void CPrefTabsDrawing::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefTabsDrawing)
	DDX_Control(pDX, IDC_T1_LBL_SCALEBYORIGINAL, m_lblScaleByOriginal);
	DDX_Control(pDX, IDC_T1_LBL_SCALEBYHEIGHT, m_lblScaleByHeight);
	DDX_Control(pDX, IDC_T1_LBL_SCALEBYBASE, m_lblScaleByBase);
	DDX_Control(pDX, IDC_T1_LBL_SCALEBYAREA, m_lblScaleByArea);
	DDX_Text(pDX, IDC_T1_DEFAULT_FACE, m_szDefaultText);
	DDX_Radio(pDX, IDC_T1_SCALEBYAREA, m_ScaleBy);
	DDX_Text(pDX, IDC_T1_TXT_SCALEBYAREA, m_szScaleByArea);
	DDX_Text(pDX, IDC_T1_TXT_SCALEBYBASE, m_szScaleByBase);
	DDX_Text(pDX, IDC_T1_TXT_SCALEBYHEIGHT, m_szScaleByHeight);
	DDX_Text(pDX, IDC_T1_TXT_SCALEBYORIGINAL, m_szScaleByOriginal);
	DDX_Check(pDX, IDC_FLOW_ADJOINING, m_bFlowAdjoining);
	DDX_Check(pDX, IDC_ENABLE_ROTATION, m_bEnableRotation);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefTabsDrawing, CPropertyPage)
	//{{AFX_MSG_MAP(CPrefTabsDrawing)
	ON_BN_CLICKED(IDC_COLOR_FACE, OnColorFace)
	ON_BN_CLICKED(IDC_COLOR_EDGE, OnColorEdge)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_T1_SCALEBYAREA, OnScalebyarea)
	ON_BN_CLICKED(IDC_COLOR_BGFACE, OnColorBgface)
	ON_BN_CLICKED(IDC_COLOR_BGTAB, OnColorBgtab)
	ON_BN_CLICKED(IDC_COLOR_INWDEDGE, OnColorInwdedge)
	ON_BN_CLICKED(IDC_COLOR_TABEDGE, OnColorTabedge)
	ON_BN_CLICKED(IDC_T1_NOSCALE, OnScalebyarea)
	ON_BN_CLICKED(IDC_T1_SCALEBYBASE, OnScalebyarea)
	ON_BN_CLICKED(IDC_T1_SCALEBYHEIGHT, OnScalebyarea)
	ON_BN_CLICKED(IDC_T1_SCALEBYORIGINAL, OnScalebyarea)
	ON_BN_CLICKED(IDC_TEXT_FONT, OnTextFont)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CPrefTabsDrawing::SetPrefs( CPreferenceSet* pPrefs )
{
	this->m_pPrefs = pPrefs;
	ResetAll();
}

/////////////////////////////////////////////////////////////////////////////
// CPrefTabsDrawing message handlers

BOOL CPrefTabsDrawing::OnApply() 
{
	SaveAll();
	return CPropertyPage::OnApply();
}

void CPrefTabsDrawing::OnOK() 
{
	SaveAll();
	CPropertyPage::OnOK();
}

void CPrefTabsDrawing::SaveAll(CPreferenceSet* pSaveTo /*= NULL*/ )
{
	if (!pSaveTo) pSaveTo = m_pPrefs;
	if (!pSaveTo) return;
	if (m_hWnd != NULL && !UpdateData())
	{
		return;
	}
	pSaveTo->SetEdgeColor( m_rgbEdge );
	pSaveTo->SetFaceColor( m_rgbFace );
	pSaveTo->SetTabColor( this->m_rgbTabEdge );
	pSaveTo->SetFaceBgColor( this->m_rgbBgFace );
	pSaveTo->SetTabBgColor( this->m_rgbBgTab );
	pSaveTo->SetInwdEdgeColor( this->m_rgbInwdEdge );
	pSaveTo->SetPictureFlow( m_bFlowAdjoining ? 1 : 0 );
	pSaveTo->SetScaleMethod( m_ScaleBy );
	pSaveTo->SetAreaRatio( atof( m_szScaleByArea ) / 100.0 );
	pSaveTo->SetHeightRatio( atof( m_szScaleByHeight ) / 100.0 );
	pSaveTo->SetBaseRatio( atof( m_szScaleByBase ) / 100.0 );
	pSaveTo->SetOrgRatio( atof( m_szScaleByOriginal ) / 100.0 );
	pSaveTo->SetDefaultFaceText( this->m_szDefaultText );
	pSaveTo->SetFontAttributes( this->m_uFontAttributes );
	pSaveTo->SetFontPointsize( this->m_nFontPointsize );
	pSaveTo->SetFontTypeface( this->m_szFontTypeface );
	pSaveTo->SetEnableRotation( this->m_bEnableRotation );
}

void CPrefTabsDrawing::ResetAll()
{
	if (!m_pPrefs) return;
	if (this->m_hWnd)
	{
		UpdateData();
	}
	m_rgbFace = m_pPrefs->GetFaceColor();
	m_rgbEdge = m_pPrefs->GetEdgeColor();
	this->m_rgbTabEdge = m_pPrefs->GetTabColor();
	this->m_rgbBgFace = m_pPrefs->GetFaceBgColor();
	this->m_rgbBgTab = m_pPrefs->GetTabBgColor();
	this->m_rgbInwdEdge = m_pPrefs->GetInwdEdgeColor();
	m_szDefaultText = m_pPrefs->GetDefaultFaceText();
	this->m_bFlowAdjoining = m_pPrefs->GetPictureFlow() ? TRUE : FALSE;
	this->m_ScaleBy = m_pPrefs->GetScaleMethod();
	this->m_szScaleByArea.Format( "%.0lf", 100 * m_pPrefs->GetAreaRatio() );
	this->m_szScaleByHeight.Format( "%.0lf", 100 * m_pPrefs->GetHeightRatio() );
	this->m_szScaleByBase.Format( "%.0lf", 100 * m_pPrefs->GetBaseRatio() );
	this->m_szScaleByOriginal.Format( "%.0lf", 100 * m_pPrefs->GetOrgRatio() );
	this->m_nFontPointsize = m_pPrefs->GetFontPointsize();
	this->m_uFontAttributes = m_pPrefs->GetFontAttributes();
	this->m_szFontTypeface = m_pPrefs->GetFontTypeface();
	this->m_bEnableRotation = m_pPrefs->GetEnableRotation();
	if (this->m_hWnd)
	{
		UpdateData( FALSE );
		m_ScaleBy = 0; // Default on entry
		this->OnScalebyarea();
	}
}

HBRUSH CPrefTabsDrawing::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);
	
	COLORREF *pclr = NULL;
	if (pWnd->m_hWnd == GetDlgItem(IDC_COLOR_EDGE)->m_hWnd)
		pclr = &m_rgbEdge;
	else if (pWnd->m_hWnd == GetDlgItem(IDC_COLOR_FACE)->m_hWnd)
		pclr = &m_rgbFace;
	else if (pWnd->m_hWnd == GetDlgItem(IDC_COLOR_BGFACE)->m_hWnd)
		pclr = &m_rgbBgFace;
	else if (pWnd->m_hWnd == GetDlgItem(IDC_COLOR_BGTAB)->m_hWnd)
		pclr = &m_rgbBgTab;
	else if (pWnd->m_hWnd == GetDlgItem(IDC_COLOR_INWDEDGE)->m_hWnd)
		pclr = &m_rgbInwdEdge;
	else if (pWnd->m_hWnd == GetDlgItem(IDC_COLOR_TABEDGE)->m_hWnd)
		pclr = &m_rgbTabEdge;
	if (pclr)
	{
		CRect rc;
		pWnd->GetClientRect( rc );
		CBrush br;
		br.CreateSolidBrush( *pclr );
		pDC->FillRect( rc, &br );
		br.DeleteObject();
	}
	else if (pWnd->m_hWnd == GetDlgItem( IDC_TEXT_FONT )->m_hWnd)
	{
		CRect rc;
		pWnd->GetClientRect( rc );
		CBrush br;
		br.CreateSolidBrush( this->m_rgbBgFace );
		pDC->FillRect( rc, &br );
		pDC->SetTextColor( this->m_rgbFace );
		// Create font based on attributes, etc.
		CFont f;
		int nLogPixels = 72;
		if (pDC->m_hAttribDC) nLogPixels = pDC->GetDeviceCaps( LOGPIXELSY );
		f.CreateFont( -MulDiv( this->m_nFontPointsize, nLogPixels, 72 ), 0, 0, 0, this->m_uFontAttributes&0xfff,
			this->m_uFontAttributes&0x1000 ? 1 : 0, this->m_uFontAttributes&0x2000 ? 1 : 0, this->m_uFontAttributes&0x4000 ? 1 : 0,
			DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, this->m_szFontTypeface );
		CFont* pfOld = pDC->SelectObject( &f );
		CString sz;
		bool bLight = (m_uFontAttributes&0xfff) < FW_NORMAL;
		bool bBold = (m_uFontAttributes&0xfff) > FW_NORMAL;
		sz.Format( "%s %dpt%s%s%s Sample text", (LPCTSTR)this->m_szFontTypeface, m_nFontPointsize,
			m_uFontAttributes&0x1000 ? " i" : "", m_uFontAttributes&0x2000 ? " u" : "",
			bLight ? " light" : bBold ? " bold" : "" );
		pDC->TextOut( 0, 0, sz );
		(void)pDC->SelectObject( pfOld );
		f.DeleteObject();
		br.DeleteObject();
	}

	// TODO: Return a different brush if the default is not desired
	return hbr;
}

void CPrefTabsDrawing::OnScalebyarea() 
{
	// Get new selection
	int nOld = this->m_ScaleBy;
	if (!UpdateData())
	{
		return;
	}
	if (m_ScaleBy == nOld)
	{
		return;
	} // no change
	static int ctlText[] = {
		IDC_T1_TXT_SCALEBYAREA,
		IDC_T1_TXT_SCALEBYHEIGHT,
		IDC_T1_TXT_SCALEBYBASE,
		IDC_T1_TXT_SCALEBYORIGINAL,
		0
							};
	static int ctlLabel[] = {
		IDC_T1_LBL_SCALEBYAREA,
		IDC_T1_LBL_SCALEBYHEIGHT,
		IDC_T1_LBL_SCALEBYBASE,
		IDC_T1_LBL_SCALEBYORIGINAL,
		0
							};
	// Disable old label and text
	// Enable new
	try {
		if (nOld >= 0 && ctlText[nOld])
		{
			GetDlgItem( ctlText[nOld] )->EnableWindow( FALSE );
			GetDlgItem( ctlLabel[nOld] )->EnableWindow( FALSE );
		}
		if (m_ScaleBy >= 0 && ctlText[m_ScaleBy])
		{
			CWnd* p = GetDlgItem( ctlText[m_ScaleBy] );
			p->EnableWindow( TRUE );
			GetDlgItem( ctlLabel[m_ScaleBy] )->EnableWindow( TRUE );
			// Since we've had to put all the radio buttons in z-order,
			// move to the text field which corresponds
			p->SetFocus();
			CEdit *pe = (CEdit*)p;
			pe->SetSel( 0, 9 );
		}
	}
	catch (...)
	{
		CDbg::Out( 0, "Error: exception on scale change!\n" );
		return;
	}
}

BOOL CPrefTabsDrawing::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	UpdateData( FALSE );
	m_ScaleBy = 0; // Default on entry
	OnScalebyarea();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPrefTabsDrawing::OnColorFace() 
{
	EditColor( m_rgbFace, IDC_COLOR_FACE );
}

void CPrefTabsDrawing::OnColorEdge() 
{
	EditColor( m_rgbEdge, IDC_COLOR_EDGE );
}

void CPrefTabsDrawing::OnColorBgface() 
{
	EditColor( m_rgbBgFace, IDC_COLOR_BGFACE );
}

void CPrefTabsDrawing::OnColorBgtab() 
{
	EditColor( m_rgbBgTab, IDC_COLOR_BGTAB );
}

void CPrefTabsDrawing::OnColorInwdedge() 
{
	EditColor( m_rgbInwdEdge, IDC_COLOR_INWDEDGE );
}

void CPrefTabsDrawing::OnColorTabedge() 
{
	EditColor( m_rgbTabEdge, IDC_COLOR_TABEDGE );
}

void CPrefTabsDrawing::EditColor(COLORREF &clr, int nControl)
{
	// Put up color picker
	CColorDialog dlg;
	dlg.m_cc.rgbResult = clr;
	dlg.m_cc.Flags |= CC_RGBINIT;
	if (dlg.DoModal() == IDOK)
	{
		clr = dlg.m_cc.rgbResult;
		CWnd* pWnd = GetDlgItem( nControl );
		if (pWnd)
		{
			pWnd->Invalidate();
		}
		if (nControl == IDC_COLOR_FACE && (pWnd = GetDlgItem( IDC_TEXT_FONT )))
		{
			pWnd->Invalidate();
		}
	}
}

void CPrefTabsDrawing::OnTextFont() 
{
	// Set up logical font
	CFontDialog dlg;
	LOGFONT lf;
	memset( &lf, 0, sizeof( lf ) );
	strncpy( lf.lfFaceName, this->m_szFontTypeface, sizeof( lf.lfFaceName ) );
	CDC *pdc = GetDC();
	CSize s;
	s.cx = 0;
	s.cy = MulDiv( m_nFontPointsize, pdc->GetDeviceCaps( LOGPIXELSY ), 72 );
	pdc->DPtoLP( &s );
	lf.lfHeight = -s.cy;
	lf.lfWeight = this->m_uFontAttributes&0xfff;
	lf.lfItalic = m_uFontAttributes&0x1000 ? 1 : 0;
	lf.lfUnderline = m_uFontAttributes&0x2000 ? 1 : 0;
	lf.lfStrikeOut = m_uFontAttributes&0x4000 ? 1 : 0;
	// Allow only truetype, scaleable fonts
	dlg.m_cf.Flags |= (CF_TTONLY | CF_FORCEFONTEXIST | CF_INITTOLOGFONTSTRUCT | CF_SCALABLEONLY);
	dlg.m_cf.lpLogFont = &lf;
	if (dlg.DoModal() == IDOK)
	{
		m_szFontTypeface = dlg.m_cf.lpLogFont->lfFaceName;
		m_uFontAttributes = dlg.m_cf.lpLogFont->lfWeight;
		if (dlg.m_cf.lpLogFont->lfItalic) m_uFontAttributes |= 0x1000;
		if (dlg.m_cf.lpLogFont->lfUnderline) m_uFontAttributes |= 0x2000;
		if (dlg.m_cf.lpLogFont->lfStrikeOut) m_uFontAttributes |= 0x4000;
		m_nFontPointsize = dlg.m_cf.iPointSize / 10;
		CWnd* pFontWindow = GetDlgItem( IDC_TEXT_FONT );
		if (pFontWindow)
		{
			pFontWindow->Invalidate();
		}
	} // Commit changes
}

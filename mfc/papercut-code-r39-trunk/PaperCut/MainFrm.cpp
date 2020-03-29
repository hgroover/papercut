/*!

	@file	 MainFrm.cpp
	
	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: MainFrm.cpp 15 2006-03-30 15:34:42Z henry_groover $

  The main MFC MDI frame window class handles all shape creation and global preference
  commands.

*/

#include "stdafx.h"
#include "PaperCut.h"
#include "PaperCutDoc.h"
#include "PaperCutView.h"
#include "GlobalPreferences.h"
#include "Preferences.h"
#include "ShapeDictionary.h"
#include "ShapeDef.h"
#include "GenericSummary.h"

#include "MainFrm.h"

#include <math.h>
#include ".\mainfrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static LPCTSTR lpSection = "Main window";

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_COMMAND(IDC_SHAPE_CREATE_TETRA, OnShapeCreateTetra)
	ON_COMMAND(IDC_SHAPE_CREATE_ICOSA, OnShapeCreateIcosa)
	ON_BN_CLICKED(IDC_DBG_ON, OnDbgOn)
	ON_WM_VSCROLL()
	ON_BN_CLICKED(IDC_LANDSCAPE, OnLandscape)
	ON_CBN_SELCHANGE(IDC_PAGEASPECT, OnSelchangePageaspect)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SIZERATIO, OnReleasedcaptureSizeratio)
	ON_COMMAND(IDC_SHAPE_CREATE_HEX, OnShapeCreateHex)
	ON_COMMAND(IDC_SHAPE_CREATE_OCTA, OnShapeCreateOcta)
	ON_COMMAND(IDC_GLOBAL_PREFERENCES, OnGlobalPreferences)
	//}}AFX_MSG_MAP
	// Global help commands
	ON_COMMAND(ID_HELP_FINDER, CMDIFrameWnd::OnHelpFinder)
	ON_COMMAND(ID_HELP, CMDIFrameWnd::OnHelp)
	ON_COMMAND(ID_CONTEXT_HELP, CMDIFrameWnd::OnContextHelp)
	ON_COMMAND(ID_DEFAULT_HELP, CMDIFrameWnd::OnHelpFinder)
	ON_NOTIFY_EX(TTN_NEEDTEXT, 0, OnToolTipNotify)
	ON_COMMAND(IDC_GLOBAL_DICTIONARY, OnGlobalDictionary)
	ON_WM_CLOSE()
	ON_COMMAND(ID_CREATE_KEPLER, OnShapeCreateKepler)
	ON_COMMAND(ID_CREATE_CUBE, OnCreateCube)
	ON_COMMAND(IDC_SHAPE_CREATE_DODECA, OnShapeCreateDodeca)
	ON_COMMAND(ID_ARCHIMEDEAN_CUBOCTAHEDRON, OnArchimedeanCuboctahedron)
	ON_COMMAND(ID_COMPOUND_CUBE_OCTAHEDRON, OnCompoundCubeOctahedron)
	ON_COMMAND(ID_ARCHIMEDEAN_TRUNCATEDOCTAHEDRON, OnArchimedeanTruncatedoctahedron)
	ON_COMMAND(ID_ARCHIMEDEAN_BUCKYBALL, OnArchimedeanBuckyball)
	ON_COMMAND(ID_ARCHIMEDEAN_SNUBCUBE, OnArchimedeanSnubcube)
	ON_COMMAND(ID_COMPOUNDS_CUBE_OCTAHEDRON, OnCompoundsCubeOctahedron)
	ON_COMMAND(ID_COMPOUNDS_ICOSA_DODECA, OnCompoundsIcosaDodeca)
	ON_COMMAND(ID_ARCHIMEDEAN_TRUNCATEDTETRAHEDRON, OnArchimedeanTruncatedtetrahedron)
	ON_UPDATE_COMMAND_UI(IDC_GLOBAL_PREFERENCES, OnUpdateGlobalPreferences)
	ON_COMMAND_RANGE(ID_SHAPEDEF_01,ID_SHAPEDEF_999, OnShapedefCreate)
	ON_COMMAND(IDC_SHAPE_CREATE_TRIANGLE, OnShapeCreateTriangle)
	ON_COMMAND(IDC_SHAPE_CREATE_SQUARE, OnShapeCreateSquare)
	ON_COMMAND(IDC_SHAPE_CREATE_PENTAGON, OnShapeCreatePentagon)
	ON_COMMAND(IDC_SHAPE_CREATE_HEXAGON, OnShapeCreateHexagon)
	ON_COMMAND(IDC_SHAPE_CREATE_OCTAGON, OnShapeCreateOctagon)
	ON_COMMAND(IDC_SHAPE_CREATE_DECAGON, OnShapeCreateDecagon)
	ON_COMMAND(ID_CREATE_ESCHER, OnCreateEscher)
	ON_COMMAND(ID_IMPORT_OFF, OnImportOff)
END_MESSAGE_MAP()
// removed:
//	ON_NOTIFY(NM_CUSTOMDRAW, IDC_ROTATE, OnCustomdrawRotate)

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	m_nUntitledTetra = 0;
	m_nUntitledIcosa = 0;
	m_nUntitledPoly = 0;
	m_dLastWidthHeight = 0;
	m_bFireEvents = true;
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	if (!m_wndViewControlBar.Create( this, IDD_DOCKABLE_VIEWCONTROL, WS_CHILD |
			CBRS_TOP | CBRS_GRIPPER | CBRS_SIZE_DYNAMIC,
			IDD_DOCKABLE_VIEWCONTROL ))
	{
		TRACE0( "Failed to create dialog toolbar\n" );
		return -1;
	}
	// Initialize controls
	m_wndViewControlBar.InitDialog();
	
	m_wndViewControlBar.EnableDocking( CBRS_ALIGN_ANY );
	//CFrameWnd* pDock = m_wndViewControlBar.GetDockingFrame();
	CRect r;
	// This won't work - it hasn't resized itself yet...
	m_wndToolBar.GetWindowRect( r );
	//r.OffsetRect( 238, 0 );
	//r.top += 2;
	//r.bottom = r.top + 12;
	r.left += 240;
	r.right += 240;
	CDbg::Out( "Trying to dock at %d, %d, %d, %d\n", r.left, r.top, r.right, r.bottom );
	LPCRECT lpr = &r;
	//DockControlBar( (CControlBar*)&m_wndViewControlBar, (UINT)0, lpr );
	DockControlBar( (CControlBar*)&m_wndViewControlBar );

	// Enable tooltip drawing via OnToolTipNotify()
	EnableToolTips();

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// Get saved window placement
	int nTop, nLeft, nBottom, nRight;
	nTop = MYAPP()->GetProfileInt( lpSection, "top", -1 );
	nLeft = MYAPP()->GetProfileInt( lpSection, "left", -1 );
	nBottom = MYAPP()->GetProfileInt( lpSection, "bottom", -1 );
	nRight = MYAPP()->GetProfileInt( lpSection, "right", -1 );
	if (nTop >= 0 && nLeft >= 0 && nRight > 0 && nBottom > 0)
	{
		cs.cx = nRight - nLeft + 1;
		cs.cy = nBottom - nTop + 1;
		cs.x = nLeft;
		cs.y = nTop;
		int nShowCmd = MYAPP()->GetProfileInt( lpSection, "showcmd", -1 );
		if (nShowCmd != -1)
		{
			if (nShowCmd == SW_SHOWMAXIMIZED)
				cs.style |= WS_MAXIMIZE;
		}
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers


void CMainFrame::OnShapeCreateTetra() 
{
	OnShapeCreate( m_nUntitledTetra, "Tetrahedron" );
}

void CMainFrame::OnShapeCreateIcosa() 
{
	OnShapeCreate( m_nUntitledIcosa, "Icosahedron" );
}

void CMainFrame::OnShapeCreateHex() 
{
	OnShapeCreate( m_nUntitledTetra, "Hexa" );
}

void CMainFrame::OnShapeCreateOcta() 
{
	OnShapeCreate( m_nUntitledTetra, "Octa" );
}

void CMainFrame::OnShapeCreateKepler()
{
	CPaperCutDoc * pDoc = OnShapeCreate( m_nUntitledTetra, "Stella-Octangula", true, 1.0, TRUE, "tetra"  );
}

void CMainFrame::OnCreateCube()
{
	// Create cube shape
	OnShapeCreate( m_nUntitledTetra, "Cube" );
}


void CMainFrame::OnShapeCreateDodeca()
{
	OnShapeCreate( m_nUntitledTetra, "Dodecahedron" );
}

CPaperCutDoc * CMainFrame::OnShapeCreate(int& nUntitled, LPCTSTR lpszName, bool bStellate /*= false*/, double dRatio /*= 0.0*/, BOOL bKepler /*= FALSE*/, LPCTSTR lpSetupName /*=NULL*/)
{
	// Create new document
	CPaperCutApp* pApp = (CPaperCutApp*)AfxGetApp();
	POSITION pos = pApp->GetFirstDocTemplatePosition();
	CMultiDocTemplate* pDocTemplate = (CMultiDocTemplate*)pApp->GetNextDocTemplate( pos );
	CPaperCutDoc* pDoc = (CPaperCutDoc*)pDocTemplate->CreateNewDocument();
	// Set up basic shape
	if (lpSetupName == NULL) lpSetupName = lpszName;
	pDoc->m_Shape.SetupShape( lpSetupName );
	// Do stellation if requested
	if (bStellate)
	{
		pDoc->m_Shape.Stellate( dRatio, bKepler );
	}
	// Create layout
	pDoc->UpdateLayout( pDoc->m_Shape.GetLayoutDefaultWidth(), pDoc->m_Shape.GetLayoutDefaultWidthOverHeight() );
	// Create view window
	CFrameWnd* pViewWindow = pDocTemplate->CreateNewFrame( pDoc, this );
	//pos = pDoc->GetFirstViewPosition();
	//CPaperCutView* pView = (CPaperCutView*)pDoc->GetNextView( pos );
	//pViewWindow->SetActiveView( pView );
	CString szName;
	nUntitled++;
	szName.Format( "%s-%d", lpszName, nUntitled );
	pDoc->SetTitle( szName );
	pDoc->m_Shape.SetName( szName );

	// This will automatically set the active view
	pViewWindow->InitialUpdateFrame( pDoc, TRUE );

	//pViewWindow->MDIActivate();

	return pDoc;
}

CPaperCutDoc * CMainFrame::OnPolyShapeCreate( char cType, LPCTSTR lpszName )
{
	// Create new document
	CPaperCutApp* pApp = (CPaperCutApp*)AfxGetApp();
	POSITION pos = pApp->GetFirstDocTemplatePosition();
	CMultiDocTemplate* pDocTemplate = (CMultiDocTemplate*)pApp->GetNextDocTemplate( pos );
	CPaperCutDoc* pDoc = (CPaperCutDoc*)pDocTemplate->CreateNewDocument();
	// Set up basic polygon shape
	pDoc->m_Shape.SetupPolygon( cType );
	// Create layout
	pDoc->UpdateLayout( pDoc->m_Shape.GetLayoutDefaultWidth(), pDoc->m_Shape.GetLayoutDefaultWidthOverHeight() );
	// Create view window
	CFrameWnd* pViewWindow = pDocTemplate->CreateNewFrame( pDoc, this );
	//pos = pDoc->GetFirstViewPosition();
	//CPaperCutView* pView = (CPaperCutView*)pDoc->GetNextView( pos );
	//pViewWindow->SetActiveView( pView );
	CString szName;
	this->m_nUntitledPoly;
	szName.Format( "%s-%d", lpszName, m_nUntitledPoly );
	pDoc->SetTitle( szName );
	pDoc->m_Shape.SetName( szName );

	// This will automatically set the active view
	pViewWindow->InitialUpdateFrame( pDoc, TRUE );

	//pViewWindow->MDIActivate();

	return pDoc;
}

void CMainFrame::OnDbgOn() 
{
	// Get status - control has already changed
	if (!m_wndViewControlBar.IsDlgButtonChecked( IDC_DBG_ON ))
	{
		CDbg::m_Level = __max( 0, CDbg::m_Level - 1 );
	}
	else
	{
		CDbg::m_Level++;
	}
}

void CMainFrame::RecalcLayout(BOOL bNotify) 
{
	// Can we force view control bar to go where we want it?
	CMDIFrameWnd::RecalcLayout(bNotify);
}

void CMainFrame::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// TODO: Add your message handler code here and/or call default
	
	CMDIFrameWnd::OnVScroll(nSBCode, nPos, pScrollBar);
}

int CMainFrame::SetMaxPage(int nMax)
{
	// Sanity check
	if (nMax <= 0)
	{
		return -1;
	}
	int nOldCount = GetMaxPage();
	return nOldCount;
}

int CMainFrame::GetMaxPage()
{
	int nParts = 999;
	return nParts;
}

// Get the current width / height ratio from page aspect dropdown
double
CMainFrame::GetWidthOverHeight()
{
	static const double dDefault = 0.762;
	CComboBox* pBox = (CComboBox*)m_wndViewControlBar.GetDlgItem( IDC_PAGEASPECT );
	CButton* pLandscape = (CButton*)m_wndViewControlBar.GetDlgItem( IDC_LANDSCAPE );
	bool bLandscape = false;
	if (pLandscape)
	{
		bLandscape = (pLandscape->GetCheck() != 0);
	}
	double dDef = dDefault;
	if (bLandscape)
	{
		dDef = 1 / dDef;
	}
	if (!pBox)
	{
		// Default to 8.5X11
		return dDef;
	}
	CString szText;
	pBox->GetWindowText( szText );
	if (szText.IsEmpty())
	{
		return dDef;
	}
	double d = atof( szText );
	if (d == 0)
	{
		return dDef;
	}
	if (bLandscape)
	{
		d = 1 / d;
	}
	m_dLastWidthHeight = d;
	return d;
} // CMainFrame::GetWidthOverHeight()

void CMainFrame::OnLandscape() 
{
	if (!m_bFireEvents) return;
	double dOld = m_dLastWidthHeight;
	double d = GetWidthOverHeight();
	if (d != dOld)
	{
		CPaperCutView* pView = (CPaperCutView*)this->GetActiveFrame()->GetActiveView();
		if (!pView)
		{
			CDbg::Out( "Error: no view!\n" );
			return;
		}
		pView->SetPageAspect( d );
	} // Changed
}

void CMainFrame::OnSelchangePageaspect() 
{
	OnLandscape();
}

void CMainFrame::OnReleasedcaptureSizeratio(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// Get new rotation value
	if (pNMHDR->idFrom == IDC_SIZERATIO)
	{
		CPaperCutView* pView = (CPaperCutView*)this->GetActiveFrame()->GetActiveView();
		if (pView)
		{
			pView->SetShapeSize( GetPageWidthInLogicalUnits() );
		}
	} // it's our control
	
	*pResult = 0;
}

double CMainFrame::GetPageWidthInLogicalUnits()
{
	CSliderCtrl *pSlider = (CSliderCtrl*)m_wndViewControlBar.GetDlgItem( IDC_SIZERATIO );
	if (!pSlider)
	{
		return 2.5;
	} // Return a default
	ASSERT( pSlider != NULL );
	int nComplement = pSlider->GetRangeMax() - pSlider->GetPos() + 1;
	return nComplement / CPaperCutView::VIEWSCALE_MAX;
}



BOOL
CMainFrame::OnToolTipNotify( UINT id, NMHDR * pNMHDR, LRESULT * pResult )
{
    TOOLTIPTEXT *pTTT = (TOOLTIPTEXT *)pNMHDR;
    UINT nID =pNMHDR->idFrom;
	CDbg::Out( 0, "ttn, code %d, id %u, ttn=%c\n", pNMHDR->code, pNMHDR->idFrom, pNMHDR->code==TTN_NEEDTEXT ? 'y' : 'n' );
    if (pTTT->uFlags & TTF_IDISHWND)
    {
        // idFrom is actually the HWND of the tool
        nID = ::GetDlgCtrlID((HWND)nID);
	}
	CDbg::Out( 0, "id %u, match=%c\n", nID, nID==IDC_SIZERATIO ? 'y' : 'n' );
    if (nID == IDC_SIZERATIO)
    {
        //pTTT->lpszText = MAKEINTRESOURCE(nID);
		pTTT->hinst = NULL;
		pTTT->lpszText = pTTT->szText;
        //pTTT->hinst = AfxGetResourceHandle();
			CSliderCtrl *pSlider = (CSliderCtrl*)m_wndViewControlBar.GetDlgItem( pNMHDR->idFrom );
			ASSERT( pSlider );
			int nPos = pSlider->GetPos();
			if (pNMHDR->idFrom == IDC_SIZERATIO)
			{
				float f = (float)((pSlider->GetRangeMax() - nPos + 1) / CPaperCutView::VIEWSCALE_MAX);
				sprintf( pTTT->szText, "scale %.1f units/page", f );
			}
			CDbg::Out( "text \"%s\"\n", pTTT->szText );
        return(TRUE);
    }
    return(FALSE);
} // CMainFrame::OnToolTipNotify()

// Set page width in logical units
void
CMainFrame::SetPageWidthInLogicalUnits( double d )
{
	// The slider is the number of logical units that will
	// fit across a page.  This means that as it increases the
	// size of triangles gets smaller.  We make it work so that
	// it has the expected sense: left --> right makes things
	// bigger.

	// Convert back to decimal units
	int n = (int)(d * CPaperCutView::VIEWSCALE_MAX);

	// Subtract from range
	CSliderCtrl *pSlider = (CSliderCtrl*)m_wndViewControlBar.GetDlgItem( IDC_SIZERATIO );
	ASSERT( pSlider );
	pSlider->SetPos( __max( 0, pSlider->GetRangeMax() - n ) );
	// This will fire off a message, but we are checking against spurious changes...
} // CMainFrame::SetPageWidthInLogicalUnits()

// Get size ratio from slider. Size ratio is the percentage of a page's smallest dimension
// (typically width) occupied by a unit.
double CMainFrame::GetSizeRatio()
{
	return 1.0;
}

// Set size ratio
void CMainFrame::SetSizeRatio( double d )
{
}


// Set width over height (and automatically set landscape)
void
CMainFrame::SetPageWidthOverHeight( double d )
{
	bool bLandscape = false;
	if (d > 1.0)
	{
		bLandscape = true;
		d = 1.0 / d;
	}
	// Suspend firing of events
	m_bFireEvents = false;
	// Enumerate entries and find closest match
	CComboBox* pBox = (CComboBox*)m_wndViewControlBar.GetDlgItem( IDC_PAGEASPECT );
	CButton* pLandscape = (CButton*)m_wndViewControlBar.GetDlgItem( IDC_LANDSCAPE );
	if (!pBox) return;
	int nSize = pBox->GetCount();
	int n;
	CString sz;
	int idxMinDelta = -1;
	double dMinDelta = 99999999;
	CString szMinDelta;
	for (n = 0; n < nSize; n++)
	{
		pBox->GetLBText( n, sz );
		double dDelta = fabs( atof( sz ) - d );
		if (dDelta < dMinDelta)
		{
			dMinDelta = dDelta;
			idxMinDelta = n;
			szMinDelta = sz;
		}
	}
	if (idxMinDelta >= 0)
	{
		pLandscape->SetCheck( bLandscape ? 1 : 0 );
		pBox->SetCurSel( idxMinDelta );
		pBox->SetWindowText( szMinDelta );
	} // Found a match
	// Resume firing events
	m_bFireEvents = true;
} // CMainFrame::SetPageWidthOverHeight()

void CMainFrame::OnGlobalPreferences() 
{
	// Get preferences from app
	// Put up dialog
	CPropertySheet dlg( "PaperCut Global Preferences" );
	CPrefTabsDrawing tabDrawing;
	CPrefTabsLayout tabLayout;
	CGlobalPreferences *pPrefs = &(MYAPP()->m_prefs);
	tabDrawing.SetPrefs( pPrefs );
	dlg.AddPage( &tabDrawing );
	tabLayout.SetPrefs( pPrefs );
	dlg.AddPage( &tabLayout );
	// If OK, saved in dialog
	// Individual pages will save to memory
	if (dlg.DoModal() == IDOK)
	{
		// Write to .ini file
		pPrefs->Save();
		// Redraw all
		CPaperCutView* pView = (CPaperCutView*)this->GetActiveFrame()->GetActiveView();
		if (pView)
		{
			pView->InvalidateRect(NULL);
			pView->RedrawWindow();
		}
	} // Save to ini
	
}

void CMainFrame::OnGlobalDictionary()
{
	// Edit global dictionary
	CShapeDictionary dlgEdit( this, &(MYAPP()->m_prefs.m_mapGlobalDict) );
	dlgEdit.m_szCaption = "Edit global dictionary";
	dlgEdit.mapCB = &(MYAPP()->m_prefs.m_mapPredefs);
	if (dlgEdit.DoModal() == IDOK)
	{
		// Write to registry
		MYAPP()->m_prefs.Save();
		CPaperCutView* pView = (CPaperCutView*)this->GetActiveFrame()->GetActiveView();
		if (pView)
		{
			pView->InvalidateRect(NULL);
			pView->RedrawWindow();
		}
	}
}

void CMainFrame::OnClose()
{
	// Get window placement
	WINDOWPLACEMENT wp;
	wp.length = sizeof( wp );
	if (GetWindowPlacement( &wp ))
	{
		// Save to registry
		MYAPP()->WriteProfileInt( lpSection, "top", wp.rcNormalPosition.top );
		MYAPP()->WriteProfileInt( lpSection, "left", wp.rcNormalPosition.left );
		MYAPP()->WriteProfileInt( lpSection, "bottom", wp.rcNormalPosition.bottom );
		MYAPP()->WriteProfileInt( lpSection, "right", wp.rcNormalPosition.right );
		MYAPP()->WriteProfileInt( lpSection, "showcmd", wp.showCmd );
	}
	CMDIFrameWnd::OnClose();

	// Shut down main app
	MYAPP()->ReleaseShapeDefs();
}



void CMainFrame::OnArchimedeanCuboctahedron()
{
	OnShapeCreate( m_nUntitledTetra, "Cuboctahedron" );
}

void CMainFrame::OnCompoundCubeOctahedron()
{
	// Use stellation ratio 1/sqrt(2)
	OnShapeCreate( m_nUntitledTetra, "Cube-Octahedron compound", true, 0.70710678118654752440084436210485, TRUE, "octa" );
}

void CMainFrame::OnArchimedeanTruncatedoctahedron()
{
	OnShapeCreate( m_nUntitledTetra, "Truncated Octahedron" );
}

void CMainFrame::OnArchimedeanBuckyball()
{
	OnShapeCreate( m_nUntitledTetra, "Buckyball" );
}

void CMainFrame::OnArchimedeanSnubcube()
{
	OnShapeCreate( m_nUntitledTetra, "Snub Cube" );
}

void CMainFrame::OnCompoundsCubeOctahedron()
{
	OnCompoundCubeOctahedron();
}

static double STELL_RATIO_ICOSADODECA = 2.0 / (1.0 + sqrt( (double)5.0 ));
void CMainFrame::OnCompoundsIcosaDodeca()
{
	// Use stellation ratio 2/(1+sqrt(5)) == 0.61803398874989484820458683436564
	OnShapeCreate( m_nUntitledTetra, "Icosahedron-Dodecahedron compound", true, STELL_RATIO_ICOSADODECA, TRUE, "icosahedron" );
}

void CMainFrame::OnArchimedeanTruncatedtetrahedron()
{
	OnShapeCreate( m_nUntitledTetra, "Truncated Tetrahedron" );
}

void CMainFrame::OnUpdateGlobalPreferences(CCmdUI *pCmdUI)
{
	// Load items from shapedefs.txt
	pCmdUI->Enable();
	MYAPP()->AddShapedefsToMenu( this->GetMenu(), ID_SHAPEDEF_01 );
}

void CMainFrame::OnShapedefCreate( UINT nID )
{
	// Get origin:0 ordinal
	int Ordinal = nID - ID_SHAPEDEF_01;
	if (Ordinal < 0 || Ordinal > 999)
	{
		AfxMessageBox( "Error: invalid menu definition for shapedef!" );
		return;
	}
	CString szShape = MYAPP()->GetShapeByOrdinal( Ordinal );
	if (szShape.IsEmpty())
	{
		AfxMessageBox( "Error: no shape definition found!" );
		return;
	}

	OnShapeCreate( m_nUntitledTetra, szShape );
}

void CMainFrame::GetMessageString(   UINT nID,   CString& rMessage ) const
{
	if (nID >= ID_SHAPEDEF_01 && nID <= ID_SHAPEDEF_999)
	{
		int Ordinal = nID - ID_SHAPEDEF_01;
		CShapeDef* pDef = MYAPP()->GetShapeDef( Ordinal );
		if (pDef != NULL)
		{
			rMessage = pDef->m_szBrief;
			return;
		}
	}
	CFrameWnd::GetMessageString( nID, rMessage );
}


void CMainFrame::OnShapeCreateTriangle()
{
	// Create triangle
	OnPolyShapeCreate( 'T', "Triangle" );
}

void CMainFrame::OnShapeCreateSquare()
{
	// Create square
	OnPolyShapeCreate( 'S', "Square" );
}

void CMainFrame::OnShapeCreatePentagon()
{
	// Create pentagon
	OnPolyShapeCreate( 'P', "Pentagon" );
}

void CMainFrame::OnShapeCreateHexagon()
{
	// Create hexagon
	OnPolyShapeCreate( 'H', "Hexagon" );
}

void CMainFrame::OnShapeCreateOctagon()
{
	// Create octagon
	OnPolyShapeCreate( 'O', "Octagon" );
}

void CMainFrame::OnShapeCreateDecagon()
{
	OnPolyShapeCreate( 'D', "Decagon" );
}

static double STELL_RATIO_ESCHER = 2.0 / sqrt( (double)3.0 );
void CMainFrame::OnCreateEscher()
{
	// Create rhombic dodecahedron and cumulate by 115.47005383792515290182975610039%
	OnShapeCreate( m_nUntitledTetra, "Escher's Solid", true, STELL_RATIO_ESCHER, FALSE, "Rhombic Dodecahedron" );
}

void CMainFrame::OnImportOff()
{
	// Get file to import
	CFileDialog dlgInput( TRUE, "off" );
	if (dlgInput.DoModal() != IDOK)
	{
		return;
	}
	// Create new document
	CPaperCutApp* pApp = (CPaperCutApp*)AfxGetApp();
	POSITION pos = pApp->GetFirstDocTemplatePosition();
	CMultiDocTemplate* pDocTemplate = (CMultiDocTemplate*)pApp->GetNextDocTemplate( pos );
	CPaperCutDoc* pDoc = (CPaperCutDoc*)pDocTemplate->CreateNewDocument();
	CString szName = dlgInput.GetFileName();
	int offLoc = szName.MakeLower().Find(".off");
	if (offLoc > 0)
	{
		szName = szName.Left( offLoc );
	}
	// Try to import it
	CString errorLog;
	int warnings, errors;
	if (pDoc->ImportOFF( dlgInput.GetPathName(), errorLog, warnings, errors ) < 1)
	{
		CString summaryTitle;
		summaryTitle.Format( "Import FAILED: %d errors and %d warnings listed below", errors, warnings );

		GenericSummary dlgSummary;
		dlgSummary.ShowDialog( "Import failed", summaryTitle, errorLog );
		pDocTemplate->RemoveDocument( pDoc );
		delete pDoc;
		return;
	}
	// Import succeeded. Show any warnings
	if (warnings > 0)
	{
		CString summaryTitle;
		summaryTitle.Format( "Import succeeded with %d warnings", warnings );
		GenericSummary dlgSummary;
		dlgSummary.ShowDialog( "Import succeeded", summaryTitle, errorLog );
	}
	// Create layout
	pDoc->UpdateLayout( pDoc->m_Shape.GetLayoutDefaultWidth(), pDoc->m_Shape.GetLayoutDefaultWidthOverHeight() );

	// Create view window
	CFrameWnd* pViewWindow = pDocTemplate->CreateNewFrame( pDoc, this );
	pDoc->m_Shape.SetName( szName );
	pDoc->SetTitle( szName );
	//pViewWindow->SetTitle( szName );
	pDoc->SetModifiedFlag();

	// This will automatically set the active view
	pViewWindow->InitialUpdateFrame( pDoc, TRUE );
}

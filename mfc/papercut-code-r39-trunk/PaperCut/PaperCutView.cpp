/*!

	@file	 PaperCutView.cpp
	
	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: PaperCutView.cpp 16 2006-04-15 06:39:12Z henry_groover $

  The primary view associated with CPaperCutDoc. Displays the flat net layout of a shape.

*/

#include "stdafx.h"
#include "PaperCut.h"

#include <math.h>
//#include <Dib.h>

#include "PaperCutDoc.h"
#include "PaperCutView.h"

#include "Vertex.h"
#include "Edge.h"
#include "Face.h"
#include "Shape.h"
#include "ShapeLayout.h"
#include "LogPoint.h"
#include "PageGroup.h"
#include "LayoutSummaryDlg.h"
#include "DlgGetIndent.h"
#include "FaceContent.h"
#include "EditFace.h"
#include "MainFrm.h"
#include "DlgSelectStellation.h"
#include "GlobalPreferences.h"
#include "Preferences.h"
#include "ShapeDictionary.h"
#include "ClipDelete.h"
#include "MediaManagerDlg.h"
#include "LayoutSplit.h"
#include ".\papercutview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPaperCutView

IMPLEMENT_DYNCREATE(CPaperCutView, CView)

BEGIN_MESSAGE_MAP(CPaperCutView, CView)
	//{{AFX_MSG_MAP(CPaperCutView)
	ON_WM_LBUTTONUP()
	ON_COMMAND(IDC_STELLATE_100, OnStellate100)
	ON_COMMAND(IDC_STELLATE_25, OnStellate25)
	ON_COMMAND(IDC_STELLATE_33, OnStellate33)
	ON_COMMAND(IDC_STELLATE_75, OnStellate75)
	ON_COMMAND(IDC_STELLATE_50, OnStellate50)
	ON_WM_SIZE()
	ON_COMMAND(IDC_SHAPE_GEO2X, OnShapeGeo2x)
	ON_COMMAND(IDC_SHAPE_GEO3X, OnShapeGeo3x)
	ON_COMMAND(IDC_SHAPE_GEO4X, OnShapeGeo4x)
	ON_UPDATE_COMMAND_UI(IDC_SHAPE_GEO2X, OnUpdateShapeGeo)
	ON_UPDATE_COMMAND_UI(IDC_VIEW_LAYOUT_SUMMARY, OnUpdateViewLayoutSummary)
	ON_COMMAND(IDC_VIEW_LAYOUT_SUMMARY, OnViewLayoutSummary)
	ON_WM_RBUTTONDOWN()
	ON_WM_DROPFILES()
	ON_COMMAND(IDC_SHOW_FACE, OnShowFace)
	ON_COMMAND(IDC_SET_FACETEXT, OnSetFacetext)
	ON_COMMAND(IDC_CLEAR_IMAGE, OnClearImage)
	ON_WM_SETFOCUS()
	ON_COMMAND(IDC_REMOVE_FACE, OnRemoveFace)
	ON_COMMAND(IDC_STELLATE_10, OnStellate10)
	ON_COMMAND(IDC_STELLATE_125, OnStellate125)
	ON_COMMAND(IDC_STELLATE_15, OnStellate15)
	ON_COMMAND(IDC_STELLATE_150, OnStellate150)
	ON_COMMAND(IDC_STELLATE_200, OnStellate200)
	ON_COMMAND(IDC_STELLATE_OTHER, OnStellateOther)
	ON_COMMAND(IDC_PREFERENCES, OnPreferences)
	ON_COMMAND(IDC_SET_FACEIMAGE, OnSetFaceimage)
	ON_COMMAND(IDC_GEOFRACTAL_STELLATE, OnGeofractalStellate)
	ON_COMMAND(IDC_FRACTAL_STELLATE, OnFractalStellate)
	ON_UPDATE_COMMAND_UI(IDC_GEOFRACTAL_STELLATE, OnUpdateGeofractalStellate)
	ON_COMMAND(IDC_STELLATE_FACE, OnStellateFace)
	ON_UPDATE_COMMAND_UI(IDC_SHAPE_GEO3X, OnUpdateShapeGeo)
	ON_UPDATE_COMMAND_UI(IDC_SHAPE_GEO4X, OnUpdateShapeGeo)
	ON_COMMAND(IDC_VIEW_NEXT_FACEGROUP, OnViewNextFacegroup)
	ON_UPDATE_COMMAND_UI(IDC_VIEW_NEXT_FACEGROUP, OnUpdateViewNextFacegroup)
	ON_COMMAND(IDC_VIEW_NEXT_PAGE, OnViewNextPage)
	ON_UPDATE_COMMAND_UI(IDC_VIEW_NEXT_PAGE, OnUpdateViewNextPage)
	ON_COMMAND(IDC_VIEW_PREV_FACE_GROUP, OnViewPrevFaceGroup)
	ON_UPDATE_COMMAND_UI(IDC_VIEW_PREV_FACE_GROUP, OnUpdateViewPrevFaceGroup)
	ON_COMMAND(IDC_VIEW_PREV_PAGE, OnViewPrevPage)
	ON_UPDATE_COMMAND_UI(IDC_VIEW_PREV_PAGE, OnUpdateViewPrevPage)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
	ON_COMMAND_RANGE(IDC_REMOVE_GROUP1, IDC_REMOVE_GROUP9, OnRemoveGroup)
	ON_COMMAND_RANGE(IDC_LAYOUT_USER1, IDC_LAYOUT_USER9, OnLayoutSwitch)
	ON_COMMAND_RANGE(ID_JOIN_EDGE_01, ID_JOIN_EDGE_999, OnJoinEdge)
	ON_COMMAND(IDC_SHAPE_DICTIONARY, OnShapeDictionary)
	ON_COMMAND(IDC_KSTELLATE_25, OnKstellate25)
	ON_COMMAND(IDC_KSTELLATE_OTHER, OnKstellateOther)
	ON_COMMAND(IDC_KSTELLATE_100, OnKstellate100)
	ON_COMMAND(IDC_KSTELLATE_50, OnKstellate50)
	ON_COMMAND(IDC_KSTELLATE_200, OnKstellate200)
	ON_COMMAND(ID_EDIT_CLIP, OnEditClip)
	ON_COMMAND(IDC_KSTELLATE_60, OnKstellate60)
	ON_COMMAND(IDC_STELLATE_60, OnStellate60)
	ON_UPDATE_COMMAND_UI(IDC_STELLATE_200, OnUpdateStellate200)
	ON_UPDATE_COMMAND_UI(IDC_STELLATE_150, OnUpdateStellate150)
	ON_UPDATE_COMMAND_UI(IDC_STELLATE_125, OnUpdateStellate125)
	ON_UPDATE_COMMAND_UI(IDC_STELLATE_100, OnUpdateStellate100)
	ON_UPDATE_COMMAND_UI(IDC_STELLATE_75, OnUpdateStellate75)
	ON_UPDATE_COMMAND_UI(IDC_STELLATE_60, OnUpdateStellate60)
	ON_UPDATE_COMMAND_UI(IDC_STELLATE_OTHER, OnUpdateStellateOther)
	ON_UPDATE_COMMAND_UI(IDC_KSTELLATE_200, OnUpdateKstellate200)
	ON_UPDATE_COMMAND_UI(IDC_KSTELLATE_100, OnUpdateKstellate100)
	ON_UPDATE_COMMAND_UI(IDC_KSTELLATE_60, OnUpdateKstellate60)
	ON_UPDATE_COMMAND_UI(IDC_KSTELLATE_OTHER, OnUpdateKstellateOther)
	ON_COMMAND(ID_SHAPE_MEDIAMANAGER, OnShapeMediamanager)
	ON_COMMAND(ID_MENU_SETIMAGECAPTION, OnMenuSetimagecaption)
	ON_COMMAND(IDC_LAYOUT_DEFAULT, OnLayoutDefault)
	ON_COMMAND(IDC_LAYOUT_SPLIT, OnLayoutSplit)
	ON_COMMAND(ID_UPDATE_LAYOUT_MENU, OnUpdateLayoutMenu)
	ON_UPDATE_COMMAND_UI(IDC_LAYOUT_DEFAULT, OnUpdateLayoutDefault)
	ON_UPDATE_COMMAND_UI_RANGE(IDC_LAYOUT_USER1, IDC_LAYOUT_USER9, OnUpdateLayoutUser)
	ON_COMMAND(ID_FILE_SAVE_DEFINITION, OnFileSaveDefinition)
	ON_COMMAND(IDC_SET_BASE_EDGE, OnSetBaseEdge)
	ON_COMMAND(IDC_ADD_TRIANGLE, OnAddTriangle)
	ON_COMMAND(IDC_ADD_SQUARE, OnAddSquare)
	ON_COMMAND(IDC_ADD_PENTAGON, OnAddPentagon)
	ON_COMMAND(IDC_ADD_HEXAGON, OnAddHexagon)
	ON_COMMAND(IDC_ADD_OCTAGON, OnAddOctagon)
	ON_COMMAND(IDC_ADD_DECAGON, OnAddDecagon)
	ON_COMMAND(IDC_DETACH_EDGE, OnDetachEdge)
	ON_UPDATE_COMMAND_UI(IDC_OUTDENT_FACE, OnUpdateOutdentFace)
	ON_COMMAND(IDC_OUTDENT_FACE, OnOutdentFace)
	ON_COMMAND(IDC_EDGE_FORCE_SPLIT, OnEdgeForceSplit)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPaperCutView construction/destruction

// Maximum scaling for view slider
double CPaperCutView::VIEWSCALE_MAX = 20.0;

// Name modifiers for adding polygons.
static char cNextTName = 'A';

CPaperCutView::CPaperCutView() :
	m_PrintOrg()
{
	m_bRotateMode = false;
	m_nRotation = 0;
	m_nTotalPages = 0;
	m_nCurrentPage = 0;
	m_nCurrentPageFace = 0;
	m_nMaxPartsPerPage = 12;
	m_cx = 0;
	m_cy = 0;
	m_bScaleValid = false;
	m_dScale = 1.0;
	m_dPrintScale = 1;
	m_pPrintFont = NULL;
	m_pRightClickFace = NULL;
	m_pRightClickEdge = NULL;
	m_mnuRightClick.LoadMenu( IDR_RIGHTCLICK );
	m_mnuDoc.LoadMenu( IDR_PAPERCTYPE );
	m_dPageWidthInLogicalUnits = 3;
	m_dPageWidthOverHeight = 0.762;
}

CPaperCutView::~CPaperCutView()
{
}

BOOL CPaperCutView::PreCreateWindow(CREATESTRUCT& cs)
{
	return CView::PreCreateWindow(cs);
}


/////////////////////////////////////////////////////////////////////////////
// CPaperCutView drawing

void CPaperCutView::OnDraw(CDC* pDC)
{
	CPaperCutDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// We should not end up here if printing
	if (pDC->IsPrinting() != 0)
	{
		return;
	}

	CString szTemp;

	// Create desktop color background
	CBrush brush;
	brush.CreateSysColorBrush( COLOR_WINDOW );
	//CSize s = pDC->GetWindowExt();
	// Use m_cx and m_cy
	int nLeftMargin = 5, nTopMargin = 5, nRightMargin = 5, nBottomMargin = 5;
	int nFontHeight = -12;
	//GetActiveWindow()->GetWindowRect( rw );
		//s.cx = r.Width();
		//s.cy = r.Height();
		CDbg::Out( "\n\nStarting window render extent = %d, %d\n", m_cx, m_cy );
	if (!m_bRotateMode)
	{
		CRect r;
		pDC->GetWindow()->GetClientRect( r );
		pDC->FillRect( r, &brush );
		// Draw page outline to show relative size of shape
	}
	brush.DeleteObject();
	pDC->SetTextAlign( TA_CENTER | TA_BASELINE );

	// If rotate mode, just draw an arrow
	if (m_bRotateMode)
	{
		// Draw an arrow starting at s.cx/2, s.cy/2
		double x = m_cx / 2;
		double y = m_cy / 2;
		pDC->MoveTo( (int)x, (int)y );
		CFace::VectorMove( x, y, this->m_nRotation, __min( m_cx, m_cy ) / 2 - 5 );
		pDC->LineTo( (int)x, (int)y );
		double x1 = x;
		double y1 = y;
		CFace::VectorMove( x, y, m_nRotation-180+20, 20 );
		pDC->LineTo( (int)x, (int)y );
		pDC->MoveTo( (int)x1, (int)y1 );
		CFace::VectorMove( x1, y1, m_nRotation+180-20, 20 );
		pDC->LineTo( (int)x1, (int)y1 );
		return;
	} // Rotating

	// If shape not valid, nothing to draw
	if (!pDoc->m_Shape.IsValid() || !pDoc->m_pShapeLayout)
	{
		szTemp = "Shape is not valid!";
		if (pDoc->m_Shape.IsValid() && !pDoc->m_pShapeLayout)
		{
			szTemp = "Shape has no layout!";
		}
		pDC->TextOut( m_cx/2, m_cy/2, szTemp );
		return;
	} // Shape is not valid

	// If redoing layout, nothing to draw
	if (!pDoc->m_pShapeLayout->IsValid())
	{
		szTemp = "Redrawing, please wait...";
		pDC->TextOut( m_cx / 2, m_cy / 2, szTemp );
		return;
	}

	// Get a TrueType font that can rotate
	CFont f;
	f.CreateFont( nFontHeight, 0,	/* height, width */
		0,					/* escapement = baseline rotation */
		0,					/* orientation - not used */
		FW_DONTCARE,		/* any weight will do */
		0,					/* no italics */
		0,					/* no underline */
		0,					/* no strikeout */
		ANSI_CHARSET,		/* character set */
		OUT_TT_PRECIS,		/* make sure it's a truetype font */
		CLIP_DEFAULT_PRECIS,/* clip precision */
		DEFAULT_QUALITY,	/* maximum quality if printing */
		FF_SWISS,			/* pitch and family */
		"Arial"				/* typeface name */
		);
	CFont* pOldFont = pDC->SelectObject( &f );

	// Create face groups
	m_nTotalPages = pDoc->m_pShapeLayout->GetPageCount();
	int nTotalFaceGroups = m_nTotalPages;
	CPageGroup* pPage = pDoc->m_pShapeLayout->GetPage( m_nCurrentPage );
	if (pPage == NULL)
	{
		// Try getting first page
		m_nCurrentPage = 0;
		m_nCurrentPageFace = 0;
		pPage = pDoc->m_pShapeLayout->GetPage( m_nCurrentPage );
	}

	if (m_nTotalPages <= 0 || pPage == NULL)
	{
		pDC->SetTextAlign( TA_CENTER );
		CString szErrMsg;
		szErrMsg.Format( "Error: no page groups - internal error on layout!" );
		pDC->TextOut( m_cx / 2, m_cy / 2, szErrMsg );
		pDC->SelectObject( pOldFont );
		f.DeleteObject();
		return;
	} // Nothing to show

	// Is current face group within page valid?
	int nFacesOnPage = pPage->GetNumFaceGroups();
	if (m_nCurrentPageFace >= nFacesOnPage)
	{
		m_nCurrentPageFace = 0;
	}

	// Set current page for the page group
	CDbg::Out( "Setting page for page group %08x to %d\n", (DWORD)pPage, m_nCurrentPage );
	pPage->SetPageNumber( m_nCurrentPage );

	// Get index into face group list
	int nFaceIndex = pPage->GetAbsoluteIndexFor( m_nCurrentPageFace );

	// Get bounding rectangle for current page in current shape units
	double dHeight, dWidth, dMinX, dMinY;
	//CDbg::Out( "\n\nGetting bounding rectangle\n" );
	pDoc->m_pShapeLayout->GetBoundingRectangleFor( nFaceIndex, dWidth, dHeight, dMinX, dMinY );
	//CDbg::Out( "Got %f w %f h min x %f y %f\n", dWidth, dHeight, dMinX, dMinY );

	// Rotation not supported yet
	//Layout.m_dOrientation = m_nRotation;

	// Scale shape units to device units
	if (dHeight == 0.0 || dWidth == 0.0)
	{
		CDbg::Out( 0, "Error: invalid size %f, %f\n", dHeight, dWidth );
		pDC->SelectObject( pOldFont );
		f.DeleteObject();
		return;
	}

	double dScale = (m_cy - nTopMargin - nBottomMargin) / dHeight;
	double dScalew = (m_cx - nLeftMargin - nRightMargin) / dWidth;
	double dScalePrint = (m_cy /*- pDC->GetDeviceCaps(LOGPIXELSY)/1.5*/) / pPage->GetHeight();
	double dScalePrintw = (m_cx /*- pDC->GetDeviceCaps(LOGPIXELSX)/1.5*/) / pPage->GetWidth();
	CDbg::Out( "Selecting scale based on window size %d,%d log height %f log width %f,\n\thscale %f, wscale %f, phscale %f, pwscale %f\n",
		m_cx, m_cy,
		dHeight, dWidth,
		dScale, dScalew,
		dScalePrint, dScalePrintw );
	if (dScalew < dScale)
	{
		CDbg::Out( "Using width scales %f print %f\n", dScalew, dScalePrintw );
		dScale = dScalew;
		dScalePrint = dScalePrintw;
	}

	brush.CreateSolidBrush( RGB( 20, 20, 20 ) );

	CDbg::Out( "\n-----------\nStarting page %d face %d abs %d render with scale %f, min x %f, min y %f\n",
		m_nCurrentPage, m_nCurrentPageFace, nFaceIndex, dScale, dMinX, dMinY );

	// Calculate origin so that minimum extremum is at xmargin, ymargin
	CLogPoint Org( nLeftMargin - ( dMinX * dScale ), nTopMargin - ( dMinY * dScale ) );

	CDbg::Out( "Set origin to %f, %f :- max x=%d, y=%d\n", Org.m_dX, Org.m_dY, (int)(dScale * dWidth), (int)(dScale * dHeight) );

		pDC->SetTextAlign( TA_RIGHT | TA_BOTTOM );
		szTemp.Format( "face %d/%d on page %d/%d",
			m_nCurrentPageFace+1,
			nFacesOnPage,
			m_nCurrentPage+1,
			m_nTotalPages );
		CSize st;
		st = pDC->GetTextExtent( szTemp );
		pDC->TextOut( m_cx - 5, m_cy - 5, szTemp );
		szTemp.Format( "%d faces, %d vertices, %d groups",
			pDoc->m_Shape.GetNumFaces(),
			pDoc->m_Shape.m_aMultiVertices.GetSize(),
			pDoc->m_pShapeLayout->GetFaceGroupCount() );
		pDC->TextOut( m_cx - 5, m_cy - st.cy - 5 - 5, szTemp );

	CDbg::Out( "org %f, %f\r\n", Org.m_dX, Org.m_dY );

	//CSize ar = pDC->GetAspectRatioFilter();
	CDbg::Out( "Starting render\r\n" );
	pDC->SetTextAlign( TA_CENTER | TA_BASELINE );
	CFace* pf;
	if (pf = pDoc->m_pShapeLayout->GetBaseFace( nFaceIndex ))
	{
		// Set reference to page group
		pf->SetPageGroup( pPage );
		// FIXME Layout will need to calculate orientation and origin
		// for all
		pf->Render( pDC, NULL, 0, Org, pDoc->m_pShapeLayout->m_pShape->GetActiveDivision(),
					fmod( pDoc->m_pShapeLayout->m_dOrientation + pDoc->m_pShapeLayout->m_adOptimalOrientation[nFaceIndex], 360.0 ),
					dScale, dScalePrint );
	} // for all face groups
	CDbg::Out( "==============\nEnding render\n" );

	brush.DeleteObject();

	// Restore font
	pDC->SelectObject( pOldFont );

	// Free font resource
	f.DeleteObject();
}

/////////////////////////////////////////////////////////////////////////////
// CPaperCutView printing

BOOL CPaperCutView::OnPreparePrinting(CPrintInfo* pInfo)
{
	//CDbg::m_Level++;
	// Set the length of the document
	m_nPrintTotalPages = GetDocument()->m_pShapeLayout->GetPageCount();
	pInfo->m_strPageDesc.Format( "OnPreparePrinting(): Printing shape, %d pages", m_nPrintTotalPages );
	pInfo->SetMinPage( 1 );
	pInfo->SetMaxPage( m_nPrintTotalPages );
	if (pInfo->m_bPreview)
	{
		CDbg::Out( "Preparing for preview\n" );
		pInfo->m_nNumPreviewPages = m_nPrintTotalPages;
	}
	// Try to select closest paper size
	// FIXME
	CDbg::Out( "OnPreparePrinting(): Printing %d pages to %s\n", 
		m_nPrintTotalPages,
		(LPCTSTR)pInfo->m_pPD->GetDriverName() );
	LPDEVMODE lpdm = pInfo->m_pPD->GetDevMode();
	if (lpdm)
	{
		CDbg::Out( "Paper orient %d (%s) size %u, width %.1fmm, height %.1fmm\n",
			lpdm->dmOrientation, lpdm->dmOrientation==1 ? "port" : lpdm->dmOrientation==2 ? "land" : "?unk", 
			lpdm->dmPaperSize, lpdm->dmPaperWidth/10.0, lpdm->dmPaperLength/10.0 );
	}

	//CDbg::m_Level++;
	// Make sure everything is valid
	CPaperCutDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc->m_Shape.IsValid() || !pDoc->m_pShapeLayout)
	{
		// Abort
		AfxMessageBox( "Cannot print" );
		return FALSE;
	} // Not in a valid state

	if (!pDoc->m_pShapeLayout->IsValid())
	{
		AfxMessageBox( "Redrawing, cannot print" );
		return FALSE;
	}

	// default preparation
	return DoPreparePrinting(pInfo);
}

void CPaperCutView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	//CDbg::m_Level++;

	// Allocate GDI resources
	m_pPrintFont = new CFont();
	m_pPrintFont->CreateFont( -36, 0,	/* height, width */
		0,		/* escapement */
		0,		/* orient */
		FW_DONTCARE,
		0,		/* no italics */
		0,		/* no underline */
		0,		/* no strikeout */
		ANSI_CHARSET,		/* character set */
		OUT_TT_PRECIS,		/* truetype please */
		CLIP_DEFAULT_PRECIS,
		PROOF_QUALITY,
		FF_SWISS,
		"Arial"
		);

	// Pre-print - start with page 1
	m_nPrintStartPage = pInfo->GetFromPage();
	m_nPrintEndPage = pInfo->GetToPage();
	CDbg::Out( "OnBeginPrinting: printing pages from %d to %d\n",
		m_nPrintStartPage, m_nPrintEndPage );
	// Width and height of screen in pixels
	int nHorzRes = pDC->GetDeviceCaps( HORZRES );
	int nVertRes = pDC->GetDeviceCaps( VERTRES );
	// Use margins of 1/3 inch
	// Pixels per logical inch along width and height, respectively
	int nYRes = pDC->GetDeviceCaps( LOGPIXELSY );
	int nXRes = pDC->GetDeviceCaps( LOGPIXELSX );
	// Printer-specific device caps..
	// PHYSICALWIDTH, PHYSICALHEIGHT: width and height of physical page in device units
	// PHYSICALOFFSETX, PHYSICALOFFSETY: offset from left and top edge to start of printable area in device units
	int nPhysWidth = pDC->GetDeviceCaps( PHYSICALWIDTH );
	int nPhysHeight = pDC->GetDeviceCaps( PHYSICALHEIGHT );
	int nPhysOffsetX = pDC->GetDeviceCaps( PHYSICALOFFSETX );
	int nPhysOffsetY = pDC->GetDeviceCaps( PHYSICALOFFSETY );
	CDbg::Out( "Physical offset = {%d,%d} size = {%d,%d} nRes = {%d,%d}\n", 
		nPhysOffsetX, nPhysOffsetY, nPhysWidth, nPhysHeight, nXRes, nYRes );
		int nTopMargin = nYRes / 4;
		int nBottomMargin = nYRes / 2;
		int nLeftMargin = nXRes / 4;
		int nRightMargin = nLeftMargin;
		CDbg::m_Level++; // DELETEME
		m_cxPrint = nHorzRes ;// - nLeftMargin - nRightMargin;
		int cxPhys = nPhysWidth - nLeftMargin - nRightMargin;
		m_cyPrint = nVertRes ;// - nTopMargin - nBottomMargin;
		int cyPhys = nPhysHeight - nTopMargin - nBottomMargin;
		CDbg::Out( "Printing to %s\n", (LPCTSTR)pInfo->m_pPD->GetDeviceName() );
		CDbg::Out( "cxyPrint=%d,%d cxyPhys=%d,%d margin{lrtb}=%d,%d,%d,%d\n"
					"res=%d,%d printable=%d,%d phys offset=%d,%d\n", 
						m_cxPrint, m_cyPrint, cxPhys + nLeftMargin + nRightMargin, cyPhys + nTopMargin + nBottomMargin,
							nLeftMargin, nRightMargin, nTopMargin, nBottomMargin,
						nXRes, nYRes, cxPhys, cyPhys, nPhysOffsetX, nPhysOffsetY );
		m_cxPrint = cxPhys;
		m_cyPrint = cyPhys;
		CDbg::m_Level--; // DELETEME
	// Calculate scale based on maximum page
	//pInfo->m_rectDraw is current usable area
//	CDbg::Out( "Usable print area is %u X %u\n", pInfo->m_rectDraw.Width(),
//		pInfo->m_rectDraw.Height() );
		CDbg::Out( "Print area is %u X %u (%.fx%.f), margins lrtb %d, %d, %d, %d\n",
			m_cxPrint, m_cyPrint, (double)nHorzRes / nXRes, (double)nVertRes / nYRes,
			nLeftMargin, nRightMargin, nTopMargin, nBottomMargin );
	// Regardless of the from and to pages, get
	// maximum rectangle for ALL pages...

	// Get max bounding rectangle for all pages in current shape units
	// We'll have to get origin for each page - this is page 1 orgx,y but
	// the max of all page widths...
	CLogRect rMax;
	int nPage;
	int nTotalPages = GetDocument()->m_pShapeLayout->GetPageCount();
	CDbg::Out( "Layout reports %d pages\n", nTotalPages );
	for (nPage = 0; nPage < nTotalPages; nPage++)
	{
		CPageGroup* pPage = GetDocument()->m_pShapeLayout->GetPage( nPage );
		ASSERT( pPage != NULL );
		if ((*pPage) > rMax)
		{
			rMax = (*pPage);
			if (nPage>0) CDbg::Out( "new max for page %d: w %.f h %.f\n", nPage+1, rMax.GetWidth(), rMax.GetHeight() );
		}
	}
	//GetDocument()->m_pShapeLayout->GetBoundingMaxRectangleFor( pInfo->m_nCurPage-1, dWidth, dHeight, dMinX, dMinY );

	CDbg::Out( "for page %d: w %.f h %.f minx %.f miny %.f\n", pInfo->m_nCurPage,
		rMax.GetWidth(), rMax.GetHeight(),
		rMax.GetMinX(), rMax.GetMinY() );

	// Calculate scale
	m_dPrintScale = m_cyPrint / rMax.GetHeight();
	double dScalew = m_cxPrint / rMax.GetWidth();
	// HG 2006-01-17 Does this work?
	if (dScalew < m_dPrintScale)
	{
		CDbg::Out( "adjusting print scale down from %.f to width scale %.f\n", m_dPrintScale, dScalew );
		m_dPrintScale = dScalew;
	}

	CDbg::Out( "print scale %.f\n", m_dPrintScale );

}

// Called before each StartPage
void CPaperCutView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo) 
{
	CDbg::Out( "OnPrepareDC()\n" );
	// If pInfo != NULL, we're printing
	CView::OnPrepareDC(pDC, pInfo);
}

void CPaperCutView::OnPrint(CDC* pDC, CPrintInfo* pInfo) 
{
	// from and to are the pages we are supposed to print,
	// as selected in the print dialog
	m_nPrintStartPage = pInfo->GetFromPage();
	m_nPrintEndPage = pInfo->GetToPage();
	// Render only pInfo->m_nCurPage
	if (pInfo->m_bPreview)
	{
		int nOldPrintEndPage = m_nPrintEndPage;
		CDbg::Out( "Print preview, showing page %d of %d (old end %d)\n", 
			pInfo->m_nCurPage, pInfo->m_nNumPreviewPages, m_nPrintEndPage );
		m_nPrintStartPage = pInfo->m_nCurPage;
		m_nPrintEndPage = m_nPrintStartPage + pInfo->m_nNumPreviewPages - 1;
		if (m_nPrintEndPage < m_nPrintStartPage)
		{
			pInfo->m_nNumPreviewPages = 1;
			m_nPrintEndPage = nOldPrintEndPage;
		}
	}
	//m_nCurPage = m_nPrintStartPage;
	CDbg::Out( "OnPrint, pages %d to %d\n", m_nPrintStartPage, m_nPrintEndPage );
	//CView::OnPrint(pDC, pInfo);

	// Select font
	CFont* pOldFont = pDC->SelectObject( m_pPrintFont );

	// Use 1/2" for bottom margin, 1/3" for all others
	int nTopMargin = pDC->GetDeviceCaps( LOGPIXELSY ) / 4;
	int nBottomMargin = pDC->GetDeviceCaps( LOGPIXELSY ) / 2;
	int nLeftMargin = pDC->GetDeviceCaps( LOGPIXELSX ) / 4;
	int nRightMargin = nLeftMargin;

	pDC->SetTextAlign( TA_CENTER | TA_BASELINE );
	CPageGroup* pPage;
	//CFace* pf;
	//int nFaceIndex = pInfo->m_nCurPage - 1;
	CPaperCutDoc* pDoc = GetDocument();
	pPage = pDoc->m_pShapeLayout->GetPage( pInfo->m_nCurPage - 1 );
	ASSERT( pPage != NULL );
	if (pInfo->m_bPreview && CDbg::m_Level > 0)
	{
		// Draw a box to show what we think are the page extents
		CDbg::Out( "Preview: usable page width %d, height %d\n", 
			pInfo->m_rectDraw.Width(), pInfo->m_rectDraw.Height() );
		CRect rc( pInfo->m_rectDraw );
		//rc.DeflateRect( 100, 100 );
		CLogRect rMax;
		rMax = (*pPage);
		CDbg::Out( "log rect max for page %d: w %.f h %.f scale %.f sw=%d sh=%d\n", pInfo->m_nCurPage, 
			rMax.GetWidth(), rMax.GetHeight(), this->m_dPrintScale,
			(int)(rMax.GetWidth() * m_dPrintScale), (int)(rMax.GetHeight() * m_dPrintScale) );

		CBrush br, bryellow;
		CPen pe;
		br.CreateSolidBrush(RGB(200,240,180));
		bryellow.CreateSolidBrush(RGB(210,200,240));
		CBrush* pbrOld = pDC->SelectObject( &br );
		pe.CreatePen( PS_SOLID, 3, RGB(10,10,10) );
		CPen* ppeOld = pDC->SelectObject( &pe );
		pDC->Rectangle( rc );
		pDC->SelectObject( &bryellow );
		rc.OffsetRect( 6, 6 );
		rc.left += 4;
		rc.right = rc.left + (int)(rMax.GetWidth() * m_dPrintScale);
		rc.top += 4;
		rc.bottom = rc.top + (int)(rMax.GetHeight() * m_dPrintScale);
		//rc.DeflateRect( pInfo->m_rectDraw.Width() - rMax.GetWidth() * this->m_dPrintScale,
		//		pInfo->m_rectDraw.Height() - rMax.GetHeight() * this->m_dPrintScale );
		pDC->Rectangle( rc );
		pDC->SelectObject( ppeOld );
		pDC->SelectObject( pbrOld );
	}
	int nPageFace;
	for (nPageFace = 0; nPageFace < pPage->GetNumFaceGroups(); nPageFace++)
	{
		CFace* pf = pPage->GetFaceGroup( nPageFace );
		ASSERT( pf != NULL );

		// Get layout rectangle
		CLogRect FaceRect;
		bool bGotRect = pPage->GetFaceRect( nPageFace, FaceRect );
		ASSERT( bGotRect );

		// Calculate origin so that minimum extremum is at xmargin, ymargin
		m_PrintOrg = FaceRect.GetOrg();
		m_PrintOrg.m_dX *= m_dPrintScale;
		m_PrintOrg.m_dY *= m_dPrintScale;
		// Offset by margin
		m_PrintOrg.m_dX += nLeftMargin;
		m_PrintOrg.m_dY += nTopMargin;
		//m_PrintOrg.Set( nLeftMargin - ( dMinX * m_dPrintScale ),
		//	nTopMargin - ( dMinY * m_dPrintScale ) );

		CDbg::Out( "org x %.f, y %.f\n", m_PrintOrg.m_dX, m_PrintOrg.m_dY );

		pf->Render( pDC, NULL, 0, m_PrintOrg, pDoc->m_pShapeLayout->m_pShape->GetActiveDivision(),
					fmod( /*pDoc->m_pShapeLayout->m_dOrientation +*/ FaceRect.GetOrientation(), 360.0 ),
					m_dPrintScale );
	}

	CString szFooterRight, szFooterLeft;
	szFooterLeft.Format( "Shape: %s", pDoc->m_Shape.GetName() );
	if (!pDoc->m_Shape.m_SavedPath.IsEmpty())
	{
		szFooterLeft += " (";
		szFooterLeft += pDoc->m_Shape.m_SavedPath;
		szFooterLeft += ")";
	}
	if (!pDoc->m_Shape.GetActiveDivision().IsEmpty())
	{
		szFooterLeft += "     Layout division: ";
		szFooterLeft += pDoc->m_Shape.GetActiveDivision();
	}
	szFooterLeft += "     printed: ";
	CTime dt;
	dt = CTime::GetCurrentTime();
	szFooterLeft += dt.Format("%c");
	szFooterRight.Format( "Page %d of %d", pInfo->m_nCurPage, m_nPrintTotalPages );
	pDC->SetTextAlign( TA_LEFT | TA_BASELINE );
	pDC->TextOut( nLeftMargin, nTopMargin + m_cyPrint - nBottomMargin, szFooterLeft );
	pDC->TextOut( nLeftMargin + m_cxPrint - pDC->GetTextExtent(szFooterRight).cx - nRightMargin, nTopMargin + m_cyPrint - nBottomMargin, szFooterRight );

	// Restore previously selected font
	pDC->SelectObject( pOldFont );
}

void CPaperCutView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// Free GDI resources allocated in OnBeginPrinting()
	if (m_pPrintFont)
	{
		m_pPrintFont->DeleteObject();
		delete m_pPrintFont;
		m_pPrintFont = NULL;
	}
	//if (CDbg::m_Level == 1)
	//{
	//	CDbg::m_Level = 0;
	//}
	//else
	//{
	//	CDbg::m_Level--;
	//}
	InvalidateRect( NULL );
}

/////////////////////////////////////////////////////////////////////////////
// CPaperCutView diagnostics

#ifdef _DEBUG
void CPaperCutView::AssertValid() const
{
	CView::AssertValid();
}

void CPaperCutView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CPaperCutDoc* CPaperCutView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CPaperCutDoc)));
	return (CPaperCutDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CPaperCutView message handlers

int CPaperCutView::SetParts(int nParts)
{
	// If no change, it's because we set it via CMainFrame::SetNumPages()
	if (nParts == m_nMaxPartsPerPage && nParts > 0)
	{
		return 0;
	}
	// Re-do layout
	// Rotate layout orientation and recalculate origin
	CPaperCutDoc* pDoc = GetDocument();
	if (nParts > 0 && pDoc && pDoc->m_Shape.IsValid() && pDoc->m_pShapeLayout
		&& (nParts < m_nTotalPages || nParts <= m_nTotalPages+1))
	{
		m_nMaxPartsPerPage = nParts;
		pDoc->m_pShapeLayout->Reset( &(pDoc->m_Shape) );
		m_nTotalPages = pDoc->m_pShapeLayout->AutoJoin( nParts );
		pDoc->m_pShapeLayout->CalculateAllBoundingRectangles();
		InvalidateRect( NULL );
	}
	return 0;
}

int CPaperCutView::SetRotation(int nFinalValue)
{
	// If no change, it's because we set it via CMainFrame::SetRotation()
	if (nFinalValue == m_nRotation)
	{
		return 0;
	}
	CDbg::Out( "New rotation: %d\n", nFinalValue );
	int nOldRotation = m_nRotation;
	m_nRotation = nFinalValue;
	// Rotate layout orientation and recalculate origin
	CPaperCutDoc* pDoc = GetDocument();
	if (pDoc && pDoc->m_Shape.IsValid() && pDoc->m_pShapeLayout)
	{
		pDoc->m_pShapeLayout->m_dOrientation = nFinalValue;
		pDoc->m_pShapeLayout->CalculateAllBoundingRectangles();
		InvalidateRect( NULL );
	}
	return nOldRotation;
}

int CPaperCutView::StartRotationDraw(int nStartingRotation)
{
	m_bRotateMode = true;
	m_nRotation = nStartingRotation;
	CDbg::Out( "StartRotationDraw %d\n", nStartingRotation );
	InvalidateRect( NULL );
	return 0;
}

int CPaperCutView::EndRotationDraw(int nFinalRotation)
{
	m_bRotateMode = false;
	m_nRotation = nFinalRotation + 1;
	return SetRotation( nFinalRotation );
}

void CPaperCutView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// FIXME Do something intelligent
	// for now just cycle through face groups
	CPaperCutDoc* pDoc = GetDocument();
	int nOldPage = m_nCurrentPage;
	int nOldPageFace = m_nCurrentPageFace;
	if (pDoc->m_pShapeLayout->IsValid())
	{
		int nTotalPages = pDoc->m_pShapeLayout->GetPageCount();
		CPageGroup* pPage = pDoc->m_pShapeLayout->GetPage( m_nCurrentPage );
		if (m_nCurrentPage > 0 && pPage == NULL)
		{
			m_nCurrentPageFace = 0;
			if (m_nCurrentPage < nTotalPages-1)
			{
				m_nCurrentPage++;
			}
			else
			{
				m_nCurrentPage = 0;
			}
			pPage = pDoc->m_pShapeLayout->GetPage( m_nCurrentPage );
		}
		ASSERT( pPage != NULL );
		int nTotalPageFaces = pPage->GetNumFaceGroups();
		if (m_nCurrentPage == nOldPage)
		{
			if (m_nCurrentPageFace < nTotalPageFaces-1)
			{
				m_nCurrentPageFace++;
			}
			else
			{
				m_nCurrentPage = (m_nCurrentPage + 1) % nTotalPages;
				m_nCurrentPageFace = 0;
			}
		} // Same page
	}
	if (m_nCurrentPage != nOldPage || m_nCurrentPageFace != nOldPageFace)
	{
		InvalidateRect( NULL );
	}
	CView::OnLButtonUp(nFlags, point);
}


void CPaperCutView::Stellate(double Percentage, BOOL bKepler /*=FALSE*/)
{
	// Get shape and delegate to shape
	CPaperCutDoc* pDoc = GetDocument();
	if (pDoc && pDoc->m_Shape.IsValid())
	{
		if (pDoc->m_Shape.Stellate( Percentage / 100.0, bKepler ) > 0)
		{
			pDoc->SetModifiedFlag();
			pDoc->UpdateLayout( m_dPageWidthInLogicalUnits, m_dPageWidthOverHeight );
			InvalidateRect( NULL );
		}
		else
		{
			AfxMessageBox( "Unable to perform this stellation. Note that K-stellation does not currently work when there are more than 20 faces." );
		}
	}
}

void CPaperCutView::OnStellate25() 
{
	Stellate( 25 );
}

void CPaperCutView::OnStellate33() 
{
	Stellate( 33 );
}

void CPaperCutView::OnStellate50() 
{
	Stellate( 50 );	
}

void CPaperCutView::OnStellate75() 
{
	Stellate( 75 );
}

void CPaperCutView::OnStellate100() 
{
	Stellate( 100 );
}

void CPaperCutView::OnStellate10() 
{
	Stellate( 10 );
}

void CPaperCutView::OnStellate125() 
{
	Stellate( 122 );
}

void CPaperCutView::OnStellate15() 
{
	Stellate( 15 );
}

void CPaperCutView::OnStellate150() 
{
	Stellate( 150 );
}

void CPaperCutView::OnStellate200() 
{
	Stellate( 200 );
}

void CPaperCutView::OnStellate60()
{
	Stellate( 60 );
}

void CPaperCutView::OnStellateOther() 
{
	CDlgSelectStellation dlg;
	if (dlg.DoModal() == IDOK)
	{
		Stellate( atof( dlg.m_Percentage.SpanExcluding( "%" ) ) );
	} // Apply
}

void CPaperCutView::SetPageAspect(double dWidthOverHeight)
{
	double dOld = m_dPageWidthOverHeight;
	m_dPageWidthOverHeight = dWidthOverHeight;
	CPaperCutDoc* pDoc = GetDocument();
	if (pDoc && pDoc->m_Shape.IsValid() &&
		dWidthOverHeight != dOld)
	{
		pDoc->UpdateLayout( m_dPageWidthInLogicalUnits, m_dPageWidthOverHeight );
		InvalidateRect( NULL );
	} // Redraw
}

void CPaperCutView::SetShapeSize(double dPageWidthInLogicalUnits)
{
	double dOld = m_dPageWidthInLogicalUnits;
	m_dPageWidthInLogicalUnits = dPageWidthInLogicalUnits;
	// Update status bar
	CPaperCutDoc* pDoc = GetDocument();
	if (pDoc && pDoc->m_Shape.IsValid() &&
		dPageWidthInLogicalUnits != dOld)
	{
		pDoc->UpdateLayout( m_dPageWidthInLogicalUnits, m_dPageWidthOverHeight );
		InvalidateRect( NULL );
	} // Redraw
}


void CPaperCutView::DrawPage(int nPage, double dScale, CDC *pDC)
{

}

void CPaperCutView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	
	// Invalidate previous scale
	m_bScaleValid = false;

	// Save new window dimensions
	m_cx = cx;
	m_cy = cy;
}

void CPaperCutView::OnShapeGeo2x() 
{
	Geodesic( 2 );
}

void CPaperCutView::OnShapeGeo3x() 
{
	Geodesic( 3 );
}

void CPaperCutView::OnShapeGeo4x() 
{
	Geodesic( 4 );
}

void CPaperCutView::OnUpdateShapeGeo(CCmdUI* pCmdUI) 
{
	// Enable these commands only for regular polyhedra with triangular faces
	CPaperCutDoc* pDoc = GetDocument();
	bool bEnable = (
			pDoc->m_Shape.IsValid()
		&& 	pDoc->m_Shape.IsRegular()
		&&	pDoc->m_Shape.IsTriangular()
		);
	// Furthermore, we only support windings for known polyhedra...
	if (bEnable)
	{
		int nFaces = pDoc->m_Shape.GetNumFaces();
		bEnable = (nFaces == 4 || nFaces == 6 || nFaces == 8 || nFaces == 20);
	}
	pCmdUI->Enable( bEnable );
}

void CPaperCutView::Geodesic(int Breakdown, bool bStellate /*=false*/)
{
	// Get shape and delegate to shape
	CPaperCutDoc* pDoc = GetDocument();
	if (pDoc && pDoc->m_Shape.IsValid())
	{
		if (pDoc->m_Shape.Geodesic( Breakdown, bStellate ) > 0)
		{
			pDoc->SetModifiedFlag();
			pDoc->UpdateLayout(m_dPageWidthInLogicalUnits, m_dPageWidthOverHeight );
			InvalidateRect( NULL );
		}
		else
		{
			AfxMessageBox( "Failed to break down shape.\nThis feature only works with polyhedra\nwhere all faces are equilateral triangles of equal size\ncurrently..." );
		}
	}
}

void CPaperCutView::OnUpdateViewLayoutSummary(CCmdUI* pCmdUI) 
{
	CPaperCutDoc* pDoc = GetDocument();
	pCmdUI->Enable(
			pDoc->m_Shape.IsValid()
		&& 	pDoc->m_pShapeLayout->IsValid()
		);
}

void CPaperCutView::OnViewLayoutSummary() 
{
	CLayoutSummaryDlg dlg;
	dlg.SetLayout( GetDocument()->m_pShapeLayout );
	dlg.DoModal();
}

void CPaperCutView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	CView::OnRButtonDown(nFlags, point);

	// Determine face from position (and optionally, nearest edge)
	CEdge* pEdge;
	m_pRightClickFace = GetFaceAt( point, pEdge );
	m_pRightClickEdge = NULL;
	if (!m_pRightClickFace)
	{
		return;
	}

	// Translate client to window coordinates
	ClientToScreen( &point );

	m_pRightClickEdge = pEdge;

	// Put up menu
	CMenu* pMenu = m_mnuRightClick.GetSubMenu(0);
	ASSERT( pMenu != NULL );
	// If there is an image loaded enable remove image and set caption
	this->m_aGroupsForFace.RemoveAll();
	if (m_pRightClickFace->m_pContent != NULL &&
		m_pRightClickFace->m_pContent->IsLoaded())
	{
		pMenu->EnableMenuItem( IDC_CLEAR_IMAGE, MF_BYCOMMAND | MF_ENABLED );
		pMenu->EnableMenuItem( ID_MENU_SETIMAGECAPTION, MF_BYCOMMAND | MF_ENABLED );
	}
	else
	{
		pMenu->EnableMenuItem( IDC_CLEAR_IMAGE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED );
		pMenu->EnableMenuItem( ID_MENU_SETIMAGECAPTION, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED );
	}
	pMenu->ModifyMenu( IDC_SHOW_FACE, MF_BYCOMMAND, IDC_SHOW_FACE, m_pRightClickFace->m_szFaceName );
#define GROUPMENU_INDEX	6
#define ADDPOLYMENU_INDEX 8
#define JOINMENU_INDEX 11
	CMenu* pGroupMenu = pMenu->GetSubMenu(GROUPMENU_INDEX);
	if (pGroupMenu)
	{
		if (m_pRightClickFace->HasGroups())
		{
			int nGroups = m_pRightClickFace->GetGroups( m_aGroupsForFace );
			int n;
			for (n = 0; n < 10; n++)
			{
				if (n < nGroups)
				{
					pGroupMenu->EnableMenuItem( IDC_REMOVE_GROUP1 + n, MF_BYCOMMAND | MF_ENABLED );
					pGroupMenu->ModifyMenu( IDC_REMOVE_GROUP1 + n, MF_BYCOMMAND, IDC_REMOVE_GROUP1 + n, this->m_aGroupsForFace.GetAt(n) );
				}
				else
				{
					pGroupMenu->EnableMenuItem( IDC_REMOVE_GROUP1 + n, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED );
					pGroupMenu->ModifyMenu( IDC_REMOVE_GROUP1 + n, MF_BYCOMMAND, IDC_REMOVE_GROUP1 + n, "" );
				}
			} // for all groups
			pMenu->EnableMenuItem( GROUPMENU_INDEX, MF_BYPOSITION | MF_ENABLED );
		} // Enable group delete
		else pMenu->EnableMenuItem( GROUPMENU_INDEX, MF_BYPOSITION | MF_DISABLED | MF_GRAYED );
	} // Got group menu
	else pMenu->EnableMenuItem( GROUPMENU_INDEX, MF_BYPOSITION | MF_DISABLED | MF_GRAYED );
	// Enable add menu iff outward edge is vacant
	// Join menu enabled iff outward edge is vacant
	// Detach menu enabled iff outward edge is set
	// Enable connect menu enabled iff edge is valid
	CMenu* pAddMenu = pMenu->GetSubMenu( ADDPOLYMENU_INDEX );
	this->m_aEdgeJoinCandidates.RemoveAll();
	// Find edge in doc shape
	CPaperCutDoc* pDoc = GetDocument();
	CEdge* pDocEdge = NULL;
	if (pDoc->m_Shape.IsValid())
	{
		pDocEdge = pDoc->m_Shape.FindFQEdge( pEdge->GetFQName() );
	}
	if (pDocEdge != NULL)
	{
		pMenu->EnableMenuItem( IDC_EDGE_FORCE_SPLIT, MF_BYCOMMAND | MF_ENABLED );
		pMenu->CheckMenuItem( IDC_EDGE_FORCE_SPLIT, MF_BYCOMMAND | (pDocEdge->GetForcedSplit() ? MF_CHECKED : MF_UNCHECKED) );
	}
	if (pDocEdge != NULL && pDocEdge->m_pOutwardConnectedEdge == NULL)
	{
		pMenu->EnableMenuItem( ADDPOLYMENU_INDEX, MF_BYPOSITION | MF_ENABLED );
		pMenu->EnableMenuItem( IDC_DETACH_EDGE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED );
		// Build join candidate list. Edges must have equal length and belong to other faces
		POSITION pos;
		CFace* pShapeFace;
		CString szFaceName;
		if (pDocEdge != NULL)
		{
			double dThisEdgeLength = pDocEdge->m_dLength;
			for (pos = pDocEdge->m_pFace->m_pOwner->m_mapFaces.GetStartPosition();
					pos != NULL; )
			{
				pDocEdge->m_pFace->m_pOwner->m_mapFaces.GetNextAssoc( pos, szFaceName, pShapeFace );
				if (szFaceName.Compare( pDocEdge->m_pFace->m_szFaceName ) == 0)
				{
					continue;
				}
				// Add unattached edges with identical length
				POSITION pose;
				CEdge* pShapeEdge;
				CString szEdgeName;
				for (pose = pShapeFace->m_pEdges.GetStartPosition(); pose != NULL; )
				{
					pShapeFace->m_pEdges.GetNextAssoc( pose, szEdgeName, pShapeEdge );
					if (pShapeEdge->m_pOutwardConnectedEdge == NULL && pShapeEdge->m_dLength == dThisEdgeLength)
					{
						CDbg::Out( "Rightclick: edge %08x (%s) in shape %08x not joined\n", 
							pShapeEdge, (LPCTSTR)pShapeEdge->GetFQName(), pShapeEdge->m_pFace->m_pOwner );
						this->m_aEdgeJoinCandidates.Add( pShapeEdge->GetFQName() );
					}
				}
			}
		} // Found edge in document
	}
	else
	{
		pMenu->EnableMenuItem( ADDPOLYMENU_INDEX, MF_BYPOSITION | MF_DISABLED | MF_GRAYED );
		pMenu->EnableMenuItem( IDC_DETACH_EDGE, MF_BYCOMMAND | MF_ENABLED );
	}
	// Set up join menu
	if (this->m_aEdgeJoinCandidates.GetSize() > 0)
	{
		CMenu* pJoinMenu = pMenu->GetSubMenu( JOINMENU_INDEX );
		pMenu->EnableMenuItem( JOINMENU_INDEX, MF_BYPOSITION | MF_ENABLED );
		int nJoin = 0;
		int nOverkill = 0;
		int nActualRemovals = 0;
		// Delete existing items
		while (nJoin < 1000)
		{
			if (!pJoinMenu->RemoveMenu( 0, MF_BYPOSITION ))
			{
				nOverkill++;
				if (nOverkill > 3)
				{
					break;
				}
			}
			else
			{
				nActualRemovals++;
			}
			nJoin++;
		}
		CDbg::Out( "Right-Click: %d menu items removed\n", nActualRemovals );
		// Re-create
		for (nJoin = 0; nJoin < this->m_aEdgeJoinCandidates.GetSize(); nJoin++)
		{
			MENUITEMINFO mif;
			mif.cbSize = sizeof(mif);
			mif.fMask = MIIM_STRING | MIIM_FTYPE | MIIM_ID;
			mif.dwTypeData = (LPSTR)(LPCTSTR)this->m_aEdgeJoinCandidates[nJoin];
			mif.cch = this->m_aEdgeJoinCandidates[nJoin].GetLength();
			mif.fType = MFT_STRING;
			mif.wID = ID_JOIN_EDGE_01 + nJoin;
			pJoinMenu->InsertMenuItem( nJoin, &mif, TRUE );
		}
		CDbg::Out( "Right-Click: %d join candidates added\n", nJoin );
	}
	else
	{
		pMenu->EnableMenuItem( JOINMENU_INDEX, MF_BYPOSITION | MF_DISABLED | MF_GRAYED );
	}
	BOOL bRes = pMenu->TrackPopupMenu( TPM_LEFTALIGN | TPM_LEFTBUTTON, point.x, point.y, this, NULL );
	//mnu.DestroyMenu();
	if (!bRes)
	{
		m_pRightClickFace = NULL;
		this->m_pRightClickEdge = NULL;
	} // Menu was dismissed

}

void CPaperCutView::OnJoinEdge( UINT nID )
{
	// Get index
	int idx = nID - ID_JOIN_EDGE_01;
	if (idx < 0 || idx >= this->m_aEdgeJoinCandidates.GetSize())
	{
		return;
	}
	// Get right click edge
	if (this->m_pRightClickEdge == NULL)
	{
		return;
	}
	CString szJoin, szThis, szOther;
	szThis = m_pRightClickEdge->GetFQName();
	szOther = m_aEdgeJoinCandidates[idx];
	szJoin.Format( "%s-%s", (LPCTSTR)szThis, (LPCTSTR)szOther );
	// Get shape in document
	CPaperCutDoc* pDoc = GetDocument();
	if (pDoc == NULL || !pDoc->m_Shape.IsValid())
	{
		return;
	}
	CString szErr;
	if (pDoc->m_Shape.JoinFromSingleEntry( szErr, szJoin ))
	{
		CDbg::Out( "OnJoinEdge: joined %s in shape %08x,\r\n  joining for edge %08x in shape %08x\n",
			(LPCTSTR)szJoin, &pDoc->m_Shape, m_pRightClickEdge, m_pRightClickEdge->m_pFace->m_pOwner );
		// Also join in layout
		bool bRes = m_pRightClickEdge->m_pFace->m_pOwner->JoinFromSingleEntry( szErr, szJoin );
		if (!bRes)
		{
			CDbg::Out( 0, "OnJoinEdge: failed to join %s in layout copy of shape: %s\n", (LPCTSTR)szJoin, (LPCTSTR)szErr );
		}
		// Rebuild unified vertex map
		pDoc->m_Shape.RebuildVertexMaps();
		// Recalc layout
		pDoc->SetModifiedFlag();
		// Redo layout - base face may have been deleted
		pDoc->UpdateLayout(m_dPageWidthInLogicalUnits, m_dPageWidthOverHeight );
		// Right-click reference is no longer valid - get edge pointer
		CEdge* pThisEdge = pDoc->m_Shape.FindFQEdge( szThis );
		CEdge* pOtherEdge = NULL;
		CShape* pThisShape = NULL;
		if (pThisEdge != NULL) 
		{
			pOtherEdge = pThisEdge->m_pOutwardConnectedEdge;
			pThisShape = pThisEdge->m_pFace->m_pOwner;
		}
		CDbg::Out( "OnJoinEdge: post layout, join for edge %08x in shape %08x is %08x\n",
			pThisEdge, pThisShape, pOtherEdge );
		InvalidateRect( NULL );
	}
}

void CPaperCutView::OnDropFiles(HDROP hDropInfo) 
{
	CPoint pt;
	if (::DragQueryPoint( hDropInfo, &pt ))
	{
		CPaperCutDoc* pDoc = GetDocument();
		ASSERT( pDoc != NULL );
		char szPath[1024];
		UINT nFiles = ::DragQueryFile( hDropInfo, 0xffffffff, szPath, 1 );
		UINT n;
		// Get face in layout
		CEdge* pEdge;
		CFace* pFace = GetFaceAt( pt, pEdge );
		if (pFace)
		{
			// Get face in doc
			pFace = pDoc->m_Shape.FindFace( pFace->m_szFaceName );
		}
		else
		{
			::AfxMessageBox( "Error: no face found - files must be dropped on a face" );
			return;
		}
		enum { WIND_RIGHT, WIND_LEFT, WIND_RDOWN, WIND_LDOWN } Direction;
		enum { FROM_BASE, FROM_LEFT, FROM_RIGHT } EntryFrom;
		Direction = WIND_RIGHT;
		// Always start with base edge
		pEdge = pFace->GetFirstEdge();
		EntryFrom = FROM_BASE;
		UINT nFaceCount = pDoc->m_Shape.GetNumFaces();
		for (n=0; pFace != NULL && n < nFiles; n++)
		{
			// If subsequent pass, find next face
			if (n > 0)
			{
				bool bDone = false;
				UINT nMissCounter = 0;
				while (!bDone && nMissCounter < nFaceCount)
				{
					bDone = true;
					if (EntryFrom == FROM_LEFT)
					{
						pEdge = pEdge->m_pEndPoints[0]->m_pSides[0];
					}
					if (EntryFrom == FROM_RIGHT)
					{
						pEdge = pEdge->m_pEndPoints[1]->m_pSides[1];
					}
					switch (Direction)
					{
					case WIND_RIGHT:
						pEdge = pEdge->m_pEndPoints[0]->m_pSides[0];
						if (!pEdge->m_pOutwardConnectedEdge)
						{
							nMissCounter++;
							Direction = WIND_RDOWN;
							break;
						}
						pEdge = pEdge->m_pOutwardConnectedEdge;
						pFace = pEdge->m_pFace;
						// For the next face, we're entering on the left
						EntryFrom = FROM_LEFT;
						if (pFace->m_pContent != NULL && pFace->m_pContent->IsLoaded())
						{
							Direction = WIND_RDOWN;
						}
						break;
					case WIND_LEFT:
					default:
						pEdge = pEdge->m_pEndPoints[1]->m_pSides[1];
						if (!pEdge->m_pOutwardConnectedEdge)
						{
							Direction = WIND_LDOWN;
							break;
						}
						pEdge = pEdge->m_pOutwardConnectedEdge;
						pFace = pEdge->m_pFace;
						// For next face, we're entering on the right
						EntryFrom = FROM_RIGHT;
						if (pFace->m_pContent != NULL && pFace->m_pContent->IsLoaded())
						{
							Direction = WIND_LDOWN;
						}
						break;
					case WIND_RDOWN:
						Direction = WIND_LEFT;
						if (!pEdge->m_pOutwardConnectedEdge)
						{
							Direction = WIND_RDOWN;
							break;
						}
						pEdge = pEdge->m_pOutwardConnectedEdge;
						pFace = pEdge->m_pFace;
						// For next face, we're entering on the base
						EntryFrom = FROM_BASE;
						break;
					case WIND_LDOWN:
						Direction = WIND_RIGHT;
						if (!pEdge->m_pOutwardConnectedEdge)
						{
							Direction = WIND_LDOWN;
							break;
						}
						pEdge = pEdge->m_pOutwardConnectedEdge;
						pFace = pEdge->m_pFace;
						// For next face, we're entering on the base
						EntryFrom = FROM_BASE;
						break;
					}
					if (pFace->m_pContent != NULL && pFace->m_pContent->IsLoaded())
					{
						bDone = false;
						nMissCounter++;
					}
					else if (bDone)
					{
						nMissCounter = 0;
					} // Reset miss counter
				} // not done
				// If nothing free found, quit
				if (pFace->m_pContent != NULL && pFace->m_pContent->IsLoaded())
				{
					CString sz;
					sz.Format( "Warning: only %d of %d files\r\nwere placed",
						n, nFiles );
					if (n < nFaceCount)
					{
						sz += " - some existing images\r\nwould have been overwritten.";
					}
					else
					{
						sz += " - all faces in shape filled.";
					}
					AfxMessageBox( sz );
					break;
				}
			} // Find next face
			// Get corresponding face in layout
			CFace* pLayoutFace = pDoc->m_pShapeLayout->m_pShape->FindFace( pFace->m_szFaceName );
			ASSERT( pLayoutFace != NULL );
		  if (::DragQueryFile( hDropInfo, n, szPath, sizeof( szPath ) ))
		  {
			// Try to load
			//if (pFace->m_pContent)
			//{
			//	//pFace->m_pDIB->DeleteObject();
			//	delete pFace->m_pContent;
			//}
			// Detach existing reference from layout back to doc
			if (pLayoutFace->m_pContent != NULL)
			{
				pLayoutFace->m_pContent->RemoveReference();
			}
			if (!pFace->m_pContent)
			{
				pFace->m_pContent = new CFaceContent();
			}
			pFace->m_pContent->AddReference();
			pLayoutFace->m_pContent = pFace->m_pContent;
			if (pFace->m_pContent->LoadFile( szPath ))
			{
				CString sz;
				sz.Format( "Error: failed to load %s", szPath );
				AfxMessageBox( sz );
			}
			else
			{
				pDoc->SetModifiedFlag();
				// FIXME only do this for part
				InvalidateRect( NULL );
			}
		  } // Got a file
		  else
		  {
			  CDbg::Out( 0, "Error: could not get file %d in drop group\n", n );
			  break;
		  }
		} // for all files
	}
//	CView::OnDropFiles(hDropInfo);
}

// Find the face at a given window position.  Return NULL if not found
CFace* CPaperCutView::GetFaceAt(const CPoint &pt, CEdge*& pEdge)
{
	CPaperCutDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// Use m_cx and m_cy
	int nLeftMargin = 5, nTopMargin = 5, nRightMargin = 5, nBottomMargin = 5;

	// Clear edge pointer
	pEdge = NULL;

	// If shape not valid, nothing to do
	if (!pDoc->m_Shape.IsValid() || !pDoc->m_pShapeLayout)
	{
		return NULL;
	} // Shape is not valid

	// If redoing layout, nothing to draw
	if (!pDoc->m_pShapeLayout->IsValid())
	{
		return NULL;
	}

	// Create face groups
	m_nTotalPages = pDoc->m_pShapeLayout->GetPageCount();
	int nTotalFaceGroups = m_nTotalPages;
	CPageGroup* pPage = pDoc->m_pShapeLayout->GetPage( m_nCurrentPage );
	if (pPage == NULL)
	{
		return NULL;
	}

	if (m_nTotalPages <= 0 || pPage == NULL)
	{
		return NULL;
	} // Nothing to show

	// Is current face group within page valid?
	int nFacesOnPage = pPage->GetNumFaceGroups();
	if (m_nCurrentPageFace >= nFacesOnPage)
	{
		return NULL;
	}

	// Get index into face group list
	int nFaceIndex = pPage->GetAbsoluteIndexFor( m_nCurrentPageFace );

	// Get bounding rectangle for current page in current shape units
	double dHeight, dWidth, dMinX, dMinY;
	//CDbg::Out( "\n\nGetting bounding rectangle\n" );
	pDoc->m_pShapeLayout->GetBoundingRectangleFor( nFaceIndex, dWidth, dHeight, dMinX, dMinY );
	//CDbg::Out( "Got %f w %f h min x %f y %f\n", dWidth, dHeight, dMinX, dMinY );

	// Rotation not supported yet
	//Layout.m_dOrientation = m_nRotation;

	// Scale shape units to device units
	if (dHeight == 0.0 || dWidth == 0.0)
	{
		return NULL;
	}

	double dScale = (m_cy - nTopMargin - nBottomMargin) / dHeight;
	double dScalew = (m_cx - nLeftMargin - nRightMargin) / dWidth;
	if (dScalew < dScale)
	{
		dScale = dScalew;
	}

	// Calculate origin so that minimum extremum is at xmargin, ymargin
	CLogPoint Org( nLeftMargin - ( dMinX * dScale ), nTopMargin - ( dMinY * dScale ) );

	CFace* pf;
	if (pf = pDoc->m_pShapeLayout->GetBaseFace( nFaceIndex ))
	{
		pf = pf->FindContainingFace( pt, NULL, 0, Org, pEdge,
					fmod( pDoc->m_pShapeLayout->m_dOrientation + pDoc->m_pShapeLayout->m_adOptimalOrientation[nFaceIndex], 360.0 ),
					dScale );
	} // for all face groups

	return pf;
}

void CPaperCutView::OnShowFace() 
{
	CString sz;
	if (!m_pRightClickFace)
	{
		sz = "No face selected";
	}
	else
	{
		sz = m_pRightClickFace->m_szFaceName;
	}
	//AfxMessageBox( sz );
}

void CPaperCutView::OnSetFacetext() 
{
	CPaperCutDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// If shape not valid, nothing to d
	if (!pDoc->m_Shape.IsValid() || !pDoc->m_pShapeLayout)
	{
		return;
	} // Shape is not valid

	// Get current face text
	CEditFace dlg;
	CString sz;
	if (!m_pRightClickFace)
	{
		AfxMessageBox( "No face selected" );
		return;
	}
	dlg.m_szFaceText = m_pRightClickFace->m_strBody;
	if (dlg.DoModal() == IDOK)
	{
		m_pRightClickFace->SetBody( dlg.m_szFaceText );
		CFace* pFace = pDoc->m_Shape.FindFace( m_pRightClickFace->m_szFaceName );
		if (pFace)
		{
			pFace->SetBody( dlg.m_szFaceText );
			pDoc->SetModifiedFlag();
		}
		InvalidateRect( NULL );
	} // Save new value
}

void CPaperCutView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();
	
	DragAcceptFiles();

}

void CPaperCutView::OnClearImage() 
{
	if (m_pRightClickFace == NULL)
	{
		AfxMessageBox( "No face selected" );
	}
	else if (m_pRightClickFace->m_pContent)
	{
		m_pRightClickFace->m_pContent->UnloadFile();
		InvalidateRect( NULL );
	}
}

void CPaperCutView::OnSetFocus(CWnd* pOldWnd) 
{
	CView::OnSetFocus(pOldWnd);
	
	// Set mainframe scale control
	CMainFrame* pMainFrame = (CMainFrame*)::AfxGetMainWnd();
	ASSERT( pMainFrame );

	// This will fire an update back, which we will ignore
	// because it will not have changed...
	pMainFrame->SetPageWidthInLogicalUnits( this->m_dPageWidthInLogicalUnits );
	pMainFrame->SetPageWidthOverHeight( this->m_dPageWidthOverHeight );
}

void CPaperCutView::OnRemoveFace() 
{
	CPaperCutDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// If shape not valid, nothing to do
	if (!pDoc->m_Shape.IsValid() || !pDoc->m_pShapeLayout)
	{
		return;
	} // Shape is not valid

	CString szFaceName;
	szFaceName = m_pRightClickFace->m_szFaceName;
	pDoc->m_Shape.DeleteFace( szFaceName );
	pDoc->m_pShapeLayout->m_pShape->DeleteFace( szFaceName );
	pDoc->SetModifiedFlag();
	// Redo layout - base face may have been deleted
	pDoc->UpdateLayout(m_dPageWidthInLogicalUnits, m_dPageWidthOverHeight );
	InvalidateRect( NULL );
}

void
CPaperCutView::OnRemoveGroup( UINT nID )
{
	int nIndex = nID - IDC_REMOVE_GROUP1;
	if (nIndex < 0 || nIndex >= 10)
	{
		CDbg::Out( 0, "Error: invalid id %u (%d, %x) passed to OnRemoveGroup\n", nID, nID, nID );
		return;
	} // Bogus
	if (nIndex >= this->m_aGroupsForFace.GetSize())
	{
		CDbg::Out( 0, "Error: index is out of range, only %d groups\n", m_aGroupsForFace.GetSize() );
		return;
	}
	CPaperCutDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// If shape not valid, nothing to do
	if (!pDoc->m_Shape.IsValid() || !pDoc->m_pShapeLayout)
	{
		return;
	} // Shape is not valid

	CString szTargetGroup;
	szTargetGroup = m_aGroupsForFace.GetAt(nIndex);
	CDbg::Out( "Removing group %s\n", (LPCTSTR)szTargetGroup );
	pDoc->m_Shape.DeleteFaceGroup( szTargetGroup );
	pDoc->m_pShapeLayout->m_pShape->DeleteFaceGroup( szTargetGroup );
	pDoc->SetModifiedFlag();
	// Redo layout - base face may have been deleted
	pDoc->UpdateLayout(m_dPageWidthInLogicalUnits, m_dPageWidthOverHeight );
	InvalidateRect( NULL );
} // CPaperCutView::OnRemoveGroup()


void CPaperCutView::OnPreferences() 
{
	CPaperCutDoc* pDoc = GetDocument();
	if (!pDoc || !pDoc->m_Shape.IsValid() || !pDoc->m_pShapeLayout || !pDoc->m_pShapeLayout->IsValid())
	{
		AfxMessageBox( "No shape to edit!" );
		return;
	} // No shape
	// Edit preferences for this shape only
	CPropertySheet dlg( "Shape Preferences" );
	CPrefTabsDrawing tabDrawing;
	CPrefTabsLayout tabLayout;
	tabDrawing.SetPrefs( &(pDoc->m_Shape) );
	dlg.AddPage( &tabDrawing );
	tabLayout.SetPrefs( &(pDoc->m_Shape) );
	dlg.AddPage( &tabLayout );
	// If OK, saved in dialog
	// Individual pages will save to memory
	if (dlg.DoModal() == IDOK)
	{
		CShape* pShape = pDoc->m_pShapeLayout->m_pShape;
		if (pShape)
		{
			tabDrawing.SaveAll( pShape );
			tabLayout.SaveAll( pShape );
		}
		pDoc->SetModifiedFlag();
		// Redraw with new settings
		InvalidateRect( NULL );
		RedrawWindow();
	} // Already in shape settings

}

void CPaperCutView::OnSetFaceimage() 
{
	CPaperCutDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// If shape not valid, nothing to d
	if (!pDoc->m_Shape.IsValid() || !pDoc->m_pShapeLayout)
	{
		return;
	} // Shape is not valid

	// Get current face image
	CString sz;
	if (!m_pRightClickFace)
	{
		AfxMessageBox( "No face selected" );
		return;
	}
	
	if (m_pRightClickFace->m_pContent && m_pRightClickFace->m_pContent->IsLoaded())
	{
		sz = m_pRightClickFace->m_pContent->GetPath();
	}

	// Find corresponding face in actual shape
	CFace* pFace = pDoc->m_Shape.FindFace( m_pRightClickFace->m_szFaceName );

	// Set up file dialog
	CFileDialog dlg( TRUE,
		NULL,
		sz,
		OFN_EXPLORER | OFN_FILEMUSTEXIST,
		"All supported image files (*.bmp, *.jpg, *.gif, *.tif, *.wmf)|*.bmp;*.jpg;*.gif;*.tif;*.wmf|All files (*.*)|*.*||",
		this );
	dlg.m_ofn.lpstrTitle = "Select image file";
	if (dlg.DoModal() == IDOK)
	{
		// Unload current image
		// Detach existing reference from layout back to doc
		if (m_pRightClickFace->m_pContent != NULL)
		{
			m_pRightClickFace->m_pContent->RemoveReference();
		}
		if (!pFace->m_pContent)
		{
			pFace->m_pContent = new CFaceContent();
		}
		pFace->m_pContent->AddReference();
		m_pRightClickFace->m_pContent = pFace->m_pContent;
		if (pFace->m_pContent->LoadFile( dlg.m_ofn.lpstrFile ))
		{
			CString szMsg;
			szMsg.Format( "Error: failed to load %s", dlg.m_ofn.lpstrFile );
			AfxMessageBox( szMsg );
		}
		else
		{
			pDoc->SetModifiedFlag();
			// FIXME only do this for part
			InvalidateRect( NULL );
		}
	}
} // OnSetFaceimage()

void CPaperCutView::OnGeofractalStellate() 
{
	Geodesic( 2, true );
}

void CPaperCutView::OnFractalStellate() 
{
	AfxMessageBox( "This feature not yet implemented" );
}

void CPaperCutView::OnUpdateGeofractalStellate(CCmdUI* pCmdUI) 
{
	OnUpdateShapeGeo( pCmdUI );
}

void CPaperCutView::OnStellateFace() 
{
	// Get shape and delegate to shape
	CPaperCutDoc* pDoc = GetDocument();
	if (pDoc && pDoc->m_Shape.IsValid())
	{
		// Get current face image
		CString sz;
		if (!m_pRightClickFace)
		{
			AfxMessageBox( "No face selected" );
			return;
		}
		// Get stellation
		CDlgSelectStellation dlg;
		dlg.m_Percentage = "100%";
		if (dlg.DoModal() == IDOK)
		{
			double dRatio = atof( dlg.m_Percentage.SpanExcluding( "%" ) ) / 100;
			if (dRatio > 0)
			{
				// Do it
				CFace* pFace = pDoc->m_Shape.FindFace( m_pRightClickFace->m_szFaceName );
				if (!pFace)
				{
					AfxMessageBox( "Internal error! Could not get face from right-click location." );
					return;
				}
				pDoc->m_Shape.ResetMinimumRatio();
				if (pDoc->m_Shape.StellateFace( pFace, dRatio ) > 0)
				{
					// Complain if minimum ratio was exceeded
					double dMinimumRatio;
					if (pDoc->m_Shape.GetMinimumStellationRatio( dMinimumRatio ))
					{
						CString sz;
						sz.Format( CShape::szMIN_STELLATION_RATIO_WARNING_fsf,
							dRatio * 100.0, "face", dMinimumRatio * 100.0 );
						::AfxMessageBox( sz, MB_OK );
					}
					pDoc->m_Shape.MapVertices();
					pDoc->SetModifiedFlag();
					pDoc->UpdateLayout(m_dPageWidthInLogicalUnits, m_dPageWidthOverHeight );
					InvalidateRect( NULL );
				}
				else
				{
					AfxMessageBox( "Stellate face failed" );
				}
			} // Valid ratio was specified
		} // Apply
	}
	
}

void CPaperCutView::OnViewNextFacegroup() 
{
	// Get shape
	CPaperCutDoc* pDoc = GetDocument();
	int nOldPage = m_nCurrentPage;
	int nOldPageFace = m_nCurrentPageFace;
	if (pDoc->m_pShapeLayout->IsValid())
	{
		int nTotalPages = pDoc->m_pShapeLayout->GetPageCount();
		CPageGroup* pPage = pDoc->m_pShapeLayout->GetPage( m_nCurrentPage );
		if (m_nCurrentPage > 0 && pPage == NULL)
		{
			// Wrapping disabled
			return;
		}
		ASSERT( pPage != NULL );
		int nTotalPageFaces = pPage->GetNumFaceGroups();
		// If face groups remain on this page, we're done
		if (m_nCurrentPageFace < nTotalPageFaces-1)
		{
			m_nCurrentPageFace++;
		}
		else
		{
			// Go to next page if possible
			if (m_nCurrentPage >= nTotalPages-1)
			{
				return;
			} // Not possible - no more pages
			// We've already eliminated wrapping, but the following code will support it
			m_nCurrentPage = (m_nCurrentPage + 1) % nTotalPages;
			// Start with the first face group
			m_nCurrentPageFace = 0;
		}
	}
	// Finally, redraw
	if (m_nCurrentPage != nOldPage || m_nCurrentPageFace != nOldPageFace)
	{
		InvalidateRect( NULL );
	}
	
}

void CPaperCutView::OnUpdateViewNextFacegroup(CCmdUI* pCmdUI) 
{
	// Enable IFF another face group exists and we're not already at the end
	CPaperCutDoc* pDoc = GetDocument();
	int nOldPage = m_nCurrentPage;
	int nOldPageFace = m_nCurrentPageFace;
	if (pDoc->m_pShapeLayout->IsValid())
	{
		int nTotalPages = pDoc->m_pShapeLayout->GetPageCount();
		CPageGroup* pPage = pDoc->m_pShapeLayout->GetPage( m_nCurrentPage );
		// If no page, disable
		if (pPage == NULL)
		{
			pCmdUI->Enable( FALSE );
			return;
		}
		// If there's another page, always enable
		if (m_nCurrentPage < nTotalPages-1)
		{
			pCmdUI->Enable();
			return;
		}
		ASSERT( pPage != NULL );
		int nTotalPageFaces = pPage->GetNumFaceGroups();
		// We're on the same page or there is no next page.
		// If no more face groups on this page, we're done.
		pCmdUI->Enable(m_nCurrentPageFace < nTotalPageFaces-1);
	}
	else pCmdUI->Enable( FALSE );
}

void CPaperCutView::OnViewNextPage() 
{
	// Get shape
	CPaperCutDoc* pDoc = GetDocument();
	int nOldPage = m_nCurrentPage;
	if (pDoc->m_pShapeLayout->IsValid())
	{
		int nTotalPages = pDoc->m_pShapeLayout->GetPageCount();
		CPageGroup* pPage = pDoc->m_pShapeLayout->GetPage( m_nCurrentPage );
		if (m_nCurrentPage > 0 && pPage == NULL)
		{
			// Wrapping disabled
			return;
		}
		ASSERT( pPage != NULL );
		// Go to next page if possible
		if (m_nCurrentPage >= nTotalPages-1)
		{
			return;
		} // Not possible - no more pages
		// We've already eliminated wrapping, but the following code will support it
		m_nCurrentPage = (m_nCurrentPage + 1) % nTotalPages;
		// Start with the first face group
		m_nCurrentPageFace = 0;
	}
	// Finally, redraw
	if (m_nCurrentPage != nOldPage)
	{
		InvalidateRect( NULL );
	}
	
}

void CPaperCutView::OnUpdateViewNextPage(CCmdUI* pCmdUI) 
{
	// Enable IFF another face group exists and we're not already at the end
	CPaperCutDoc* pDoc = GetDocument();
	int nOldPage = m_nCurrentPage;
	if (pDoc->m_pShapeLayout->IsValid())
	{
		int nTotalPages = pDoc->m_pShapeLayout->GetPageCount();
		CPageGroup* pPage = pDoc->m_pShapeLayout->GetPage( m_nCurrentPage );
		// If no page, disable
		if (pPage == NULL)
		{
			pCmdUI->Enable( FALSE );
			return;
		}
		// If there's another page, always enable
		pCmdUI->Enable( m_nCurrentPage < nTotalPages-1 );
	}
	else pCmdUI->Enable( FALSE );
}

void CPaperCutView::OnViewPrevFaceGroup() 
{
	// Get shape
	CPaperCutDoc* pDoc = GetDocument();
	int nOldPage = m_nCurrentPage;
	int nOldPageFace = m_nCurrentPageFace;
	if (pDoc->m_pShapeLayout->IsValid())
	{
		CPageGroup* pPage = pDoc->m_pShapeLayout->GetPage( m_nCurrentPage );
		if (m_nCurrentPage > 0 && pPage == NULL)
		{
			// Wrapping disabled
			return;
		}
		ASSERT( pPage != NULL );
		// If not the first face group on this page, we're done
		if (m_nCurrentPageFace > 0)
		{
			m_nCurrentPageFace--;
		}
		else
		{
			// Go to prev page if possible
			if (m_nCurrentPage == 0)
			{
				return;
			} // Not possible - no more pages
			m_nCurrentPage--;
			// Start with the first face group
			pPage = pDoc->m_pShapeLayout->GetPage( m_nCurrentPage );
			ASSERT( pPage != NULL );
			m_nCurrentPageFace = pPage->GetNumFaceGroups() - 1;
		}
	}
	// Finally, redraw
	if (m_nCurrentPage != nOldPage || m_nCurrentPageFace != nOldPageFace)
	{
		InvalidateRect( NULL );
	}
	
}

void CPaperCutView::OnUpdateViewPrevFaceGroup(CCmdUI* pCmdUI) 
{
	// Enable IFF another face group exists and we're not already at the beginning
	CPaperCutDoc* pDoc = GetDocument();
	int nOldPage = m_nCurrentPage;
	int nOldPageFace = m_nCurrentPageFace;
	if (pDoc->m_pShapeLayout->IsValid())
	{
		CPageGroup* pPage = pDoc->m_pShapeLayout->GetPage( m_nCurrentPage );
		// If no page, disable
		if (pPage == NULL)
		{
			pCmdUI->Enable( FALSE );
			return;
		}
		// If there's another page, always enable
		if (m_nCurrentPage > 0)
		{
			pCmdUI->Enable();
			return;
		}
		ASSERT( pPage != NULL );
		// We're on the same page or there is no previous page.
		// If no more face groups on this page, we're done.
		pCmdUI->Enable( m_nCurrentPageFace > 0 );
	}
	else pCmdUI->Enable( FALSE );
}

void CPaperCutView::OnViewPrevPage() 
{
	// Get shape
	CPaperCutDoc* pDoc = GetDocument();
	int nOldPage = m_nCurrentPage;
	if (pDoc->m_pShapeLayout->IsValid())
	{
		CPageGroup* pPage = pDoc->m_pShapeLayout->GetPage( m_nCurrentPage );
		if (pPage == NULL)
		{
			// Wrapping disabled
			return;
		}
		//ASSERT( pPage != NULL );
		// Go to prev page if possible
		if (m_nCurrentPage == 0)
		{
			return;
		} // Not possible - no more pages
		// We've already eliminated wrapping, but the following code will support it
		m_nCurrentPage--;
		// Start with the first face group
		m_nCurrentPageFace = 0;
	}
	// Finally, redraw
	if (m_nCurrentPage != nOldPage)
	{
		InvalidateRect( NULL );
	}
}

void CPaperCutView::OnUpdateViewPrevPage(CCmdUI* pCmdUI) 
{
	// Enable IFF another face group exists and we're not already at the beginning
	CPaperCutDoc* pDoc = GetDocument();
	int nOldPage = m_nCurrentPage;
	if (pDoc->m_pShapeLayout->IsValid())
	{
		CPageGroup* pPage = pDoc->m_pShapeLayout->GetPage( m_nCurrentPage );
		// If no page, disable
		if (pPage == NULL)
		{
			pCmdUI->Enable( FALSE );
			return;
		}
		pCmdUI->Enable( m_nCurrentPage > 0 );
	}
	else pCmdUI->Enable( FALSE );
}

void CPaperCutView::OnShapeDictionary()
{
	// Get shape dictionary
	CPaperCutDoc* pDoc = GetDocument();
	CShapeDictionary dlgDict( this, &pDoc->m_Shape.m_mapVariableDictionary );
	dlgDict.mapCB = &(MYAPP()->m_prefs.m_mapPredefs);
	// Dialog's OnOK handler will copy entries back to map
	if (dlgDict.DoModal() == IDOK)
	{
		pDoc->SetModifiedFlag();
		// Update copy of shape included in layout
		if (pDoc->m_pShapeLayout->IsValid())
		{
			pDoc->m_pShapeLayout->m_pShape->CopyDict( pDoc->m_Shape );
		}
		// Redraw
		InvalidateRect( NULL );
	}
}

void CPaperCutView::OnKstellate25()
{
	Stellate( 25, TRUE );
}

void CPaperCutView::OnKstellateOther()
{
	CDlgSelectStellation dlg;
	if (dlg.DoModal() == IDOK)
	{
		Stellate( atof( dlg.m_Percentage.SpanExcluding( "%" ) ), TRUE );
	} // Apply
}

void CPaperCutView::OnKstellate100()
{
	Stellate( 100, TRUE );
}

void CPaperCutView::OnKstellate50()
{
	Stellate( 50, TRUE );
}

void CPaperCutView::OnKstellate200()
{
	Stellate( 200, TRUE );
}

void CPaperCutView::OnKstellate60()
{
	Stellate( 60, TRUE );
}

void CPaperCutView::OnEditClip()
{
	// Get shape
	CPaperCutDoc* pDoc = GetDocument();
	if (pDoc && pDoc->m_Shape.IsValid())
	{
		CClipDelete dlgClip;
		dlgClip.m_pShape = &pDoc->m_Shape;
		if (dlgClip.DoModal() == IDOK)
		{
			pDoc->SetModifiedFlag();
			pDoc->UpdateLayout(m_dPageWidthInLogicalUnits, m_dPageWidthOverHeight );
			InvalidateRect( NULL );
		}
	}
}


// Delegate enabling menu items based on triangular faces
BOOL CPaperCutView::IsValidStellationCandidate()
{
	// Get shape
	CPaperCutDoc* pDoc = GetDocument();
	if (pDoc && pDoc->m_Shape.IsValid())
	{
		// Shapes where 3 <= maximum edges per face <= 4 can be cumulated
		int nMinSides, nMaxSides;
		double dMinLength, dMaxLength;
		pDoc->m_Shape.GetStats( nMinSides, nMaxSides, dMinLength, dMaxLength );
		if (nMinSides < 3 || nMaxSides > 4)
		{
			return FALSE;
		}
		// HGroover: I'm not sure why this check is necessary...
		CFace* pFace = pDoc->m_Shape.GetFirstFace();
		if (pFace != NULL)
		{
			// Stellation currently only supported for triangular and square faces
			int Edges = pFace->m_pEdges.GetSize();
			return (Edges == 3 || Edges == 4);
		}
	}
	return FALSE;
}

// Delegate enabling menu items based on triangular faces where face count in {4,6,8,20}
BOOL CPaperCutView::IsValidGeoCandidate()
{
	// Get shape
	CPaperCutDoc* pDoc = GetDocument();
	if (pDoc && pDoc->m_Shape.IsValid())
	{
		int nFaces = pDoc->m_Shape.GetNumFaces();
		if (nFaces != 4 && nFaces != 6 && nFaces != 8 && nFaces != 20)
		{
			return FALSE;
		}
		if (!pDoc->m_Shape.IsTriangular() || !pDoc->m_Shape.IsRegular())
		{
			return FALSE;
		}
		CFace* pFace = pDoc->m_Shape.GetFirstFace();
		if (pFace != NULL)
		{
			// Geodesic only supported where triangular faces with count of 4, 6, 8 or 20
			return (pFace->m_pEdges.GetSize() == 3);
		}
	}
	return FALSE;
}

void CPaperCutView::OnUpdateStellate200(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( IsValidStellationCandidate() );
}


void CPaperCutView::OnUpdateStellate150(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( IsValidStellationCandidate() );
}

void CPaperCutView::OnUpdateStellate125(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( IsValidStellationCandidate() );
}

void CPaperCutView::OnUpdateStellate100(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( IsValidStellationCandidate() );
}

void CPaperCutView::OnUpdateStellate75(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( IsValidStellationCandidate() );
}

void CPaperCutView::OnUpdateStellate60(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( IsValidStellationCandidate() );
}

void CPaperCutView::OnUpdateStellateOther(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( IsValidStellationCandidate() );
}

void CPaperCutView::OnUpdateKstellate200(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( IsValidGeoCandidate() );
}

void CPaperCutView::OnUpdateKstellate100(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( IsValidGeoCandidate() );
}

void CPaperCutView::OnUpdateKstellate60(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( IsValidGeoCandidate() );
}

void CPaperCutView::OnUpdateKstellateOther(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( IsValidGeoCandidate() );
}

void CPaperCutView::OnShapeMediamanager()
{
	CPaperCutDoc* pDoc = GetDocument();
	if (pDoc && pDoc->m_Shape.IsValid())
	{
		CMediaManagerDlg dlgMediaManager;
		dlgMediaManager.m_pShape = &pDoc->m_Shape;
		if (dlgMediaManager.DoModal() == IDOK)
		{
			;
		}
	}
}

void CPaperCutView::OnMenuSetimagecaption()
{
	CPaperCutDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// If shape not valid, nothing to do
	if (!pDoc->m_Shape.IsValid() || !pDoc->m_pShapeLayout)
	{
		return;
	} // Shape is not valid

	// Get current image caption
	CString sz;
	if (!m_pRightClickFace)
	{
		AfxMessageBox( "No face selected" );
		return;
	}
	CFaceContent* pImage = m_pRightClickFace->m_pContent;
	if (pImage == NULL)
	{
		AfxMessageBox( "No image on face" );
		return;
	}
	sz = pImage->GetCaption();

	CEditFace dlg;
	dlg.m_szFaceText = sz;
	if (dlg.DoModal() == IDOK)
	{
		pImage->SetCaption( dlg.m_szFaceText );
		CFace* pFace = pDoc->m_Shape.FindFace( m_pRightClickFace->m_szFaceName );
		if (pFace)
		{
			pDoc->SetModifiedFlag();
		}
		InvalidateRect( NULL );
	} // Save new value
}

void
CPaperCutView::OnLayoutSwitch( UINT nID )
{
	// Default layout is 0, user1-9 are 1-9...
	int nIndex = nID - IDC_LAYOUT_DEFAULT;
	if (nIndex < 0 || nIndex >= 10)
	{
		CDbg::Out( 0, "Error: invalid id %u (%d, %x) passed to OnLayoutSwitch()\n", nID, nID, nID );
		return;
	} // Bogus
	CPaperCutDoc* pDoc = GetDocument();
	if (!pDoc)
	{
		return;
	}

	// If shape not valid, nothing to do
	if (!pDoc->m_Shape.IsValid())
	{
		return;
	} // Shape is not valid

	// Check for valid index
	if (nIndex >= pDoc->m_Shape.GetDivisionCount())
	{
		CDbg::Out( 0, "Error: index %d is out of range, only %d divisions\n", 
			nIndex, pDoc->m_Shape.GetDivisionCount() );
		return;
	}

	// If no change to active division, nothing to do
	if (nIndex == pDoc->m_Shape.GetActiveDivisionIndex())
	{
		return;
	}

	// Change active division
	int nResult = pDoc->m_Shape.SetActiveDivision( nIndex );
	if (nResult < 0)
	{
		CDbg::Out( 0, "Error: set active division to %d failed\n", nIndex );
	}

	// Update checkmark in menu
	UpdateLayoutMenu();

	// Redo layout
	pDoc->UpdateLayout(m_dPageWidthInLogicalUnits, m_dPageWidthOverHeight );
	InvalidateRect( NULL );
} // CPaperCutView::OnLayoutSwitch()

void CPaperCutView::OnLayoutDefault()
{
	OnLayoutSwitch( IDC_LAYOUT_DEFAULT );
}

void CPaperCutView::OnLayoutSplit()
{
	// Get shape
	CPaperCutDoc* pDoc = GetDocument();
	if (!pDoc->m_pShapeLayout->IsValid())
	{
		::AfxMessageBox( "Layout invalid!" );
		return;
	}
	// Get splits
	CLayoutSplit dlgSplit;
	dlgSplit.m_pShapeLayout = pDoc->m_pShapeLayout;
	// If changed, update layout menu
	if (dlgSplit.DoModal() == IDOK)
	{
		// Apply changes in shape layout to actual shape
		CFace* pSrc;
		CFace* pDest;
		CString szFaceName;
		POSITION pos;
		for (pos = pDoc->m_pShapeLayout->m_pShape->m_mapFaces.GetStartPosition(); pos != NULL; )
		{
			pDoc->m_pShapeLayout->m_pShape->m_mapFaces.GetNextAssoc( pos, szFaceName, pSrc );
			if (pDoc->m_Shape.m_mapFaces.Lookup( szFaceName, pDest ))
			{
				pDest->SetLayoutDivision( pSrc->GetLayoutDivision() );
			}
		}
		// Get active division set in dialog
		pDoc->m_Shape.SetActiveDivision( pDoc->m_pShapeLayout->m_pShape->GetActiveDivision() );
		// Mark doc as dirty
		pDoc->SetModifiedFlag();
		// Put new divisions in menu (and remove deleted ones)
		UpdateLayoutMenu();
		// Redo layout
		pDoc->UpdateLayout( m_dPageWidthInLogicalUnits, m_dPageWidthOverHeight );
		InvalidateRect( NULL );
	}
}

// Update layout menu with alternate layout divisions
void CPaperCutView::UpdateLayoutMenu()
{
	int nAdditionalItems = 0;
	CMenu* pLayoutMenu = ::AfxGetMainWnd()->GetMenu();
	if (pLayoutMenu == NULL) 
	{
		CDbg::Out( 0, "UpdateLayoutMenu(): no menu on main window!\n" );
		return;
	}
	if (pLayoutMenu->m_hMenu == NULL) 
	{
		CDbg::Out( 0, "UpdateLayoutMenu(): main window menu handle == NULL!\n" );
		return;
	}
	pLayoutMenu = pLayoutMenu->GetSubMenu(5);
	if (pLayoutMenu == NULL) 
	{
		CDbg::Out( 0, "UpdateLayoutMenu(): GetSubMenu(5) returns NULL\n" );
		return;
	}
	// Get layout
	CPaperCutDoc* pDoc = GetDocument();
	if (pDoc->m_Shape.IsValid())
	{
		nAdditionalItems = pDoc->m_Shape.GetDivisionCount() - 1;
	}
	// Get starting index position
	int nStartingPosition = 0;
	while (nStartingPosition < 100 && pLayoutMenu->GetMenuItemID( nStartingPosition ) != IDC_LAYOUT_DEFAULT)
	{
		nStartingPosition++;
	}
	if (nStartingPosition>=100)
	{
		CDbg::Out( 0, "Error: unable to set menu" );
		return;
	}
	int nID;
	int nActive = pDoc->m_Shape.GetActiveDivisionIndex();
	// Set check on default
	pLayoutMenu->CheckMenuItem( IDC_LAYOUT_DEFAULT, MF_BYCOMMAND | (nActive == 0 ? MF_CHECKED : MF_UNCHECKED) );
	// Clear everything
	for (nID = IDC_LAYOUT_USER1; nID <= IDC_LAYOUT_USER9; nID++)
	{
		pLayoutMenu->RemoveMenu( nID, MF_BYCOMMAND );
	}
	// Add items
	int nAdditional;
	for (nAdditional = 1; nAdditional <= nAdditionalItems; nAdditional++)
	{
		unsigned int uFlags = MF_STRING | MF_ENABLED;
		if (nAdditional == nActive) 
			uFlags |= MF_CHECKED;
		else
			uFlags |= MF_UNCHECKED;
		CString szMenuName = "&";
		szMenuName += pDoc->m_Shape.GetDivisionName(nAdditional);
		pLayoutMenu->AppendMenu( uFlags, IDC_LAYOUT_USER1 + nAdditional - 1, szMenuName );
	}
}

// Event handler for internally-fired message
void 
CPaperCutView::OnUpdateLayoutMenu()
{
	CDbg::Out( 0, "Received OnUpdateLayoutMenu()\n" );
	// This happens too early to be really useful...
	UpdateLayoutMenu();
}

void
CPaperCutView::OnInitMenu(CMenu* pMenu)
{
	CView::OnInitMenu(pMenu);
	CDbg::Out( 0, "Received OnInitMenu()\n" );
	UpdateLayoutMenu();
}


void CPaperCutView::OnUpdateLayoutDefault(CCmdUI *pCmdUI)
{
	// Hopefully this is a place we can update it...
	CDbg::Out( 0, "Got OnUpdateLayoutDefault()\n" );
	pCmdUI->Enable( TRUE );
	CPaperCutDoc *pDoc = this->GetDocument();
	if (pDoc == NULL) return;
	BOOL IsDefault = (pDoc->m_Shape.GetActiveDivisionIndex() == 0);
	pCmdUI->SetCheck( IsDefault );
	// If more divisions, modify menu
	if (pDoc->m_Shape.GetDivisionCount() > 1)
	{
		UpdateLayoutMenu();
	}
}

static void UpdateUserLayout( CCmdUI *pCmdUI, int Index, CPaperCutDoc *pDoc )
{
	BOOL bShowing = (pDoc != NULL);
	if (bShowing)
	{
		bShowing = (pDoc->m_Shape.GetDivisionCount() > Index);
		if (bShowing)
		{
			CString szText = "&";
			szText += pDoc->m_Shape.GetDivisionName(Index);
			pCmdUI->SetText( szText );
			BOOL bChecked = (Index == pDoc->m_Shape.GetActiveDivisionIndex());
			pCmdUI->SetCheck( bChecked );
			pCmdUI->Enable( TRUE );
		}
		else
		{
			// Hide it
			//pCmdUI->m_pMenu->ModifyMenu( IDC_LAYOUT_DEFAULT + Index, MF_BYCOMMAND );
			pCmdUI->SetCheck( FALSE );
			pCmdUI->Enable( FALSE );
			pCmdUI->SetText( "" );
		}
	}
}

void CPaperCutView::OnUpdateLayoutUser(CCmdUI *pCmdUI)
{
	UINT nID = pCmdUI->m_nID;
	UpdateUserLayout( pCmdUI, nID - IDC_LAYOUT_DEFAULT, this->GetDocument() );
}


void CPaperCutView::OnFileSaveDefinition()
{
	// Save shape as definition
	// Get filename with .shapedef extension
	CPaperCutDoc* pDoc = GetDocument();
	if (!pDoc->m_Shape.IsValid())
	{
		return;
	}
	CFileDialog dlg( TRUE, ".shapedef", pDoc->m_Shape.GetName(), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "Shapedef files (*.shapedef)|*.shapedef|All files (*.*)|*.*||", this );
	if (dlg.DoModal() != IDOK)
	{
		return;
	}
	// Don't clear pDoc->modified flag - we don't save everything
	// FIXME Check for attributes we don't save and clear modified flag iff any of those were present
	pDoc->m_Shape.WriteAsDefinition( dlg.GetPathName() );
}

void CPaperCutView::OnSetBaseEdge()
{
	// Set selected edge as base edge
	if (m_pRightClickEdge != NULL)
	{
		// Find edge in doc
		CPaperCutDoc *pDoc = GetDocument();
		CEdge* pEdge = pDoc->m_Shape.FindFQEdge(m_pRightClickEdge->GetFQName());
		// Set in both layout and original shapes - this avoids re-doing layout
		m_pRightClickEdge->m_pFace->SetBaseEdgeName( m_pRightClickEdge->GetName() );
		pEdge->m_pFace->SetBaseEdgeName( m_pRightClickEdge->GetName() );
		// Redraw
		InvalidateRect( NULL );
	}
}

void CPaperCutView::OnDetachEdge()
{
	// Detach edge
	if (m_pRightClickEdge != NULL)
	{
		// Find edge in doc
		CPaperCutDoc *pDoc = GetDocument();
		CEdge* pEdge = pDoc->m_Shape.FindFQEdge(m_pRightClickEdge->GetFQName());
		if (pEdge == NULL)
		{
			return ;
		}
		// Is it joined?
		if (pEdge->m_pOutwardConnectedEdge == NULL)
		{
			// Not joined, nothing to do
			return ;
		}
		// Detach it
		pEdge->Detach();
		// Redo layout
		pDoc->m_Shape.MapVertices();
		pDoc->SetModifiedFlag();
		pDoc->UpdateLayout(m_dPageWidthInLogicalUnits, m_dPageWidthOverHeight );
		InvalidateRect( NULL );
	}
}

// Add polygon (type in {'T','S','P','H','O','D'}) to edge
BOOL CPaperCutView::AddPolygonToEdge( CEdge* pLayoutEdge, char cType /*= 'T'*/, int nSides /*= 3*/ )
{
	if (pLayoutEdge == NULL)
	{
		return FALSE;
	}
	// We can only add to an edge that doesn't already have a connection
	if (pLayoutEdge->m_pOutwardConnectedEdge != NULL)
	{
		return FALSE;
	}
	// Find edge in doc
	CPaperCutDoc *pDoc = GetDocument();
	CEdge* pEdge = pDoc->m_Shape.FindFQEdge(pLayoutEdge->GetFQName());
	if (pEdge == NULL)
	{
		return FALSE;
	}
	// Create equilateral polygon
	double dLength = pEdge->m_dLength;
	double adLengths[] = {dLength,dLength,dLength,dLength,
							dLength,dLength,dLength,dLength,
							dLength,dLength,dLength,dLength};
	LPCTSTR aszNames[] = {"a","b","c","d",
							"e","f","g","h",
							"i","j","k","l"};
	static double adAngles[] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
	CString szName = CFace::UniquePolyName(cType);
	CFace* pNewFace = new CFace(pEdge->m_pFace->m_pOwner, szName );
	pNewFace->CreatePolygon( aszNames, adLengths, adAngles, nSides );
	pNewFace->SetBaseEdgeName( "a" );
	pEdge->JoinOutward( pNewFace->GetEdge( "a" ) );
	// Add to shape
	pEdge->m_pFace->m_pOwner->m_mapFaces.SetAt( szName, pNewFace );
	// Redo layout
	pDoc->m_Shape.MapVertices();
	pDoc->SetModifiedFlag();
	pDoc->UpdateLayout(m_dPageWidthInLogicalUnits, m_dPageWidthOverHeight );
	InvalidateRect( NULL );

	return TRUE;
}

void CPaperCutView::OnAddTriangle()
{
	AddPolygonToEdge( m_pRightClickEdge, 'T', 3 );
}

void CPaperCutView::OnAddSquare()
{
	AddPolygonToEdge( m_pRightClickEdge, 'S', 4 );
}

void CPaperCutView::OnAddPentagon()
{
	AddPolygonToEdge( m_pRightClickEdge, 'P', 5 );
}

void CPaperCutView::OnAddHexagon()
{
	AddPolygonToEdge( m_pRightClickEdge, 'H', 6 );
}

void CPaperCutView::OnAddOctagon()
{
	AddPolygonToEdge( m_pRightClickEdge, 'O', 8 );
}

void CPaperCutView::OnAddDecagon()
{
	AddPolygonToEdge( m_pRightClickEdge, 'D', 10 );
}

void CPaperCutView::OnUpdateOutdentFace(CCmdUI *pCmdUI)
{
	// Enable iff face is quadrangular
	BOOL bOn = 0;
	if (this->m_pRightClickFace != NULL)
	{
		if (m_pRightClickFace->m_pEdges.GetCount() == 4)
		{
			bOn = 1;
		}
	}
	pCmdUI->Enable( bOn );
}

void CPaperCutView::OnOutdentFace()
{
	// Make sure face is quadrangular
	if (this->m_pRightClickFace == NULL)
	{
		::AfxMessageBox( "No face selected via right-click" );
		return;
	}
	if (m_pRightClickFace->m_pEdges.GetCount() != 4)
	{
		::AfxMessageBox( "Error: this operation is only supported for quadrangular faces" );
		return;
	}
	// Get shape and delegate to shape
	CPaperCutDoc* pDoc = GetDocument();
	if (pDoc && pDoc->m_Shape.IsValid())
	{
		// Get actual face
		CFace* pFace = pDoc->m_Shape.FindFace( m_pRightClickFace->m_szFaceName );
		if (!pFace)
		{
			AfxMessageBox( "Internal error! Could not get face from right-click location." );
			return ;
		}

		// Get proportions
		CDlgGetIndent dlg;
		dlg.m_SideProportion = 0.5;
		dlg.m_HeightProportion = 0.35;
		if (dlg.DoModal() != IDOK)
		{
			return;
		}

		int FacesAdded = pDoc->m_Shape.OutdentFace( pFace, dlg.m_HeightProportion, dlg.m_SideProportion );
		if (FacesAdded > 0)
		{
			pDoc->m_Shape.MapVertices();
			pDoc->SetModifiedFlag();
			pDoc->UpdateLayout( m_dPageWidthInLogicalUnits, m_dPageWidthOverHeight );
			InvalidateRect( NULL );
		}

	}

}

void CPaperCutView::OnEdgeForceSplit()
{
	if (m_pRightClickEdge == NULL)
	{
		::AfxMessageBox( "No edge selected via right-click" );
		return;
	}
	// We'll always recalculate even if there is no current join
	bool wasConnected = (m_pRightClickEdge->m_pOutwardConnectedEdge != NULL);
	// Find edge in doc
	CPaperCutDoc *pDoc = GetDocument();
	if (!pDoc || !pDoc->m_Shape.IsValid())
	{
		::AfxMessageBox( "Shape is not valid, cannot continue" );
		return;
	}
	CEdge* pEdge = pDoc->m_Shape.FindFQEdge(m_pRightClickEdge->GetFQName());
	if (pEdge == NULL)
	{
			AfxMessageBox( "Internal error! Could not get edge from right-click location." );
			return ;
	}

	// Toggle the current setting
	pEdge->SetForcedSplit( !pEdge->GetForcedSplit() );

	if (wasConnected || true)
	{
		// We've changed, recalculate layout
		pDoc->m_Shape.MapVertices();
		pDoc->SetModifiedFlag();
		pDoc->UpdateLayout( m_dPageWidthInLogicalUnits, m_dPageWidthOverHeight );
		InvalidateRect( NULL );
	}
}

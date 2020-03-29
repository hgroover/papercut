/*!

	@file	 PaperCutView.h
	
	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: PaperCutView.h 16 2006-04-15 06:39:12Z henry_groover $

  The primary view associated with CPaperCutDoc. Displays the flat net layout of a shape.

*/

#if !defined(AFX_PAPERCUTVIEW_H__9837ABB3_3851_11D6_A858_0040F4309CCE__INCLUDED_)
#define AFX_PAPERCUTVIEW_H__9837ABB3_3851_11D6_A858_0040F4309CCE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CLogPoint;

class CPaperCutView : public CView
{
protected: // create from serialization only
	CPaperCutView();
	DECLARE_DYNCREATE(CPaperCutView)

// Attributes
public:
	CPaperCutDoc* GetDocument();

	static double VIEWSCALE_MAX;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPaperCutView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);
	virtual void OnInitialUpdate();
	virtual void OnInitMenu(CMenu* pMenu);
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
	CFace* m_pRightClickFace;
	CEdge* m_pRightClickEdge;
	CFace* GetFaceAt( CPoint const& pt, CEdge*& pEdge );
	void Geodesic( int Breakdown, bool bStellate = false );
	// Add polygon (type in {'T','S','P','H','O'}) to edge
	BOOL AddPolygonToEdge( CEdge* pEdge, char cType = 'T', int nSides = 3 );
	double m_dPrintScale;
	void DrawPage( int nPage, double dScale, CDC* pDC );
	DOCINFO m_PrintDocInfo;
	int m_nMaxPartsPerPage;
	double m_dPageWidthInLogicalUnits;
	double m_dPageWidthOverHeight;
	void SetShapeSize( double dPageWidthInLogicalUnits );
	void SetPageAspect( double dWidthOverHeight );
	int m_nCurrentPage;
	int m_nCurrentPageFace;
	int m_nTotalPages;
	int EndRotationDraw( int nFinalRotation );
	int StartRotationDraw( int nStartingRotation );
	int SetRotation( int nFinalValue );
	int SetParts( int nParts );
	virtual ~CPaperCutView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Update layout menu with alternate layout divisions
	void UpdateLayoutMenu();

protected:

// Generated message map functions
protected:
	CMenu m_mnuRightClick;
	CMenu m_mnuDoc;
	int m_cyPrint;
	int m_cxPrint;
	int m_nPrintTotalPages;
	CFont* m_pPrintFont;
	CLogPoint m_PrintOrg;
	bool m_bScaleValid;
	int m_cy;
	int m_cx;
	CString m_PrintJobName;
	void Stellate( double Percentage, BOOL bKepler = FALSE );
	// Delegate enabling menu items based on triangular faces
	BOOL IsValidStellationCandidate();
	// Delegate enabling menu items based on triangular faces where face count in {4,6,8,20}
	BOOL IsValidGeoCandidate();
	int m_nPrintEndPage;
	int m_nPrintStartPage;
	double m_dScale;
	bool m_bRotateMode;
	int m_nRotation;
	CArray<CString,LPCTSTR> m_aGroupsForFace;
	CArray<CString,LPCTSTR> m_aEdgeJoinCandidates;
	//{{AFX_MSG(CPaperCutView)
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnStellate100();
	afx_msg void OnStellate25();
	afx_msg void OnStellate33();
	afx_msg void OnStellate75();
	afx_msg void OnStellate50();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnShapeGeo2x();
	afx_msg void OnShapeGeo3x();
	afx_msg void OnShapeGeo4x();
	afx_msg void OnUpdateShapeGeo(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewLayoutSummary(CCmdUI* pCmdUI);
	afx_msg void OnViewLayoutSummary();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnShowFace();
	afx_msg void OnSetFacetext();
	afx_msg void OnClearImage();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnRemoveFace();
	afx_msg void OnStellate10();
	afx_msg void OnStellate125();
	afx_msg void OnStellate15();
	afx_msg void OnStellate150();
	afx_msg void OnStellate200();
	afx_msg void OnStellateOther();
	afx_msg void OnPreferences();
	afx_msg void OnSetFaceimage();
	afx_msg void OnGeofractalStellate();
	afx_msg void OnFractalStellate();
	afx_msg void OnUpdateGeofractalStellate(CCmdUI* pCmdUI);
	afx_msg void OnStellateFace();
	afx_msg void OnViewNextFacegroup();
	afx_msg void OnUpdateViewNextFacegroup(CCmdUI* pCmdUI);
	afx_msg void OnViewNextPage();
	afx_msg void OnUpdateViewNextPage(CCmdUI* pCmdUI);
	afx_msg void OnViewPrevFaceGroup();
	afx_msg void OnUpdateViewPrevFaceGroup(CCmdUI* pCmdUI);
	afx_msg void OnViewPrevPage();
	afx_msg void OnUpdateViewPrevPage(CCmdUI* pCmdUI);
	//}}AFX_MSG
	afx_msg void OnRemoveGroup( UINT nID );
	afx_msg void OnLayoutSwitch( UINT nID );
	afx_msg void OnJoinEdge( UINT nID );
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnShapeDictionary();
	afx_msg void OnKstellate25();
	afx_msg void OnKstellateOther();
	afx_msg void OnKstellate100();
	afx_msg void OnKstellate50();
	afx_msg void OnKstellate200();
	afx_msg void OnEditClip();
	afx_msg void OnKstellate60();
	afx_msg void OnStellate60();
	afx_msg void OnUpdateStellate200(CCmdUI *pCmdUI);
	afx_msg void OnUpdateStellate150(CCmdUI *pCmdUI);
	afx_msg void OnUpdateStellate125(CCmdUI *pCmdUI);
	afx_msg void OnUpdateStellate100(CCmdUI *pCmdUI);
	afx_msg void OnUpdateStellate75(CCmdUI *pCmdUI);
	afx_msg void OnUpdateStellate60(CCmdUI *pCmdUI);
	afx_msg void OnUpdateStellateOther(CCmdUI *pCmdUI);
	afx_msg void OnUpdateKstellate200(CCmdUI *pCmdUI);
	afx_msg void OnUpdateKstellate100(CCmdUI *pCmdUI);
	afx_msg void OnUpdateKstellate60(CCmdUI *pCmdUI);
	afx_msg void OnUpdateKstellateOther(CCmdUI *pCmdUI);
	afx_msg void OnShapeMediamanager();
	afx_msg void OnMenuSetimagecaption();
	afx_msg void OnLayoutDefault();
	afx_msg void OnLayoutSplit();
	afx_msg void OnUpdateLayoutMenu();
	afx_msg void OnUpdateLayoutDefault(CCmdUI *pCmdUI);
	afx_msg void OnUpdateLayoutUser(CCmdUI *pCmdUI);
	afx_msg void OnFileSaveDefinition();
	afx_msg void OnSetBaseEdge();
	afx_msg void OnAddTriangle();
	afx_msg void OnAddSquare();
	afx_msg void OnAddPentagon();
	afx_msg void OnAddHexagon();
	afx_msg void OnAddOctagon();
	afx_msg void OnAddDecagon();
	afx_msg void OnDetachEdge();
	afx_msg void OnUpdateOutdentFace(CCmdUI *pCmdUI);
	afx_msg void OnOutdentFace();
	afx_msg void OnEdgeForceSplit();
};

#ifndef _DEBUG  // debug version in PaperCutView.cpp
inline CPaperCutDoc* CPaperCutView::GetDocument()
   { return (CPaperCutDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PAPERCUTVIEW_H__9837ABB3_3851_11D6_A858_0040F4309CCE__INCLUDED_)

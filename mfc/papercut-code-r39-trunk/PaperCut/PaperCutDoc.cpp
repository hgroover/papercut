/*!

	@file	 PaperCutDoc.cpp
	
	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: PaperCutDoc.cpp 9 2006-03-08 14:41:10Z henry_groover $

  The document containing the actual CShape and serialization wrapper functionality.

*/

#include "stdafx.h"
#include "PaperCut.h"

#include "PaperCutDoc.h"
#include "PaperCutView.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern char szBuildVer[];
extern int nBuildNumber;
extern time_t BuildTimestamp;

/////////////////////////////////////////////////////////////////////////////
// CPaperCutDoc

IMPLEMENT_DYNCREATE(CPaperCutDoc, CDocument)

BEGIN_MESSAGE_MAP(CPaperCutDoc, CDocument)
	//{{AFX_MSG_MAP(CPaperCutDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CPaperCutDoc, CDocument)
	//{{AFX_DISPATCH_MAP(CPaperCutDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//      DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// Note: we add support for IID_IPaperCut to support typesafe binding
//  from VBA.  This IID must match the GUID that is attached to the 
//  dispinterface in the .ODL file.

// {9837ABA5-3851-11D6-A858-0040F4309CCE}
static const IID IID_IPaperCut =
{ 0x9837aba5, 0x3851, 0x11d6, { 0xa8, 0x58, 0x0, 0x40, 0xf4, 0x30, 0x9c, 0xce } };

BEGIN_INTERFACE_MAP(CPaperCutDoc, CDocument)
	INTERFACE_PART(CPaperCutDoc, IID_IPaperCut, Dispatch)
END_INTERFACE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPaperCutDoc construction/destruction

CPaperCutDoc::CPaperCutDoc()
{
	m_pShapeLayout = NULL;

	EnableAutomation();

	AfxOleLockApp();
}

CPaperCutDoc::~CPaperCutDoc()
{
	AfxOleUnlockApp();
	if (m_pShapeLayout)
	{
		delete m_pShapeLayout;
		m_pShapeLayout = NULL;
	}
}

BOOL CPaperCutDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// Create basic shape
	m_Shape.SetupShape( "Cube" );
	m_Shape.SetName( "Photocube" );
	SetTitle( "Photocube - drag and drop pictures to get started" );
	UpdateLayout( m_Shape.GetLayoutDefaultWidth(), m_Shape.GetLayoutDefaultWidthOverHeight() );

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CPaperCutDoc serialization

// Read file into an array. Return elements read or -1 if error
int CPaperCutDoc::ReadFileIntoArray( CFile* pf, CArray<CString,LPCTSTR>& a )
{
	// Parse file, stripping leading whitespace and ignoring
	// comments and blank lines.
	ULONGLONG uSize = pf->SeekToEnd();
	pf->SeekToBegin();
	CString szFile;
	char *pszBuff = szFile.GetBuffer( (size_t)uSize );
	if (pszBuff == NULL)
	{
		AfxMessageBox( "Error: memory allocation for file read failed.\n" );
		return -1;
	}
	UINT uRead;
	do {
		uRead = pf->Read( pszBuff, (size_t)uSize );
		if (uRead == uSize)
		{
			szFile.ReleaseBuffer( uRead );
			break;
		} // Got entire file
		szFile.Format( "Error: failed to read %dK bytes from file!\n",
			uSize / 1024 );
		AfxMessageBox( szFile );
		return -1;
	} while (uRead);
	// Use strtok to parse lines
	char *pszTok;
	for (pszTok = strtok( pszBuff, "\r\n\x1a" ); pszTok; pszTok = strtok( NULL, "\r\n\x1a" ))
	{
		// Skip leading whitespace
		pszTok += strspn( pszTok, " \t" );
		// Leave trailing whitespace alone
		// Ignore comments and blank lines
		if (*pszTok == '\0' || *pszTok == '#')
		{
			continue;
		}
		// Add to array
		a.Add( pszTok );
	}
	return a.GetSize();
}

void CPaperCutDoc::Serialize(CArchive& ar)
{
	// Use absolute path as basis of SaveTransformPath() and LoadTransformPath()
	m_Shape.SetTransformBaseDir( ar.GetFile()->GetFilePath() );
	if (ar.IsStoring())
	{
		CArray<CString,const char*> aShape;
		CFile* pf = ar.GetFile();
		CString sz;
		char szTime[ 128 ];
		time_t now = ::time( NULL );
		struct tm *plt = localtime( &now );
		::strftime( szTime, sizeof( szTime ), "%c", plt );
		sz.Format( "# Written by PaperCut v%s.%u\n", szBuildVer, nBuildNumber );
		sz += "# Saved at ";
		sz += szTime;
		//sz += "\n#\n# ";
		//sz += 
		// Save path in shape
		m_Shape.m_SavedPath = ar.m_strFileName;
		m_Shape.SaveSymbolic( aShape );
		sz += "\n#\n";
		pf->Write( (const char*)sz, sz.GetLength() );
		//aShape.Serialize( ar );
		// Save layout attributes
		sz = m_pShapeLayout->SaveSymbolic();
		pf->Write( sz, sz.GetLength() );
		int n;
		static char *pszIndent = "                ";
		int nMaxIndent = 8;
		int nIndent = 0;
		for (n = 0; n < aShape.GetSize(); n++)
		{
			if (aShape[n] == "}")
			{
				if (nIndent > 0) nIndent--;
			}
			pf->Write( pszIndent, nIndent * 2 );
			pf->Write( (const char*)aShape[n], aShape[n].GetLength() );
			pf->Write( "\n", 1 );
			if (aShape[n] == "{")
			{
				if (nIndent < nMaxIndent) nIndent++;
			}
		} // for all
		pf->Write( "\n", 1 );
		// Clear modified flag
		SetModifiedFlag( FALSE );
	}
	else
	{
		CArray<CString,const char*> aShape;
		CFile* pf = ar.GetFile();
		// Read file into array. Return elements read or -1 if error
		int nLinesRead = ReadFileIntoArray( pf, aShape );
		if (nLinesRead <= 0)
		{
			AfxMessageBox( "Error: unable to read from file" );
			AfxAbort();
		}
		CString szLayout;
		bool bInLayout = false;
		int nStartDelete = -1, nEndDelete = -1;
		for (int n = 0; n < aShape.GetSize(); n++)
		{
			//aShape.Add( sz );
			if (bInLayout)
			{
				szLayout += aShape[n];
				szLayout += '\n';
				if (aShape[n] == "}")
				{
					bInLayout = false;
					nEndDelete = n;
				}
			} // Processing layout
			else
			{
				if (aShape[n].CompareNoCase( "layout" ) == 0)
				{
					bInLayout = true;
					szLayout = aShape[n];
					szLayout += '\n';
					nStartDelete = n;
				}
			} // Processing shape
		} // for all
		// Remove layout
		if (nStartDelete >= 0 && nEndDelete >= nStartDelete)
		{
			aShape.RemoveAt( nStartDelete, nEndDelete - nStartDelete + 1 );
		}
		// Create shape in two stages
		m_Shape.CreateFromSymbolic1( aShape );
		m_Shape.FixupSymbolic();
		m_Shape.MapVertices();
		// Make sure all divisions used in faces occur in shape
		for (POSITION posFace = m_Shape.m_mapFaces.GetStartPosition(); posFace != NULL; )
		{
			CFace* pFace;
			CString szFaceName;
			m_Shape.m_mapFaces.GetNextAssoc( posFace, szFaceName, pFace );
			// Layout divisions have been set directly as data but need to go through the
			// mutator function, which will add them to the shape.
			pFace->SetLayoutDivision( pFace->GetLayoutDivision() );
		}
		// Save path in shape
		m_Shape.m_SavedPath = ar.m_strFileName;
		double dPageWidthInLogicalUnits, dPageWidthOverHeight;
		CShapeLayout::RestoreSymbolic( szLayout, dPageWidthInLogicalUnits, dPageWidthOverHeight );
		UpdateLayout( dPageWidthInLogicalUnits, dPageWidthOverHeight );
		// Recreate layout
		POSITION posView = GetFirstViewPosition();
		CPaperCutView* pView = (CPaperCutView*)GetNextView( posView );
		if (pView)
		{
			UpdateAllViews( pView );
			// Update layout divisions menu by posting a message
			pView->PostMessage( WM_COMMAND, ID_UPDATE_LAYOUT_MENU );
		} // Update all views
		// Clear modified flag
		SetModifiedFlag( FALSE );
	}
}

/////////////////////////////////////////////////////////////////////////////
// CPaperCutDoc diagnostics

#ifdef _DEBUG
void CPaperCutDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CPaperCutDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CPaperCutDoc commands

// Return number of face groups or 0 if no shape
int CPaperCutDoc::UpdateLayout( double dPageWidthInLogicalUnits, double dPageWidthOverHeight )
{
	int nReturn = 0;
	if (m_pShapeLayout)
	{
		delete m_pShapeLayout;
		m_pShapeLayout = NULL;
	}
	if (m_Shape.IsValid())
	{
		CMainFrame* pMain = (CMainFrame*)::AfxGetMainWnd();
		// Turn off redraw
		m_Shape.SetValid( false );
		// Rebuild join list
		m_Shape.BuildJoinList();
		CDbg::Out( "\n\nCreating layout for shape\n" );
		m_pShapeLayout = new CShapeLayout( &m_Shape );
		// Set initial max shapes / page
		int nInitialPages = __max( 1, pMain->GetMaxPage() );
		m_pShapeLayout->m_dPageWidthInLogicalUnits = dPageWidthInLogicalUnits;
		m_pShapeLayout->m_dPageWidthOverHeight = dPageWidthOverHeight;
		nReturn = m_pShapeLayout->AutoJoin( nInitialPages );
		CDbg::Out( "Created %d face groups\n", nReturn );
		// Calculate bounding rectangle in face units
		double dHeight, dWidth, dMinX, dMinY;
		CDbg::Out( "Calculating bounding rectangles\n" );
		m_pShapeLayout->CalculateAllBoundingRectangles();
		CDbg::Out( "Getting bounding rectangles\n" );
		m_pShapeLayout->GetBoundingRectangleFor( 0, dWidth, dHeight, dMinX, dMinY );
		CDbg::Out( "Got %f w %f h min x %f y %f\n", dWidth, dHeight, dMinX, dMinY );
		// Use shape name for title
		SetTitle( m_Shape.GetName() );
		// Enable redraw
		m_Shape.SetValid( true );
		//// This may trigger a redraw
		//pMain->SetMaxPage( nInitialPages );
	}
	return nReturn;
}

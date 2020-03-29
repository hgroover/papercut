/*!

	@file	 PaperCut.cpp
	
	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: PaperCut.cpp 12 2006-03-10 06:10:30Z henry_groover $

  PaperCut application MFC execution class. Some global functionality which requires
  access to files present at startup in the same directory as the exe are also
  encapsulated here (such as the papercut.shapedef access functions).

*/

#include "stdafx.h"
#include "PaperCut.h"

#include <direct.h>
#include <io.h>

#include "MainFrm.h"
#include "ChildFrm.h"
#include "PaperCutDoc.h"
#include "PaperCutView.h"
#include "AskShapeExtension.h"
#include "ShapeDef.h"

//#include "versiondef.h"
extern char szBuildVer[];
extern int nBuildNumber;
extern time_t BuildTimestamp;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPaperCutApp

BEGIN_MESSAGE_MAP(CPaperCutApp, CWinApp)
	//{{AFX_MSG_MAP(CPaperCutApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPaperCutApp construction

CPaperCutApp::CPaperCutApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CPaperCutApp object

CPaperCutApp theApp;

// This identifier was generated to be statistically unique for your app.
// You may change it if you prefer to choose a specific identifier.

// {9837ABA3-3851-11D6-A858-0040F4309CCE}
static const CLSID clsid =
{ 0x9837aba3, 0x3851, 0x11d6, { 0xa8, 0x58, 0x0, 0x40, 0xf4, 0x30, 0x9c, 0xce } };

/////////////////////////////////////////////////////////////////////////////
// CPaperCutApp initialization

BOOL CPaperCutApp::InitInstance()
{

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	//// Initialize PixieLib
	//PixieLibInit();

	// This will get free()'d later, so make sure it's a valid heap pointer
	CString szIniPath, szHelpBasePath;
	szIniPath = m_pszHelpFilePath;
	// Major kluge - get help path, strip off hlp at end
	szHelpBasePath = szIniPath.Left( szIniPath.GetLength() - 3 );
	szIniPath = szHelpBasePath + "ini";
	this->m_pszProfileName = strdup( szIniPath );

	// Get shape definitions
	int nShapedefsLoaded = 0;
	m_ShapeDefPath = szHelpBasePath + "shapedef";
	if (!_access( m_ShapeDefPath, 00 ))
	{
		nShapedefsLoaded = CShapeDef::LoadFromFile( m_ShapeDefPath, this->m_mapShapeDefs );
	}
	else
	{
		// Try current dir
		m_ShapeDefPath = "Papercut.shapedef";
		if (!_access( m_ShapeDefPath, 00 ))
		{
			nShapedefsLoaded = CShapeDef::LoadFromFile( m_ShapeDefPath, this->m_mapShapeDefs );
		}
		else
		{
			nShapedefsLoaded = 0;
		}
	}
	if (nShapedefsLoaded > 0)
	{
		CreateOrdinalList();
	}

	// If this is undefined, we should be able to get things saved in a local
	// .ini file... but why is that a bad thing?
	// We'll have entries saved in HKEY_CURRENT_USER\Software\HGSoft\PaperCut
	SetRegistryKey(_T("HGSoft"));

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	// Load global preferences
	m_prefs.Load();

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	// Here are the strings in IDR_PAPERCUT_TYPERESOURCE (all separated by newline):
	// 0: not used for MDI
	// 1: docName - number added for new documents
	// 2: fileNewName - name of document type displayed in File / New
	// 3: filterName - name of file type and filter in parens for file open
	// 4: filterExt - extension to use if none supplied
	// 5: registry type ID - symbolic ID for registry
	// 6: registry type name - friendly name for registry

	// Notes re: changing resource ID: the same ID is used for several
	// different resources, including the document menu.
	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(
		/*IDR_PAPERCUT_TYPERESOURCE*/IDR_PAPERCTYPE,
		RUNTIME_CLASS(CPaperCutDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CPaperCutView));
	AddDocTemplate(pDocTemplate);

	// Connect the COleTemplateServer to the document template.
	//  The COleTemplateServer creates new documents on behalf
	//  of requesting OLE containers by using information
	//  specified in the document template.
	m_server.ConnectTemplate(clsid, pDocTemplate, FALSE);

	// Register all OLE server factories as running.  This enables the
	//  OLE libraries to create objects from other applications.
	COleTemplateServer::RegisterAll();
		// Note: MDI applications register all server objects without regard
		//  to the /Embedding or /Automation on the command line.

	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;
	m_pMainWnd = pMainFrame;

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Check for .shp extension
	CAskShapeExtension dlgShape;
	LPTSTR lpBuff;
	//lpBuff = dlgShape.m_szAppPath.GetBuffer( 1024 );
	//::_getcwd( lpBuff, 1023 );
	//dlgShape.m_szAppPath.ReleaseBuffer();
	//dlgShape.m_szAppPath += '\\';
	//CString szExeName;
	//szExeName = m_pszExeName;
	dlgShape.m_szAppPath = m_pszHelpFilePath;
	//szExeName = m_pszExeName;
	// Major kluge - get help path, strip off hlp at end
	dlgShape.m_szAppPath = dlgShape.m_szAppPath.Left( dlgShape.m_szAppPath.GetLength() - 3 );
	dlgShape.m_szAppPath += "exe";
	lpBuff = dlgShape.m_szShortAppPath.GetBuffer( 1024 );
	::GetShortPathName( dlgShape.m_szAppPath, lpBuff, 1024 );
	dlgShape.m_szShortAppPath.ReleaseBuffer();
	if (dlgShape.ShouldAskToRegister())
	{
		dlgShape.DoModal();
	}

	// Check to see if launched as OLE server
	if (cmdInfo.m_bRunEmbedded || cmdInfo.m_bRunAutomated)
	{
		// Application was run with /Embedding or /Automation.  Don't show the
		//  main window in this case.
		return TRUE;
	}

	// When a server application is launched stand-alone, it is a good idea
	//  to update the system registry in case it has been damaged.
	m_server.UpdateRegistry(OAT_DISPATCH_OBJECT);
	COleObjectFactory::UpdateRegistryAll();

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The main window has been initialized, so show and update it.
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

	return TRUE;
}

CString CPaperCutApp::GetAppVersion() const
{
	CString sz;
	sz.Format( "%s.%d", szBuildVer, nBuildNumber );
	return sz;
}

//time_t CPaperCutApp::GetAppVersionDate() const
//{
//	return BuildTimeStamp;
//}

CString CPaperCutApp::GetAppVersionDate() const
{
	char szBuildTime[ 256 ];
	strftime( szBuildTime, sizeof( szBuildTime ), "%x %X", localtime( &BuildTimestamp ) );
	return szBuildTime;
}

// Initialize shape from definitions or return false if not found
bool CPaperCutApp::InitShape( CShape& shp, LPCTSTR ShapeName )
{
	return CShapeDef::CreateShape( shp, ShapeName, this->m_mapShapeDefs );
}

// Release shape definitions
void CPaperCutApp::ReleaseShapeDefs()
{
	CString szName;
	CShapeDef* pDef;
	POSITION pos;
	for (pos = m_mapShapeDefs.GetStartPosition(); pos != NULL; )
	{
		m_mapShapeDefs.GetNextAssoc( pos, szName, pDef );
		delete pDef;
		m_mapShapeDefs[szName] = NULL;
	}
	m_mapShapeDefs.RemoveAll();
	this->m_aShapeNames.RemoveAll();

	// Remove submenus - this should also delete the associated UI artifacts
	CMenu* pMenu;
	for (pos = m_mapSubmenus.GetStartPosition(); pos != NULL; )
	{
		m_mapSubmenus.GetNextAssoc( pos, szName, pMenu );
		if (pMenu != NULL)
		{
			pMenu->DestroyMenu();
			delete pMenu;
			pMenu = NULL;
			m_mapSubmenus[szName] = pMenu;
		}
	}
	m_mapSubmenus.RemoveAll();
}

// Re-read shape definitions. This is not usable without a way to remove menu items from menus.
void CPaperCutApp::ReloadShapedefs()
{
	ReleaseShapeDefs();
	CShapeDef::LoadFromFile( this->m_ShapeDefPath, m_mapShapeDefs );
}

// Given an origin:0 ordinal, return the shape name
CString CPaperCutApp::GetShapeByOrdinal( int MenuOrdinal )
{
	if (MenuOrdinal < 0 || MenuOrdinal >= this->m_aShapeNames.GetSize())
	{
		return "";
	}
	return m_aShapeNames[MenuOrdinal];
}

// Create ordinal list of shape names
void CPaperCutApp::CreateOrdinalList()
{
	m_aShapeNames.RemoveAll();
	CString szName;
	CShapeDef* pDef;
	for (POSITION pos = m_mapShapeDefs.GetStartPosition(); pos != NULL; )
	{
		m_mapShapeDefs.GetNextAssoc( pos, szName, pDef );
		m_aShapeNames.Add( szName );
	}
}

// Create or find containing menu
CMenu* CPaperCutApp::FindOrCreateContainingMenu( CMenu* pMenu, CArray<CString,LPCTSTR>& aMenuNames )
{
	// Search all but last entry
	int ContainerCount = aMenuNames.GetSize() - 1;
	// Create menu path as we go along, delimited by "/"
	CString szMenuSeparator;
	CString szMenuPath;
	for (int Menu = 0; Menu < ContainerCount; Menu++)
	{
		UINT MenuCount = pMenu->GetMenuItemCount();
		CString MenuString, Target;
		Target = aMenuNames[Menu];
		szMenuPath += szMenuSeparator;
		szMenuPath += Target;
		szMenuSeparator = "/";
		bool bFound = false;
		UINT MenuItem;
		for (MenuItem = 0; MenuItem < MenuCount && !bFound; MenuItem++)
		{
			pMenu->GetMenuString( MenuItem, MenuString, MF_BYPOSITION );
			bFound = (MenuString == Target);
			if (bFound)
			{
				pMenu = pMenu->GetSubMenu( MenuItem );
			}
		}
		if (!bFound)
		{
			// See if it already exists
			CMenu *pSubMenu;
			bFound = m_mapSubmenus.Lookup( szMenuPath, pSubMenu );
			if (!bFound)
			{
				pSubMenu = new CMenu();
				pSubMenu->CreatePopupMenu();
				m_mapSubmenus.SetAt( szMenuPath, pSubMenu );
			}
			pMenu->AppendMenu( MF_POPUP, (UINT)pSubMenu->m_hMenu, Target );
			pMenu = pMenu->GetSubMenu( MenuItem );
		}
	}
	return pMenu;
}

// Put shape defs in menu
void CPaperCutApp::AddShapedefsToMenu( CMenu* pMenu, unsigned int CommandIDBase )
{
	// Find Shape / Create menu
	CMenu* pShapeCreate = NULL;
	UINT MenuCount = pMenu->GetMenuItemCount();
	UINT MenuItem;
	int ShapeMenuIndex = -1;
	int ShapeCreateMenuIndex = -1;
	CMenu* pCurMenu = pMenu;
	for (MenuItem = 0; MenuItem < MenuCount; MenuItem++)
	{
		// Get string with & stripped out
		CString MenuString;
		pCurMenu->GetMenuString( MenuItem, MenuString, MF_BYPOSITION );
		if (MenuString == "&Shape")
		{
			pCurMenu = pCurMenu->GetSubMenu( MenuItem );
			MenuCount = pCurMenu->GetMenuItemCount();
			ShapeMenuIndex = MenuItem;
			MenuItem = 0;
			pCurMenu->GetMenuString( MenuItem, MenuString, MF_BYPOSITION );
		}
		if (ShapeMenuIndex >= 0 && MenuString == "&Create")
		{
			pShapeCreate = pCurMenu = pCurMenu->GetSubMenu( MenuItem );
			ShapeCreateMenuIndex = MenuItem;
			break;
		}
	}
	if (pShapeCreate == NULL)
	{
		AfxMessageBox( "Error: unable to get Shape / Create menu" );
		return;
	}
	for (int n = 0; n < this->m_aShapeNames.GetSize(); n++)
	{
		CShapeDef* pDef;
		if (this->m_mapShapeDefs.Lookup( m_aShapeNames[n], pDef ))
		{
			// Find hierarchy or create it
			pCurMenu = FindOrCreateContainingMenu( pShapeCreate, pDef->m_aMenuTags );
			if (pCurMenu != NULL)
			{
				// Check for this command
				MENUITEMINFO ItemInfo;
				memset( &ItemInfo, 0, sizeof(ItemInfo) );
				ItemInfo.cbSize = sizeof( ItemInfo );
				if (!pCurMenu->GetMenuItemInfo( CommandIDBase + n, &ItemInfo ))
				{
					pCurMenu->AppendMenu( MF_BYCOMMAND, CommandIDBase + n, pDef->m_aMenuTags[pDef->m_aMenuTags.GetUpperBound()] );
					//pCurMenu = pCurMenu->GetMenuIte
				}
			}
		}
		else
		{
			CString szMsg;
			szMsg.Format( "Error: could not access shape %s", (LPCTSTR)m_aShapeNames[n] );
			AfxMessageBox( szMsg );
		}
	}
}

// Get shape definition record from origin:0 ordinal
CShapeDef* CPaperCutApp::GetShapeDef( int Ordinal )
{
	CString szName = GetShapeByOrdinal( Ordinal );
	if (szName.IsEmpty())
	{
		return NULL;
	}
	CShapeDef* pDef;
	if (!this->m_mapShapeDefs.Lookup( szName, pDef ))
	{
		return NULL;
	}
	return pDef;
}


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnKstellate60();
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CPaperCutApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CPaperCutApp message handlers


CString
FormatTimeDuration( time_t u )
{
	CString sz;
	// Check for years
	static time_t secsperyear = 36525 * 864;
	int nYears = (int)(u / secsperyear);
	u -= (nYears * secsperyear);
	static time_t secsperweek = 7 * 86400;
	int nWeeks = (int)(u / secsperweek);
	u -= (nWeeks * secsperweek);
	int nDays = (int)(u / 86400);
	u -= (nDays * 86400);
	int nHours = (int)(u / 3600);
	u -= (nHours * 3600);
	int nMinutes = (int)(u / 60);
	u -= (nMinutes * 60);
	int nSeconds = (int)u;
	static char *aszNames[] = {
		" years ",
		" weeks ",
		" days ",
		"h ",
		"m ",
		" seconds"
	};
	int anTimes[6] = {
		nYears,
		nWeeks,
		nDays,
		nHours,
		nMinutes,
		nSeconds
	};
#define NUMTIMES (sizeof(anTimes)/sizeof(anTimes[0]))
	int nValid;
	for (nValid = 0; nValid < NUMTIMES - 1; nValid++)
	{
		if (anTimes[nValid] > 0) break;
	}
	char szBuff[128];
	sz.Empty();
	for ( ; nValid < NUMTIMES; nValid++)
	{
		sprintf( szBuff, "%u%s", anTimes[nValid], aszNames[nValid] );
		sz += szBuff;
	}
	return sz;
} // FormatTimeDuration()

BOOL CAboutDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	time_t Beginning = 1016252400; // 15Mar02 2020 PST
	time_t DOB_add = ((9 * 12 + 1) * 30 + 21 + 50) * 86400 - 4 * 3600 - 50 * 60; // seconds from 07Nov1960 2050 EST
	CWnd* pWnd = GetDlgItem( IDC_VERSIONBOX );
	CString szVersion;
	time_t now = time( NULL );
	time_t SinceBeginning = now - Beginning;
	time_t SinceBuild = now - BuildTimestamp;
	char szBuildTime[ 256 ];
	strftime( szBuildTime, sizeof( szBuildTime ), "%x %X", localtime( &BuildTimestamp ) );
	char szStartTime[ 256 ];
	strftime( szStartTime, sizeof( szStartTime ), "%x %X", localtime( &Beginning ) );
	//time_t birthday40 = (36525 * 864 * 40) - DOB_add;
	//char szBirthday40[ 256 ];
	//strftime( szBirthday40, sizeof( szBirthday40 ), "%x %X", localtime( &birthday40 ) );
	szVersion.Format(
		"Version %s build %u created %s\r\n"
			"This version is %s old.\r\n"
		"PaperCut development was started on %s, which is %s ago.\r\n",
//			"The creator of this software is now %s old.\r\n",
		szBuildVer, nBuildNumber, szBuildTime,
			(LPCTSTR)FormatTimeDuration( SinceBuild ),
		szStartTime, (LPCTSTR)FormatTimeDuration( SinceBeginning )
//			,	(LPCTSTR)FormatTimeDuration( DOB_add + now ) 
			);
	pWnd->SetWindowText( szVersion );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


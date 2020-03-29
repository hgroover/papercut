/*!

	@file	AskShapeExtension.cpp
	
	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: AskShapeExtension.cpp 9 2006-03-08 14:41:10Z henry_groover $
  AskShapeExtension.cpp : implementation file
*/

#include "stdafx.h"
#include "PaperCut.h"
#include "AskShapeExtension.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAskShapeExtension dialog


CAskShapeExtension::CAskShapeExtension(CWnd* pParent /*=NULL*/)
	: CDialog(CAskShapeExtension::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAskShapeExtension)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CAskShapeExtension::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAskShapeExtension)
	DDX_Control(pDX, IDC_ALWAYSCHECK, m_chkAlways);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAskShapeExtension, CDialog)
	//{{AFX_MSG_MAP(CAskShapeExtension)
	ON_BN_CLICKED(IDOK, OnYes)
	ON_BN_CLICKED(IDCANCEL, OnNo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAskShapeExtension message handlers

void CAskShapeExtension::OnYes() 
{
	// Yes, do it...
	HKEY hkShp;
	DWORD dwDisposition;

	// Step 1: create HKEY_CLASSES_ROOT\\.shp with default value of our doc name
	if (ERROR_SUCCESS != ::RegCreateKeyEx( HKEY_CLASSES_ROOT,
		".shp",
		0,
		NULL,		// No classes
		REG_OPTION_NON_VOLATILE,
		KEY_WRITE,
		NULL,		// No security attributes
		&hkShp,
		&dwDisposition ))
	{
		AfxMessageBox( "Error: unable to create registry key under HKEY_CLASSES_ROOT" );
		EndDialog( IDCANCEL );
		return;
	} // Try creating

	// Set to our PaperCut.document type
	CString szDoc;
	szDoc = "PaperCut.document";
	LONG lReturn = ::RegSetValueEx( hkShp,
		NULL, // Default
		0,
		REG_SZ,
		(LPBYTE)(LPCTSTR)szDoc,
		szDoc.GetLength() );

	// Close key
	::RegCloseKey( hkShp );

	if (lReturn != ERROR_SUCCESS)
	{
		AfxMessageBox( "Error: unable to set value under HKEY_CLASSES_ROOT\\.shp" );
		EndDialog( IDCANCEL );
		return;
	}

	// Step 2: create DefaultIcon entry under HKEY_CLASSES_ROOT\\PaperCut.document
	if (ERROR_SUCCESS != ::RegCreateKeyEx( HKEY_CLASSES_ROOT,
		"PaperCut.document\\DefaultIcon",
		0,
		NULL,		// No classes
		REG_OPTION_NON_VOLATILE,
		KEY_WRITE,
		NULL,		// No security attributes
		&hkShp,
		&dwDisposition ))
	{
		AfxMessageBox( "Error: unable to create doc default icon key under HKEY_CLASSES_ROOT" );
		EndDialog( IDCANCEL );
		return;
	} // Try creating

	szDoc.Format( "%s,0", (LPCTSTR)m_szShortAppPath );
	lReturn = ::RegSetValueEx( hkShp,
		NULL, // Default
		0,
		REG_SZ,
		(LPBYTE)(LPCTSTR)szDoc,
		szDoc.GetLength() );

	// Close key
	::RegCloseKey( hkShp );

	if (lReturn != ERROR_SUCCESS)
	{
		AfxMessageBox( "Error: unable to set value under HKEY_CLASSES_ROOT\\PaperCut.document\\DefaultIcon" );
		EndDialog( IDCANCEL );
		return;
	}


	// Step 3: Create shell\\open\\command

	// Now create under HKEY_CLASSES_ROOT\\PaperCut.document
	if (ERROR_SUCCESS != ::RegCreateKeyEx( HKEY_CLASSES_ROOT,
		"PaperCut.document\\shell\\open\\command",
		0,
		NULL,		// No classes
		REG_OPTION_NON_VOLATILE,
		KEY_WRITE,
		NULL,		// No security attributes
		&hkShp,
		&dwDisposition ))
	{
		AfxMessageBox( "Error: unable to create doc registry key under HKEY_CLASSES_ROOT" );
		EndDialog( IDCANCEL );
		return;
	} // Try creating

	szDoc.Format( "\"%s\" \"%%1\"", (LPCTSTR)m_szAppPath );
	lReturn = ::RegSetValueEx( hkShp,
		NULL, // Default
		0,
		REG_SZ,
		(LPBYTE)(LPCTSTR)szDoc,
		szDoc.GetLength() );

	// Close key
	::RegCloseKey( hkShp );

	if (lReturn != ERROR_SUCCESS)
	{
		AfxMessageBox( "Error: unable to set value under HKEY_CLASSES_ROOT\\PaperCut.document" );
		EndDialog( IDCANCEL );
		return;
	}

	CWinApp* pMyApp = AfxGetApp();
	// If checkbox is cleared, don't check again
	if (m_chkAlways.GetCheck() == 0)
	{
		pMyApp->WriteProfileInt( "Startup", "CheckFileExtension", 0 );
	} // Checkbox is clear

	// And we're done
	EndDialog( IDYES );
}

void CAskShapeExtension::OnNo() 
{
	// No, don't... but check the checkbox, and if empty, save
	// in registry
	if (m_chkAlways.GetCheck() == 0)
	{
		CWinApp* pMyApp = AfxGetApp();
		pMyApp->WriteProfileInt( "Startup", "CheckFileExtension", 0 );
	} // Checkbox is clear

	EndDialog( IDNO );
}

BOOL CAskShapeExtension::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Set the checkbox
	UpdateData( FALSE );
	m_chkAlways.SetCheck( 1 );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

bool CAskShapeExtension::IsExtensionRegistered()
{
	// Try to open key
	HKEY hkShape;
	if (ERROR_SUCCESS !=
		::RegOpenKeyEx( HKEY_CLASSES_ROOT,
			".shp",
			0,
			KEY_READ,
			&hkShape ))
			return false;

	bool bReturn = false;

	// Try to get value
	unsigned char szBuff[1024];
	DWORD dwType = 0;
	DWORD dwLen = sizeof( szBuff ) - 1;
	if (ERROR_SUCCESS ==
		::RegQueryValueEx( hkShape,
		NULL,	//  Default value
		NULL,	// Reserved
		&dwType,
		szBuff,
		&dwLen ))
	{
		szBuff[ dwLen ] = '\0';
		bReturn = (!strcmp( (LPCTSTR)(&szBuff[0]), "PaperCut.document" ));
	} // Got value

	::RegCloseKey( hkShape );

	// Wait, there's more...
	if (!bReturn) return false;
	if (ERROR_SUCCESS !=
		::RegOpenKeyEx( HKEY_CLASSES_ROOT,
			"PaperCut.document\\shell\\open\\command",
			0,
			KEY_READ,
			&hkShape ))
	{
		return false;
	} // Failed to open document key

	// Default should be our path
	dwLen = sizeof( szBuff ) - 1;
	bReturn = (ERROR_SUCCESS ==
		::RegQueryValueEx( hkShape,
			NULL,
			NULL, // Reserved
			&dwType,
			szBuff,
			&dwLen ));

	if (bReturn)
	{
		char *psz = (char*)&szBuff[0];
		psz += strspn( psz, "\"" );
		m_szInstalledPath = psz;
		m_szInstalledPath = m_szInstalledPath.SpanExcluding( "\"" );
		bReturn = (stricmp( m_szAppPath, m_szInstalledPath ) == 0);
	} // Got value

	::RegCloseKey( hkShape );

	return bReturn;
}

bool CAskShapeExtension::ShouldAskToRegister()
{
	CWinApp* pMyApp = AfxGetApp();
	int nCheck = pMyApp->GetProfileInt( "Startup", "CheckFileExtension", 1 );
	if (!nCheck) return false;
	return !IsExtensionRegistered();
}

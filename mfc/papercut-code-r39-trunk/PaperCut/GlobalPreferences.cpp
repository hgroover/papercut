/*!

	@file	 GlobalPreferences.cpp

	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: GlobalPreferences.cpp 16 2006-04-15 06:39:12Z henry_groover $

	Preferences set. This class exposes all possible preferences and encapsulates
	serialization of preferences (by multiple inheritance). It handles resolution of 
	multiple levels of overrides, allowing global preferences to be overridden by shape 
	preferences (or other lower levels which may be implemented in future).

*/

#include "stdafx.h"
#include "PaperCut.h"
#include "GlobalPreferences.h"
#include "Face.h"
#include "FaceContent.h"
#include "Shape.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGlobalPreferences::CGlobalPreferences()
{
	Reset();
	// Set up predefined callback
	m_mapPredefs.RemoveAll();
	m_mapPredefs.SetAt( "__date__", CGlobalPreferences::PDVDate );
	m_mapPredefs.SetAt( "__time__", CGlobalPreferences::PDVTime );
	m_mapPredefs.SetAt( "__face__", CGlobalPreferences::PDVFace );
	m_mapPredefs.SetAt( "__group__", CGlobalPreferences::PDVGroup );
	m_mapPredefs.SetAt( "__contentpath__", CGlobalPreferences::PDVContentPath );
	m_mapPredefs.SetAt( "__contentfile__", CGlobalPreferences::PDVContentFile );
	m_mapPredefs.SetAt( "__font__", CGlobalPreferences::PDVFont );
	m_mapPredefs.SetAt( "__page__", CGlobalPreferences::PDVPageNo );
	m_mapPredefs.SetAt( "__shape__", CGlobalPreferences::PDVShapeName );
	m_mapPredefs.SetAt( "__nfaces__", CGlobalPreferences::PDVShapeNumFaces );
	m_mapPredefs.SetAt( "__shapepath__", CGlobalPreferences::PDVShapePath );
	// Set up predefined static values
	m_mapGlobalDict.SetAt( "__ver__", MYAPP()->GetAppVersion() );
	m_mapGlobalDict.SetAt( "__verdate__", MYAPP()->GetAppVersionDate() );
}

CGlobalPreferences::~CGlobalPreferences()
{

}

// New values need to be represented in the following:
//	CGlobalPreferences::Save()
//	CGlobalPreferences::Load()
//	CPreferenceSet::Reset()
//	CPreferenceSet::CopyFrom()
//	CPreferenceSet::GetSymbolicForm()
//	CPreferenceSet::ParseSymbolicForm()
int
CGlobalPreferences::Save()
{
	CPaperCutApp* pApp = MYAPP();
	CString sz;
	LPCTSTR lpSection = "Drawing";
#define	CVTWRITE(fmt,varbase,var)	sz.Format( fmt, m_##varbase##var ); pApp->WriteProfileString( lpSection, #var, sz )
#define	STRWRITE(var)	pApp->WriteProfileString( lpSection, #var, m_sz##var )

	CVTWRITE( "%u", n, ScaleMethod );
	CVTWRITE( "%u", n, PictureFlow );
	CVTWRITE( "%u", n, FontPointsize );
	CVTWRITE( "%u", n, EnableRotation );
	CVTWRITE( "%u", n, UseTabs );
	CVTWRITE( "%u", n, LayoutFit );
	CVTWRITE( "%lx", u, FontAttributes );
	CVTWRITE( "%08x", clr, EdgeColor );
	CVTWRITE( "%08x", clr, FaceColor );
	CVTWRITE( "%08x", clr, TabColor );
	CVTWRITE( "%08x", clr, FaceBgColor );
	CVTWRITE( "%08x", clr, TabBgColor );
	CVTWRITE( "%08x", clr, InwdEdgeColor );
	CVTWRITE( "%lf", d, TabShoulderAngle );
	CVTWRITE( "%lf", d, MinFitAngle );
	CVTWRITE( "%lf", d, AreaRatio );
	CVTWRITE( "%lf", d, HeightRatio );
	CVTWRITE( "%lf", d, BaseRatio );
	CVTWRITE( "%lf", d, OrgRatio );
	STRWRITE( DefaultFaceText );
	STRWRITE( FontTypeface );

	// Global dictionary
	lpSection = "Dictionary";
	CString szEntryName, szEntryValue;
	POSITION pos;
	for (pos = this->m_mapGlobalDict.GetStartPosition(); pos != NULL; )
	{
		m_mapGlobalDict.GetNextAssoc( pos, szEntryName, szEntryValue );
		pApp->WriteProfileString( lpSection, szEntryName, szEntryValue );
	}

	return 0;
} // CGlobalPreferences::Save()

int
CGlobalPreferences::Load()
{
	CPaperCutApp* pApp = MYAPP();
	CString sz;
	LPCTSTR lpSection = "Drawing";

#define	CVTREAD(fmt,varbase,var,defvalue) \
		sz = pApp->GetProfileString( lpSection, #var, defvalue ); \
		sscanf( sz, fmt, &m_##varbase##var )
#define STRREAD(var,defvalue) \
		m_sz##var = pApp->GetProfileString( lpSection, #var, defvalue )

	CString szUTB_FULL;
	szUTB_FULL.Format( "%u", UTB_FULL );
	CString szLFIT_FULL;
	szLFIT_FULL.Format( "%u", LFIT_FULL );

	CVTREAD( "%u", n, ScaleMethod, "0" );
	CVTREAD( "%u", n, PictureFlow, "1" );
	CVTREAD( "%u", n, FontPointsize, "12" );
	CVTREAD( "%u", n, EnableRotation, "0" );
	CVTREAD( "%u", n, UseTabs, szUTB_FULL );
	CVTREAD( "%u", n, LayoutFit, szLFIT_FULL );
	CVTREAD( "%lx", u, FontAttributes, "0190" );
	CVTREAD( "%x", clr, EdgeColor, "000000" );
	CVTREAD( "%x", clr, FaceColor, "000000" );
	CVTREAD( "%x", clr, TabColor, "000000" );
	CVTREAD( "%x", clr, FaceBgColor, "ffffff" );
	CVTREAD( "%x", clr, TabBgColor, "ffffff" );
	CVTREAD( "%x", clr, InwdEdgeColor, "000000" );
	CVTREAD( "%lf", d, TabShoulderAngle, "30" );
	CVTREAD( "%lf", d, MinFitAngle, "60" );
	CVTREAD( "%lf", d, AreaRatio, "0.75" );
	CVTREAD( "%lf", d, HeightRatio, "0.82" );
	CVTREAD( "%lf", d, BaseRatio, "0.88" );
	CVTREAD( "%lf", d, OrgRatio, "1.0" );
	STRREAD( DefaultFaceText, "" );
	STRREAD( FontTypeface, "Arial" );

	// Get global dictionary
	// Enumerate HKEY_LOCAL_USER\Software\HGSoft\PaperCut\Dictionary keys
	HKEY hk;
	lpSection = "Dictionary";
	// We have some static predefined values
	//this->m_mapGlobalDict.RemoveAll();
	if (::RegOpenKey( MYAPP()->GetAppRegistryKey(), lpSection, &hk ) == ERROR_SUCCESS)
	{
		DWORD dwEntryNum, dwEntrySize;
		TCHAR szKeyName[256];
		CString szKeyValue;
		for (dwEntryNum = 0; ; dwEntryNum++)
		{
			dwEntrySize = sizeof( szKeyName );
			if (::RegEnumValue( hk, dwEntryNum, szKeyName, &dwEntrySize, NULL, NULL, NULL, NULL ) != ERROR_SUCCESS)
				break;
			szKeyName[dwEntrySize] = '\0';
			szKeyValue = pApp->GetProfileString( lpSection, szKeyName, "" );
			this->m_mapGlobalDict.SetAt( szKeyName, szKeyValue );
		}
		::RegCloseKey( hk );
	}
	return 0;
} // CGlobalPreferences::Load()

CString 
CGlobalPreferences::PDVDate(unsigned int junk1, unsigned int junk2)
{
	CTime dt;
	dt = CTime::GetCurrentTime();
	return dt.Format("%x");
}

CString 
CGlobalPreferences::PDVTime(unsigned int junk1, unsigned int junk2)
{
	CTime dt;
	// formatting a la strftime
	dt = CTime::GetCurrentTime();
	return dt.Format("%X");
}

CString
CGlobalPreferences::PDVFace(unsigned int pFace, unsigned int junk)
{
	CFace* p = (CFace*)pFace;
	if (p != NULL)
		return p->m_szFaceName;
	else
		return "?err";
}

CString
CGlobalPreferences::PDVGroup(unsigned int pFace, unsigned int Index)
{
	CFace* p = (CFace*)pFace;
	CArray<CString,LPCTSTR> aGroups;
	if (p != NULL)
	{
		if (p->GetGroups( aGroups ) < 1)
			return "";
		// Check index
		if (Index >= aGroups.GetSize())
			return "";
		return aGroups[Index];
	}
	else
		return "?err";
}

CString 
CGlobalPreferences::PDVContentPath(unsigned int pFace, unsigned int junk)
{
	CFace* p = (CFace*)pFace;
	CFaceContent* pfc = p->m_pContent;
	if (pfc == NULL) return "";
	return pfc->GetPath();
}

CString
CGlobalPreferences::PDVContentFile(unsigned int pFace, unsigned int junk)
{
	CFace* p = (CFace*)pFace;
	CFaceContent* pfc = p->m_pContent;
	if (pfc == NULL) return "";
	CString sz = pfc->GetPath();
	int nSlash = sz.ReverseFind( '\\' );
	if (nSlash == -1) nSlash = sz.ReverseFind( '/' );
	if (nSlash >= 0) sz = sz.Mid( nSlash + 1 );
	return sz;
}

CString
CGlobalPreferences::PDVPageNo(unsigned int pFace, unsigned int junk)
{
	CFace* p = (CFace*)pFace;
	CString sz;
	sz.Format( "%d", p->GetPageNumber() + 1 );
	return sz;
}

CString
CGlobalPreferences::PDVFont(unsigned int pFace, unsigned int junk)
{
	CFace* p = (CFace*)pFace;
	return p->GetFontTypeface();
}

CString 
CGlobalPreferences::PDVShapeName(unsigned int pFace, unsigned int junk)
{
	CFace* p = (CFace*)pFace;
	CShape* ps = p->m_pOwner;
	if (ps == NULL) return "";
	return ps->GetName();
}

CString 
CGlobalPreferences::PDVShapeNumFaces(unsigned int pFace, unsigned int junk)
{
	CFace* p = (CFace*)pFace;
	CShape* ps = p->m_pOwner;
	if (ps == NULL) return "";
	CString sz;
	sz.Format( "%d", ps->GetNumFaces() );
	return sz;
}

CString 
CGlobalPreferences::PDVShapePath(unsigned int pFace, unsigned int junk)
{
	CFace* p = (CFace*)pFace;
	if (p == NULL) return "";
	CShape* ps = p->m_pOwner;
	if (ps == NULL) return "";
	return ps->m_SavedPath;
}

// Implementation of CPreferenceSet

CPreferenceSet::CPreferenceSet()
{
	// Default to no parent
	m_pParent = NULL;
	// Reset all to default values
	Reset();
}

CPreferenceSet::CPreferenceSet( CPreferenceSet* pParent )
{
	// Set parent to delegate to
	m_pParent = pParent;
	// Reset all to default values
	Reset();
}

// Reset to default values
void CPreferenceSet::Reset()
{
#define	SETDEFAULT(varbase,var,defval)	m_##varbase##var = defval; m_bHas##var = false
	SETDEFAULT( n, PictureFlow, 1 );
	SETDEFAULT( n, ScaleMethod, 0 );
	SETDEFAULT( n, FontPointsize, 12 );
	SETDEFAULT( n, EnableRotation, 0 );
	SETDEFAULT( n, UseTabs, UTB_FULL );
	SETDEFAULT( u, FontAttributes, 0 * 0x1000 /*no italic*/ | 0 * 0x2000 /*no ul*/ | 0 * 0x4000 /*no strikeout*/ | FW_NORMAL );
	SETDEFAULT( n, LayoutFit, LFIT_FULL );
	SETDEFAULT( clr, EdgeColor, RGB(0,0,0) );
	SETDEFAULT( clr, FaceColor, RGB(0,0,0) );
	SETDEFAULT( clr, TabColor, RGB(0,0,0) );
	SETDEFAULT( clr, FaceBgColor, RGB(255,255,255) );
	SETDEFAULT( clr, TabBgColor, RGB(255,255,255) );
	SETDEFAULT( clr, InwdEdgeColor, RGB(0,0,0) );
	SETDEFAULT( d, TabShoulderAngle, 30 );
	SETDEFAULT( d, MinFitAngle, 60 );
	SETDEFAULT( d, AreaRatio, 0.75 );
	SETDEFAULT( d, HeightRatio, 0.82 );
	SETDEFAULT( d, BaseRatio, 0.88 );
	SETDEFAULT( d, OrgRatio, 1.0 );
	SETDEFAULT( sz, DefaultFaceText, "" );
	SETDEFAULT( sz, FontTypeface, "Arial" );
} // CPreferenceSet::Reset()

// Set default values from another set (such as globals)
void
CPreferenceSet::CopySettings( CPreferenceSet const& src )
{
	// Clear all m_bHas settings
	Reset();
	m_nPictureFlow = src.m_nPictureFlow;
	m_nScaleMethod = src.m_nScaleMethod;
	m_nFontPointsize = src.m_nFontPointsize;
	m_nEnableRotation = src.m_nEnableRotation;
	m_uFontAttributes = src.m_uFontAttributes;
	m_nUseTabs = src.m_nUseTabs;
	m_nLayoutFit = src.m_nLayoutFit;
	m_clrEdgeColor = src.m_clrEdgeColor;
	m_clrFaceColor = src.m_clrFaceColor;
	m_clrTabColor = src.m_clrTabColor;
	m_clrFaceBgColor = src.m_clrFaceBgColor;
	m_clrTabBgColor = src.m_clrTabBgColor;
	m_clrInwdEdgeColor = src.m_clrInwdEdgeColor;
	m_dTabShoulderAngle = src.m_dTabShoulderAngle;
	m_dMinFitAngle = src.m_dMinFitAngle;
	m_dAreaRatio = src.m_dAreaRatio;
	m_dHeightRatio = src.m_dHeightRatio;
	m_dBaseRatio = src.m_dBaseRatio;
	m_dOrgRatio = src.m_dOrgRatio;
	m_szDefaultFaceText = src.m_szDefaultFaceText;
	m_szFontTypeface = src.m_szFontTypeface;
} // CPreferenceSet::CopySettings()

// Get only the values that have been explicitly set in symbolic form
CString
CPreferenceSet::GetSymbolicForm() const
{
	CString sz;
	CString szTemp;
#define	ADDSYMBOLSTR(var) if (m_bHas##var) \
		{ \
			CString szEscaped( m_sz##var ); \
			szEscaped.Replace( "\"", "\\\"" ); \
			szTemp.Format( " " #var "=\"%s\"", (LPCTSTR)szEscaped ); \
			sz += szTemp; \
		}
#define	ADDSYMBOLCVT(varbase,var,fmt) if (m_bHas##var) \
		{ \
			szTemp.Format( " " #var "=" fmt, m_##varbase##var ); \
			sz += szTemp; \
		}

	ADDSYMBOLCVT( n, PictureFlow, "%u" );
	ADDSYMBOLCVT( n, ScaleMethod, "%u" );
	ADDSYMBOLCVT( n, FontPointsize, "%u" );
	ADDSYMBOLCVT( n, EnableRotation, "%u" );
	ADDSYMBOLCVT( n, UseTabs, "%u" );
	ADDSYMBOLCVT( n, LayoutFit, "%u" );
	ADDSYMBOLCVT( u, FontAttributes, "%lx" );
	ADDSYMBOLCVT( clr, EdgeColor, "%x" );
	ADDSYMBOLCVT( clr, FaceColor, "%x" );
	ADDSYMBOLCVT( clr, TabColor, "%x" );
	ADDSYMBOLCVT( clr, FaceBgColor, "%x" );
	ADDSYMBOLCVT( clr, TabBgColor, "%x" );
	ADDSYMBOLCVT( clr, InwdEdgeColor, "%x" );
	ADDSYMBOLCVT( d, TabShoulderAngle, "%lf" );
	ADDSYMBOLCVT( d, MinFitAngle, "%lf" );
	ADDSYMBOLCVT( d, AreaRatio, "%lf" );
	ADDSYMBOLCVT( d, HeightRatio, "%lf" );
	ADDSYMBOLCVT( d, BaseRatio, "%lf" );
	ADDSYMBOLCVT( d, OrgRatio, "%lf" );
	ADDSYMBOLSTR( DefaultFaceText );
	ADDSYMBOLSTR( FontTypeface );

	return sz;
} // CPreferenceSet::GetSymbolicForm()

// Parse values returned by GetSymbolicForm()
int
CPreferenceSet::ParseSymbolicForm( CString& szData )
{
	CString sz;
	int nTotal = 0;
	int nFound;
#define	PARSESYMBOLCVT(varbase,var,fmt) if ((nFound = szData.Find( #var "=" )) != -1) \
		{ \
			nTotal++; \
			nFound += strlen( #var "=" ); \
			sz = szData.Mid( nFound ).SpanExcluding( " \t\r\n" ); \
			m_bHas##var = (sscanf( sz, fmt, &m_##varbase##var ) >= 1); \
		}
#define	PARSESYMBOLSTR(var) if ((nFound = szData.Find( #var "=\"" )) != -1) \
		{ \
			nTotal++; \
			nFound += strlen( #var "=\"" ); \
			m_sz##var = szData.Mid( nFound ); \
			m_bHas##var = true; \
			int n; \
			for (n = 0; n < m_sz##var.GetLength(); n++) \
			{ \
				char c = m_sz##var.GetAt( n ); \
				if (c == '\\') \
				{ \
					m_sz##var.Delete( n++, 1 ); \
				} else if (c == '"') \
				{ \
					m_sz##var.Delete( n, m_sz##var.GetLength() - n ); \
				} \
			} \
		}

	PARSESYMBOLCVT( n, PictureFlow, "%u" );
	PARSESYMBOLCVT( n, ScaleMethod, "%u" );
	PARSESYMBOLCVT( n, FontPointsize, "%u" );
	PARSESYMBOLCVT( n, EnableRotation, "%u" );
	PARSESYMBOLCVT( n, UseTabs, "%u" );
	PARSESYMBOLCVT( n, LayoutFit, "%u" );
	PARSESYMBOLCVT( u, FontAttributes, "%lx" );
	PARSESYMBOLCVT( clr, EdgeColor, "%x" );
	PARSESYMBOLCVT( clr, FaceColor, "%x" );
	PARSESYMBOLCVT( clr, TabColor, "%x" );
	PARSESYMBOLCVT( clr, FaceBgColor, "%x" );
	PARSESYMBOLCVT( clr, TabBgColor, "%x" );
	PARSESYMBOLCVT( clr, InwdEdgeColor, "%x" );
	PARSESYMBOLCVT( d, TabShoulderAngle, "%lf" );
	PARSESYMBOLCVT( d, MinFitAngle, "%lf" );
	PARSESYMBOLCVT( d, AreaRatio, "%lf" );
	PARSESYMBOLCVT( d, HeightRatio, "%lf" );
	PARSESYMBOLCVT( d, BaseRatio, "%lf" );
	PARSESYMBOLCVT( d, OrgRatio, "%lf" );
	PARSESYMBOLSTR( DefaultFaceText );
	PARSESYMBOLSTR( FontTypeface );

	return nTotal;

} // CPreferenceSet::ParseSymbolicForm()

#define	GETORDELEGATE( varbase, var )	if (m_bHas##var || !GetParent()) return m_##varbase##var; \
										return GetParent()->Get##var()
#define SETVALUE( varbase, var, val )	m_##varbase##var = val; m_bHas##var = true

int CPreferenceSet::GetPictureFlow() const { GETORDELEGATE( n, PictureFlow ); }
void CPreferenceSet::SetPictureFlow( int n ) { SETVALUE( n, PictureFlow, n ); }

int CPreferenceSet::GetScaleMethod() const { GETORDELEGATE( n, ScaleMethod ); }
void CPreferenceSet::SetScaleMethod( int n ) { SETVALUE( n, ScaleMethod, n ); }

int CPreferenceSet::GetFontPointsize() const { GETORDELEGATE( n, FontPointsize ); }
void CPreferenceSet::SetFontPointsize( int n ) { SETVALUE( n, FontPointsize, n ); }

int CPreferenceSet::GetEnableRotation() const { GETORDELEGATE( n, EnableRotation ); }
void CPreferenceSet::SetEnableRotation( int n ) { SETVALUE( n, EnableRotation, n ); }

int CPreferenceSet::GetUseTabs() const { GETORDELEGATE( n, UseTabs ); }
void CPreferenceSet::SetUseTabs( int n ) { SETVALUE( n, UseTabs, n ); }

int CPreferenceSet::GetLayoutFit() const { GETORDELEGATE( n, LayoutFit ); }
void CPreferenceSet::SetLayoutFit( int n ) { SETVALUE( n, LayoutFit, n ); }

unsigned long CPreferenceSet::GetFontAttributes() const { GETORDELEGATE( u, FontAttributes ); }
void CPreferenceSet::SetFontAttributes( unsigned long u ) { SETVALUE( u, FontAttributes, u ); }

double CPreferenceSet::GetMinFitAngle() const { GETORDELEGATE( d, MinFitAngle ); }
void CPreferenceSet::SetMinFitAngle(double d) { SETVALUE( d, MinFitAngle, d ); }

double CPreferenceSet::GetTabShoulderAngle() const { GETORDELEGATE( d, TabShoulderAngle ); }
void CPreferenceSet::SetTabShoulderAngle( double d ) { SETVALUE( d, TabShoulderAngle, d ); }

double CPreferenceSet::GetAreaRatio() const { GETORDELEGATE( d, AreaRatio ); }
void CPreferenceSet::SetAreaRatio( double d ) { SETVALUE( d, AreaRatio, d ); }

double CPreferenceSet::GetHeightRatio() const { GETORDELEGATE( d, HeightRatio ); }
void CPreferenceSet::SetHeightRatio( double d ) { SETVALUE( d, HeightRatio, d ); }

double CPreferenceSet::GetBaseRatio() const { GETORDELEGATE( d, BaseRatio ); }
void CPreferenceSet::SetBaseRatio( double d ) { SETVALUE( d, BaseRatio, d ); }

double CPreferenceSet::GetOrgRatio() const { GETORDELEGATE( d, OrgRatio ); }
void CPreferenceSet::SetOrgRatio( double d ) { SETVALUE( d, OrgRatio, d ); }

COLORREF CPreferenceSet::GetEdgeColor() const { GETORDELEGATE( clr, EdgeColor ); }
void CPreferenceSet::SetEdgeColor( COLORREF clr ) { SETVALUE( clr, EdgeColor, clr ); }

COLORREF CPreferenceSet::GetFaceColor() const { GETORDELEGATE( clr, FaceColor ); }
void CPreferenceSet::SetFaceColor( COLORREF clr ) { SETVALUE( clr, FaceColor, clr ); }

COLORREF CPreferenceSet::GetTabColor() const { GETORDELEGATE( clr, TabColor ); }
void CPreferenceSet::SetTabColor( COLORREF clr ) { SETVALUE( clr, TabColor, clr ); }

COLORREF CPreferenceSet::GetFaceBgColor() const { GETORDELEGATE( clr, FaceBgColor ); }
void CPreferenceSet::SetFaceBgColor( COLORREF clr ) { SETVALUE( clr, FaceBgColor, clr ); }

COLORREF CPreferenceSet::GetTabBgColor() const { GETORDELEGATE( clr, TabBgColor ); }
void CPreferenceSet::SetTabBgColor( COLORREF clr ) { SETVALUE( clr, TabBgColor, clr ); }

COLORREF CPreferenceSet::GetInwdEdgeColor() const { GETORDELEGATE( clr, InwdEdgeColor ); }
void CPreferenceSet::SetInwdEdgeColor( COLORREF clr ) { SETVALUE( clr, InwdEdgeColor, clr ); }

CString CPreferenceSet::GetDefaultFaceText() const { GETORDELEGATE( sz, DefaultFaceText ); }
void CPreferenceSet::SetDefaultFaceText( LPCTSTR lp ) { SETVALUE( sz, DefaultFaceText, lp ); }

CString CPreferenceSet::GetFontTypeface() const { GETORDELEGATE( sz, FontTypeface ); }
void CPreferenceSet::SetFontTypeface( LPCTSTR lp ) { SETVALUE( sz, FontTypeface, lp ); }


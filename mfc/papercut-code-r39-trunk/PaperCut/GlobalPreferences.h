/*!

	@file	 GlobalPreferences.h

	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: GlobalPreferences.h 16 2006-04-15 06:39:12Z henry_groover $

	Preferences set. This class exposes all possible preferences and encapsulates
	serialization of preferences (by multiple inheritance). It handles resolution of 
	multiple levels of overrides, allowing global preferences to be overridden by shape 
	preferences (or other lower levels which may be implemented in future).

*/

#if !defined(AFX_GLOBALPREFERENCES_H__987E9C41_5638_11D6_A858_0040F4459482__INCLUDED_)
#define AFX_GLOBALPREFERENCES_H__987E9C41_5638_11D6_A858_0040F4459482__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxtempl.h>

class CPreferenceSet;

// Predefined global variable definition fetch callback
typedef CString (*PredefinedCallback)(unsigned int, unsigned int);

class CPreferenceSet
{
	// Enable access in CopySettings()
	friend class CPreferenceSet;
public:
	CPreferenceSet();
	CPreferenceSet( CPreferenceSet* pParent );

	// Reset all values to defaults
	void Reset();

	// Set parent to delegate to
	void SetParent( CPreferenceSet* pParent ) { m_pParent = pParent; }

	// Get parent preference set
	virtual CPreferenceSet* GetParent() const { return m_pParent; }

	// Set default values from another set (such as globals)
	virtual void CopySettings( CPreferenceSet const& src );

	// Get only the values that have been explicitly set in symbolic form
	virtual CString GetSymbolicForm() const;

	// Parse values returned by GetSymbolicForm()
	virtual int ParseSymbolicForm( CString& szData );

	// Scale method values
	enum {
		SM_BYAREA,
		SM_BYHEIGHT,
		SM_BYBASE,
		SM_BYPICTURE,
		SM_NONE
	};

	// Use tabs values
	enum {
		UTB_NONE,	// No tabs at all
		UTB_BLANK,	// Blank tabs with no help text
		UTB_NOHINT,	// Tabs with label text but no page hints
		UTB_FULL	// (Default) normal tabs
	};

	// Layout fit values
	enum {
		LFIT_NOROTATE,	// Do not rotate
		LFIT_BY90,		// Rotate only by 90 degree increments
		LFIT_NEG90,		// Rotate starting from -90
		LFIT_BY45,		// Rotate by 45 degree increments
		LFIT_NEG45,		// Rotate starting from -45
		LFIT_BY15,		// Rotate by 15 degree increments
		LFIT_NEG15,		// Rotate starting from -15
		LFIT_FULL		// Use most aggressive rotation fitting
	};

	// Accessors will delegate to parent if not set
	// Mutators always set Has attribute - only Reset() will clear

	int GetScaleMethod() const;
	void SetScaleMethod( int n );
	bool HasScaleMethod() const { return m_bHasScaleMethod; }

	int GetPictureFlow() const;
	void SetPictureFlow( int n );
	bool HasPictureFlow() const { return m_bHasPictureFlow; }

	int GetFontPointsize() const;
	void SetFontPointsize( int n );
	bool HasFontPointsize() const { return m_bHasFontPointsize; }

	int GetEnableRotation() const;
	void SetEnableRotation( int n );
	bool HasEnableRotation() const { return m_bHasEnableRotation; }

	int GetLayoutFit() const;
	void SetLayoutFit( int n );
	bool HasLayoutFit() const { return m_bHasLayoutFit; }

	unsigned long GetFontAttributes() const;
	void SetFontAttributes( unsigned long u );
	bool HasFontAttributes() const { return m_bHasFontAttributes; }

	double GetMinFitAngle() const;
	void SetMinFitAngle( double d );
	bool HasMinFitAngle() const { return m_bHasMinFitAngle; }

	double GetTabShoulderAngle() const;
	void SetTabShoulderAngle( double d );
	bool HasTabShoulderAngle() const { return m_bHasTabShoulderAngle; }

	double GetAreaRatio() const;
	void SetAreaRatio( double d );
	bool HasAreaRatio() const { return m_bHasAreaRatio; }

	double GetHeightRatio() const;
	void SetHeightRatio( double d );
	bool HasHeightRatio() const { return m_bHasHeightRatio; }

	double GetBaseRatio() const;
	void SetBaseRatio( double d );
	bool HasBaseRatio() const { return m_bHasBaseRatio; }

	double GetOrgRatio() const;
	void SetOrgRatio( double d );
	bool HasOrgRatio() const { return m_bHasOrgRatio; }

	COLORREF GetEdgeColor() const;
	void SetEdgeColor( COLORREF clr );
	bool HasEdgeColor() const { return m_bHasEdgeColor; }

	COLORREF GetFaceColor() const;
	void SetFaceColor( COLORREF clr );
	bool HasFaceColor() const { return m_bHasFaceColor; }

	COLORREF GetTabColor() const;
	void SetTabColor( COLORREF clr );
	bool HasTabColor() const { return m_bHasTabColor; }

	COLORREF GetFaceBgColor() const;
	void SetFaceBgColor( COLORREF clr );
	bool HasFaceBgColor() const { return m_bHasFaceBgColor; }

	COLORREF GetTabBgColor() const;
	void SetTabBgColor( COLORREF clr );
	bool HasTabBgColor() const { return m_bHasTabBgColor; }

	COLORREF GetInwdEdgeColor() const;
	void SetInwdEdgeColor( COLORREF clr );
	bool HasInwdEdgeColor() const { return m_bHasInwdEdgeColor; }

	CString GetDefaultFaceText() const;
	void SetDefaultFaceText( LPCTSTR lp );
	bool HasDefaultFaceText() const { return m_bHasDefaultFaceText; }

	CString GetFontTypeface() const;
	void SetFontTypeface( LPCTSTR lp );
	bool HasFontTypeface() const { return m_bHasFontTypeface; }

	int GetUseTabs() const;
	void SetUseTabs( int n );
	bool HasUseTabs() const { return m_bHasUseTabs; }

	double GetTabTextPct() const { return 0.65; }
	double GetTabTextOff() const { return 0.04; }
	double GetTabText2Pct() const { return 0.20; }
	double GetTabText2Off() const { return 0.68; }
protected:
	// Parent.. Defaults to null
	CPreferenceSet* m_pParent;

	// m_bHas flags indicate whether value has ever been set
	bool m_bHasScaleMethod;
	bool m_bHasPictureFlow;
	bool m_bHasMinFitAngle;
	bool m_bHasTabShoulderAngle;
	bool m_bHasAreaRatio;
	bool m_bHasHeightRatio;
	bool m_bHasBaseRatio;
	bool m_bHasOrgRatio;
	bool m_bHasEdgeColor;
	bool m_bHasFaceColor;
	bool m_bHasTabColor;
	bool m_bHasFaceBgColor;
	bool m_bHasTabBgColor;
	bool m_bHasInwdEdgeColor;
	bool m_bHasDefaultFaceText;
	bool m_bHasFontPointsize;
	bool m_bHasFontAttributes;
	bool m_bHasFontTypeface;
	bool m_bHasEnableRotation;
	bool m_bHasUseTabs;
	bool m_bHasLayoutFit;

	int m_nScaleMethod;
	int m_nPictureFlow;
	int m_nFontPointsize;
	int m_nEnableRotation;
	int m_nUseTabs;
	int m_nLayoutFit;

	unsigned long m_uFontAttributes;

	double m_dMinFitAngle;
	double m_dTabShoulderAngle;
	double m_dAreaRatio;
	double m_dHeightRatio;
	double m_dBaseRatio;
	double m_dOrgRatio;

	COLORREF m_clrEdgeColor;
	COLORREF m_clrFaceColor;
	COLORREF m_clrTabColor;
	COLORREF m_clrFaceBgColor;
	COLORREF m_clrTabBgColor;
	COLORREF m_clrInwdEdgeColor;

	CString m_szDefaultFaceText;
	CString m_szFontTypeface;
};

class CGlobalPreferences : public CPreferenceSet
{
public:
	CGlobalPreferences();
	~CGlobalPreferences();

	// There is no parent preference set
	virtual CPreferenceSet* GetParent() const { return NULL; }

	int Save();
	int Load();

	// Global preference dictionary for $varname$ substitution
	CMap<CString,const char*,CString,const char*> m_mapGlobalDict;

	// Predefined value dictionary for $varname$ substitutions such as $__date__$ etc.
	CMap<CString,const char*,PredefinedCallback,PredefinedCallback> m_mapPredefs;

	// Predefined value return functions
	static CString PDVDate(unsigned int junk1, unsigned int junk2);
	static CString PDVTime(unsigned int junk1, unsigned int junk2);
	static CString PDVFace(unsigned int pFace, unsigned int junk);
	static CString PDVGroup(unsigned int pFace, unsigned int Index);
	static CString PDVContentPath(unsigned int pFace, unsigned int junk);
	static CString PDVContentFile(unsigned int pFace, unsigned int junk);
	static CString PDVFont(unsigned int pFace, unsigned int junk);
	static CString PDVPageNo(unsigned int pFace, unsigned int junk);
	static CString PDVShapeName(unsigned int pFace, unsigned int junk);
	static CString PDVShapeNumFaces(unsigned int pFace, unsigned int junk);
	static CString PDVShapePath(unsigned int pFace, unsigned int junk);


protected:
};

#endif // !defined(AFX_GLOBALPREFERENCES_H__987E9C41_5638_11D6_A858_0040F4459482__INCLUDED_)

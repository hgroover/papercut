/*!

	@file	 FaceContent.h

	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: FaceContent.h 9 2006-03-08 14:41:10Z henry_groover $

	Content attached to a face. This class currently handles only images of various
	types via PLIB, but also handles cacheing of image objects when the same image
	has been used multiple times.

*/

#if !defined(AFX_FACECONTENT_H__A814E421_4EEF_11D6_A858_0040F4459482__INCLUDED_)
#define AFX_FACECONTENT_H__A814E421_4EEF_11D6_A858_0040F4459482__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <time.h>

class PLWinBmp;
class PLAnyPicDecoder;
class CShape;

// Wrapper class for face content (text, bitmap, etc)
class CFaceContent  
{
public:
	CFaceContent();
	virtual ~CFaceContent();

	// Returns -1 if failed, else file type
	int LoadFile( LPCTSTR lpFile );

	enum {
		ANY_GRAPHIC_FILE=0,
		BMP_FILE,
		GIF_FILE,
		JPEG_FILE,
		PNG_FILE,
		TIFF_FILE
	};

	// Unload file
	void UnloadFile() { m_bLoaded = false; }

	// Is file loaded?
	bool IsLoaded() const { return m_bLoaded; }

	// Get size of loaded bitmap
	SIZE GetSize();

	// Get path if loaded
	CString GetPath() const { return m_bLoaded ? m_szLoadedPath : ""; }

	// Render bitmap in device context
	void Draw( CDC* pDC, double dRotation, CRect *pDest, bool bClip, LPPOINT aMaskPolygon, int nMaskPoints, bool bDrawCaption = true );

	// Get serialized format
	CString GetSerialized( CShape* pShape );

	// Restore from serialization
	int FromSerialized( LPCTSTR lpSerialization, CShape* pShape );

	// Add to reference count
	void AddReference();

	// Decrement reference count, return number remaining
	int RemoveReference();

	// Copy actual file to new location iff different from current dir. Return bytes copied or -1 if failed
	int CopyContentFile( LPCTSTR lpDestDir, BOOL OverwriteExisting );

	// Set/get caption text
	void SetCaption( LPCTSTR szCaptionText ) { m_szCaption = szCaptionText; }
	CString GetCaption() { return m_szCaption; }

protected:
	int m_nReferenceCount;
	bool m_bLoaded;
	PLWinBmp *m_pBmp;
	PLAnyPicDecoder *m_pDecoder;
	typedef struct {
		time_t age;
		bool valid;
		int width;
		int height;
		double rotation;
		PLWinBmp *pBmp;
	} RCACHE;
	// Clear and free all cache entries
	void ClearCache();
	// Get index of a free cache entry, freeing oldest entry if necessary
	int GetFreeCacheIndex();
	// Get matching cache entry or -1 if not found
	int GetCacheMatch( int width, int height, double rotation );
	enum { MAX_CACHE = 12 };
	RCACHE m_aRotated[MAX_CACHE];
	size_t m_nRotatedCount;

	CString m_szLoadedPath;
	CString m_szCaption;
};

#endif // !defined(AFX_FACECONTENT_H__A814E421_4EEF_11D6_A858_0040F4459482__INCLUDED_)

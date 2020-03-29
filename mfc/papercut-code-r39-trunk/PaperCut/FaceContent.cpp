/*!

	@file	 FaceContent.cpp

	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: FaceContent.cpp 9 2006-03-08 14:41:10Z henry_groover $

	Content attached to a face. This class currently handles only images of various
	types via PLIB, but also handles cacheing of image objects when the same image
	has been used multiple times.

*/

#include "stdafx.h"
#include "PaperCut.h"
#include "FaceContent.h"
#include "Face.h"
#include "Shape.h"

#include <math.h>

#include "planydec.h"
#include "plwinbmp.h"

#include "Filter/plfilterresizebilinear.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFaceContent::CFaceContent()
{
	m_bLoaded = false;
	// We start with 1 because someone created us...
	m_nReferenceCount = 1;
	// No cache entries
	m_nRotatedCount = 0;
	// Hide instantiation of embedded classes from public view
	this->m_pBmp = new PLWinBmp();
	this->m_pDecoder = new PLAnyPicDecoder();
}

CFaceContent::~CFaceContent()
{
	// Destroy cache entries
	ClearCache();
}

// Clear and free all cache entries
void 
CFaceContent::ClearCache()
{
	size_t n;
	for (n = 0; n < m_nRotatedCount; n++)
	{
		if (!m_aRotated[n].valid)
		{
			continue;
		}
		m_aRotated[n].valid = false;
		delete m_aRotated[n].pBmp;
		m_aRotated[n].pBmp = NULL;
	}
	m_nRotatedCount = 0;
} // CFaceContent::ClearCache()

// Get index of a free cache entry, freeing oldest entry if necessary
int
CFaceContent::GetFreeCacheIndex()
{
	size_t n;
	time_t now = time( NULL );
	time_t oldest = now + 10000;
	int nOldest = -1;
	// First pass: look for invalid entries and oldest
	for (n = 0; n < m_nRotatedCount; n++)
	{
		if (!m_aRotated[n].valid)
		{
			m_aRotated[n].age = now;
			return n;
		}
		if (m_aRotated[n].age < oldest)
		{
			oldest = m_aRotated[n].age;
			nOldest = n;
		}
	} // for all entries

	// Do we have free entries on the end?
	if (m_nRotatedCount < MAX_CACHE)
	{
		n = m_nRotatedCount;
		m_nRotatedCount++;
		m_aRotated[n].valid = false;
		m_aRotated[n].age = now;
		return n;
	} // Add new entry

	if (nOldest < 0)
	{
		nOldest = 0;
	}

	// If we got this far, we have an entry which is
	// valid and must be freed.
	m_aRotated[nOldest].valid = false;
	delete m_aRotated[nOldest].pBmp;
	m_aRotated[nOldest].pBmp = NULL;
	m_aRotated[nOldest].age = now;
	return nOldest;
} // CFaceContent::GetFreeCacheIndex()

// Get matching cache entry or -1 if not found
int
CFaceContent::GetCacheMatch( int width, int height, double rotation )
{
	size_t n;
	for (n = 0; n < m_nRotatedCount; n++)
	{
		if (m_aRotated[n].valid
			&& m_aRotated[n].width == width
			&& m_aRotated[n].height == height
			&& m_aRotated[n].rotation == rotation)
		{
			m_aRotated[n].age = time( NULL );
			return n;
		} // found a match
	} // for all entries

	// No match
	return -1;
} // CFaceContent::GetCacheMatch()

// Returns -1 if failed, else file type
int
CFaceContent::LoadFile( LPCTSTR lpFile )
{
	m_szLoadedPath = lpFile;
	ClearCache();
	try {
		m_bLoaded = false;
		m_pDecoder->MakeBmpFromFile( lpFile, m_pBmp );
	}
	catch (...)
	{
		return -1;
	}
	m_bLoaded = true;
	return ANY_GRAPHIC_FILE;
} // CFaceContent::LoadFile()

// Copy actual file to new location iff different from current dir. Return bytes copied or -1 if failed
int 
CFaceContent::CopyContentFile( LPCTSTR lpDestDir, BOOL OverwriteExisting )
{
	char szDrive[1024];
	char szDir[1024];
	char szFilename[1024];
	char szExt[1024];
	if (m_szLoadedPath.IsEmpty()) return 0;
	CString szFileName, szSourceDir, szDestDir;
	szDestDir = lpDestDir;
	if (szDestDir.IsEmpty()) return 0;
	_splitpath( m_szLoadedPath, szDrive, szDir, szFilename, szExt );
	// Get forms of source and dest dir with trailing backslash
	if (szDestDir.Right(1) != "\\" && szDestDir.Right(1) != "/")
		szDestDir += '\\';
	int BytesCopied = -1;
	szSourceDir = szDrive;
	szSourceDir += szDir;
	if (!szSourceDir.IsEmpty() && 
		szSourceDir.Right(1) != "\\" && szSourceDir.Right(1) != "/")
		szSourceDir += '\\';
	szFileName = szFilename;
	szFileName += szExt;
	CString szDestPath;
	szDestPath = szDestDir;
	szDestPath += szFileName;
	if (CopyFile( m_szLoadedPath, szDestPath, OverwriteExisting ))
	{
		FILE *pf = fopen( szDestPath, "rb" );
		if (pf != NULL)
		{
			if (fseek( pf, 0L, SEEK_END ) == 0)
			{
				BytesCopied = ftell( pf );
				if (BytesCopied > 0)
				{
					// Change recorded path - no need to reload
					m_szLoadedPath = szDestPath;
				}
			}
			fclose( pf );
		}
	}
	return BytesCopied;
} // CFaceContent::CopyContentFile()

// GetRotatedBitmap	- Create a new bitmap with rotated image
// Returns		- Returns new bitmap with rotated image
// hDIB			- Device-independent bitmap to rotate
// radians		- Angle of rotation in radians
// clrBack		- Color of pixels in the resulting bitmap that do
//			  not get covered by source pixels
static HANDLE GetRotatedBitmap( BITMAPINFOHEADER& bmiHeader, RGBQUAD bmiColors[], float radians, COLORREF clrBack )
{
	int bpp = bmiHeader.biBitCount;		// Bits per pixel
	
	int nColors = bmiHeader.biClrUsed ? bmiHeader.biClrUsed : 
					1 << bpp;
	int nWidth = bmiHeader.biWidth;
	int nHeight = bmiHeader.biHeight;
	int nRowBytes = ((((nWidth * bpp) + 31) & ~31) / 8);

	// Make sure height is positive and biCompression is BI_RGB or BI_BITFIELDS
	DWORD &compression = bmiHeader.biCompression;
	if( nHeight < 0 || (compression!=BI_RGB && compression!=BI_BITFIELDS))
		return NULL;

	LPVOID lpDIBBits;
	if( bmiHeader.biBitCount > 8 )
		lpDIBBits = (LPVOID)((LPDWORD)(bmiColors +
			bmiHeader.biClrUsed) + 
			((compression == BI_BITFIELDS) ? 3 : 0));
	else
		lpDIBBits = (LPVOID)(bmiColors + nColors);

    
	// Compute the cosine and sine only once
	float cosine = (float)cos(radians);
	float sine = (float)sin(radians);

	// Compute dimensions of the resulting bitmap
	// First get the coordinates of the 3 corners other than origin
	int x1 = (int)(-nHeight * sine);
	int y1 = (int)(nHeight * cosine);
	int x2 = (int)(nWidth * cosine - nHeight * sine);
	int y2 = (int)(nHeight * cosine + nWidth * sine);
	int x3 = (int)(nWidth * cosine);
	int y3 = (int)(nWidth * sine);

	int minx = min(0,min(x1, min(x2,x3)));
	int miny = min(0,min(y1, min(y2,y3)));
	int maxx = max(x1, max(x2,x3));
	int maxy = max(y1, max(y2,y3));

	int w = maxx - minx;
	int h = maxy - miny;


	// Create a DIB to hold the result
	int nResultRowBytes = ((((w * bpp) + 31) & ~31) / 8);
	long len = nResultRowBytes * h;
	int nHeaderSize = sizeof( bmiHeader ); //((LPBYTE)lpDIBBits-(LPBYTE)hDIB) ;
	HANDLE hDIBResult = GlobalAlloc(GMEM_FIXED,len+nHeaderSize);
	// Initialize the header information
	memcpy( (void*)hDIBResult, (void*)&bmiHeader, nHeaderSize);
	memcpy( ((BYTE*)hDIBResult)+nHeaderSize, bmiColors, (LPBYTE)lpDIBBits-(LPBYTE)bmiColors );

	BITMAPINFO &bmInfoResult = *(LPBITMAPINFO)hDIBResult ;
	bmInfoResult.bmiHeader.biWidth = w;
	bmInfoResult.bmiHeader.biHeight = h;
	bmInfoResult.bmiHeader.biSizeImage = len;

	LPVOID lpDIBBitsResult = (LPVOID)((LPBYTE)hDIBResult + nHeaderSize);

	// Get the back color value (index)
	ZeroMemory( lpDIBBitsResult, len );
	DWORD dwBackColor;
	switch(bpp)
	{
	case 1:	//Monochrome
		if( clrBack == RGB(255,255,255) )
			memset( lpDIBBitsResult, 0xff, len );
		break;
	case 4:
	case 8:	//Search the color table
		int i;
		for(i = 0; i < nColors; i++ )
		{
			if( bmiColors[i].rgbBlue ==  GetBValue(clrBack)
				&& bmiColors[i].rgbGreen ==  GetGValue(clrBack)
				&& bmiColors[i].rgbRed ==  GetRValue(clrBack) )
			{
				if(bpp==4) i = i | i<<4;
				memset( lpDIBBitsResult, i, len );
				break;
			}
		}
		// If not match found the color remains black
		break;
	case 16:
		// Windows95 supports 5 bits each for all colors or 5 bits for red & blue
		// and 6 bits for green - Check the color mask for RGB555 or RGB565
		if( *((DWORD*)bmiColors) == 0x7c00 )
		{
			// Bitmap is RGB555
			dwBackColor = ((GetRValue(clrBack)>>3) << 10) + 
					((GetRValue(clrBack)>>3) << 5) +
					(GetBValue(clrBack)>>3) ;
		}
		else
		{
			// Bitmap is RGB565
			dwBackColor = ((GetRValue(clrBack)>>3) << 11) + 
					((GetRValue(clrBack)>>2) << 5) +
					(GetBValue(clrBack)>>3) ;
		}
		break;
	case 24:
	case 32:
		dwBackColor = (((DWORD)GetRValue(clrBack)) << 16) | 
				(((DWORD)GetGValue(clrBack)) << 8) |
				(((DWORD)GetBValue(clrBack)));
		break;
	}


	// Now do the actual rotating - a pixel at a time
	// Computing the destination point for each source point
	// will leave a few pixels that do not get covered
	// So we use a reverse transform - e.i. compute the source point
	// for each destination point

	for( int y = 0; y < h; y++ )
	{
		for( int x = 0; x < w; x++ )
		{
			int sourcex = (int)((x+minx)*cosine + (y+miny)*sine);
			int sourcey = (int)((y+miny)*cosine - (x+minx)*sine);
			if( sourcex >= 0 && sourcex < nWidth && sourcey >= 0 
				&& sourcey < nHeight )
			{
				// Set the destination pixel
				switch(bpp)
				{
					BYTE mask;
				case 1:		//Monochrome
					mask = *((LPBYTE)lpDIBBits + nRowBytes*sourcey + 
						sourcex/8) & (0x80 >> sourcex%8);
					//Adjust mask for destination bitmap
					mask = mask ? (0x80 >> x%8) : 0;
					*((LPBYTE)lpDIBBitsResult + nResultRowBytes*(y) + 
								(x/8)) &= ~(0x80 >> x%8);
					*((LPBYTE)lpDIBBitsResult + nResultRowBytes*(y) + 
								(x/8)) |= mask;
					break;
				case 4:
					mask = *((LPBYTE)lpDIBBits + nRowBytes*sourcey + 
						sourcex/2) & ((sourcex&1) ? 0x0f : 0xf0);
					//Adjust mask for destination bitmap
					if( (sourcex&1) != (x&1) )
						mask = (mask&0xf0) ? (mask>>4) : (mask<<4);
					*((LPBYTE)lpDIBBitsResult + nResultRowBytes*(y) + 
							(x/2)) &= ~((x&1) ? 0x0f : 0xf0);
					*((LPBYTE)lpDIBBitsResult + nResultRowBytes*(y) + 
							(x/2)) |= mask;
					break;
				case 8:
					BYTE pixel ;
					pixel = *((LPBYTE)lpDIBBits + nRowBytes*sourcey + 
							sourcex);
					*((LPBYTE)lpDIBBitsResult + nResultRowBytes*(y) + 
							(x)) = pixel;
					break;
				case 16:
					DWORD dwPixel;
					dwPixel = *((LPWORD)((LPBYTE)lpDIBBits + 
							nRowBytes*sourcey + sourcex*2));
					*((LPWORD)((LPBYTE)lpDIBBitsResult + 
						nResultRowBytes*y + x*2)) = (WORD)dwPixel;
					break;
				case 24:
					dwPixel = *((LPDWORD)((LPBYTE)lpDIBBits + 
						nRowBytes*sourcey + sourcex*3)) & 0xffffff;
					*((LPDWORD)((LPBYTE)lpDIBBitsResult + 
						nResultRowBytes*y + x*3)) |= dwPixel;
					break;
				case 32:
					dwPixel = *((LPDWORD)((LPBYTE)lpDIBBits + 
						nRowBytes*sourcey + sourcex*4));
					*((LPDWORD)((LPBYTE)lpDIBBitsResult + 
						nResultRowBytes*y + x*4)) = dwPixel;
				}
			}
			else 
			{
				// Draw the background color. The background color
				// has already been drawn for 8 bits per pixel and less
				switch(bpp)
				{
				case 16:
					*((LPWORD)((LPBYTE)lpDIBBitsResult + 
						nResultRowBytes*y + x*2)) = 
						(WORD)dwBackColor;
					break;
				case 24:
					*((LPDWORD)((LPBYTE)lpDIBBitsResult + 
						nResultRowBytes*y + x*3)) |= dwBackColor;
					break;
				case 32:
					*((LPDWORD)((LPBYTE)lpDIBBitsResult + 
						nResultRowBytes*y + x*4)) = dwBackColor;
					break;
				}
			}
		}
	}

	return hDIBResult;
}

static HANDLE GetRotatedBitmap( HANDLE hDIB, float radians, COLORREF clrBack )
{
	// Get source bitmap info
	BITMAPINFO &bmInfo = *(LPBITMAPINFO)hDIB ;
	return GetRotatedBitmap( bmInfo.bmiHeader, bmInfo.bmiColors, radians, clrBack );
}

//
//    See the Raster operations in MSDN
//
static DWORD ROP3FromIndex(DWORD dwIndex)
{
    static const DWORD ROP3Table[256] =
    {
        0x00000042, 0x00010289,
        0x00020C89, 0x000300AA,
        0x00040C88, 0x000500A9,
        0x00060865, 0x000702C5,
        0x00080F08, 0x00090245,
        0x000A0329, 0x000B0B2A,
        0x000C0324, 0x000D0B25,
        0x000E08A5, 0x000F0001,
        0x00100C85, 0x001100A6,
        0x00120868, 0x001302C8,
        0x00140869, 0x001502C9,
        0x00165CCA, 0x00171D54,
        0x00180D59, 0x00191CC8,
        0x001A06C5, 0x001B0768,
        0x001C06CA, 0x001D0766,
        0x001E01A5, 0x001F0385,
        0x00200F09, 0x00210248,
        0x00220326, 0x00230B24,
        0x00240D55, 0x00251CC5,
        0x002606C8, 0x00271868,
        0x00280369, 0x002916CA,
        0x002A0CC9, 0x002B1D58,
        0x002C0784, 0x002D060A,
        0x002E064A, 0x002F0E2A,
        0x0030032A, 0x00310B28,
        0x00320688, 0x00330008,
        0x003406C4, 0x00351864,
        0x003601A8, 0x00370388,
        0x0038078A, 0x00390604,
        0x003A0644, 0x003B0E24,
        0x003C004A, 0x003D18A4,
        0x003E1B24, 0x003F00EA,
        0x00400F0A, 0x00410249,
        0x00420D5D, 0x00431CC4,
        0x00440328, 0x00450B29,
        0x004606C6, 0x0047076A,
        0x00480368, 0x004916C5,
        0x004A0789, 0x004B0605,
        0x004C0CC8, 0x004D1954,
        0x004E0645, 0x004F0E25,
        0x00500325, 0x00510B26,
        0x005206C9, 0x00530764,
        0x005408A9, 0x00550009,
        0x005601A9, 0x00570389,
        0x00580785, 0x00590609,
        0x005A0049, 0x005B18A9,
        0x005C0649, 0x005D0E29,
        0x005E1B29, 0x005F00E9,
        0x00600365, 0x006116C6,
        0x00620786, 0x00630608,
        0x00640788, 0x00650606,
        0x00660046, 0x006718A8,
        0x006858A6, 0x00690145,
        0x006A01E9, 0x006B178A,
        0x006C01E8, 0x006D1785,
        0x006E1E28, 0x006F0C65,
        0x00700CC5, 0x00711D5C,
        0x00720648, 0x00730E28,
        0x00740646, 0x00750E26,
        0x00761B28, 0x007700E6,
        0x007801E5, 0x00791786,
        0x007A1E29, 0x007B0C68,
        0x007C1E24, 0x007D0C69,
        0x007E0955, 0x007F03C9,
        0x008003E9, 0x00810975,
        0x00820C49, 0x00831E04,
        0x00840C48, 0x00851E05,
        0x008617A6, 0x008701C5,
        0x008800C6, 0x00891B08,
        0x008A0E06, 0x008B0666,
        0x008C0E08, 0x008D0668,
        0x008E1D7C, 0x008F0CE5,
        0x00900C45, 0x00911E08,
        0x009217A9, 0x009301C4,
        0x009417AA, 0x009501C9,
        0x00960169, 0x0097588A,
        0x00981888, 0x00990066,
        0x009A0709, 0x009B07A8,
        0x009C0704, 0x009D07A6,
        0x009E16E6, 0x009F0345,
        0x00A000C9, 0x00A11B05,
        0x00A20E09, 0x00A30669,
        0x00A41885, 0x00A50065,
        0x00A60706, 0x00A707A5,
        0x00A803A9, 0x00A90189,
        0x00AA0029, 0x00AB0889,
        0x00AC0744, 0x00AD06E9,
        0x00AE0B06, 0x00AF0229,
        0x00B00E05, 0x00B10665,
        0x00B21974, 0x00B30CE8,
        0x00B4070A, 0x00B507A9,
        0x00B616E9, 0x00B70348,
        0x00B8074A, 0x00B906E6,
        0x00BA0B09, 0x00BB0226,
        0x00BC1CE4, 0x00BD0D7D,
        0x00BE0269, 0x00BF08C9,
        0x00C000CA, 0x00C11B04,
        0x00C21884, 0x00C3006A,
        0x00C40E04, 0x00C50664,
        0x00C60708, 0x00C707AA,
        0x00C803A8, 0x00C90184,
        0x00CA0749, 0x00CB06E4,
        0x00CC0020, 0x00CD0888,
        0x00CE0B08, 0x00CF0224,
        0x00D00E0A, 0x00D1066A,
        0x00D20705, 0x00D307A4,
        0x00D41D78, 0x00D50CE9,
        0x00D616EA, 0x00D70349,
        0x00D80745, 0x00D906E8,
        0x00DA1CE9, 0x00DB0D75,
        0x00DC0B04, 0x00DD0228,
        0x00DE0268, 0x00DF08C8,
        0x00E003A5, 0x00E10185,
        0x00E20746, 0x00E306EA,
        0x00E40748, 0x00E506E5,
        0x00E61CE8, 0x00E70D79,
        0x00E81D74, 0x00E95CE6,
        0x00EA02E9, 0x00EB0849,
        0x00EC02E8, 0x00ED0848,
        0x00EE0086, 0x00EF0A08,
        0x00F00021, 0x00F10885,
        0x00F20B05, 0x00F3022A,
        0x00F40B0A, 0x00F50225,
        0x00F60265, 0x00F708C5,
        0x00F802E5, 0x00F90845,
        0x00FA0089, 0x00FB0A09,
        0x00FC008A, 0x00FD0A0A,
        0x00FE02A9, 0x00FF0062,
    };

    return ROP3Table[dwIndex&0xFF];
}

static BYTE SwapROP3_SrcDst(BYTE bRop3)
{
    // swap 1,2 bit and 5,6 bit
    typedef struct
    {
        unsigned bit0 : 1;
        unsigned bit1 : 1;
        unsigned bit2 : 1;
        unsigned bit3 : 1;

        unsigned bit4 : 1;
        unsigned bit5 : 1;
        unsigned bit6 : 1;
        unsigned bit7 : 1;
    } BITS;
    DWORD dwRop3 = bRop3;
    BITS bits = *(BITS*)&dwRop3;

    unsigned t = bits.bit1;
    bits.bit1 = bits.bit2;
    bits.bit2 = t;

    t = bits.bit5;
    bits.bit5 = bits.bit6;
    bits.bit6 = t;

    return *(BYTE*)&bits;
}


#define    FORE_ROP3(ROP4)        (0x00FFFFFF&(ROP4))
#define    BACK_ROP3(ROP4)        (ROP3FromIndex(SwapROP3_SrcDst(BYTE((ROP4)>>24))))
#define DSTCOPY 0x00AA0029
#define DSTERASE 0x00220326 // dest = dest & (~src) : DSna

static BOOL WINAPI MyMaskBlt(HDC hdcDest, int nXDest, int nYDest, int nWidth, int nHeight,
    HDC hdcSrc, int nXSrc, int nYSrc,
    HBITMAP hbmMask, int xMask, int yMask,
    DWORD dwRop
)
{
    if ( hbmMask == NULL )
        return BitBlt(hdcDest, nXDest, nYDest, nWidth, nHeight, hdcSrc, 
                      nXSrc, nYSrc, FORE_ROP3(dwRop));

    // 1. make mask bitmap's dc
    HDC hDCMask = ::CreateCompatibleDC(hdcDest);
    HBITMAP hOldMaskBitmap = (HBITMAP)::SelectObject(hDCMask, hbmMask);
	// Why is this expected non-NULL???
    //ASSERT ( hOldMaskBitmap );

    // 2. make masked Background bitmap

    // 2.1 make bitmap
    HDC hDC1 = ::CreateCompatibleDC(hdcDest);
    ASSERT ( hDC1 );
    HBITMAP hBitmap2 = ::CreateCompatibleBitmap(hdcDest, nWidth, nHeight);
    HBITMAP hOldBitmap2 = (HBITMAP)::SelectObject(hDC1, hBitmap2);
    ASSERT ( hOldBitmap2 );

    // 2.2 draw dest bitmap and mask
    DWORD dwRop3 = BACK_ROP3(dwRop);
    ::BitBlt(hDC1, 0, 0, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, SRCCOPY);
    ::BitBlt(hDC1, 0, 0, nWidth, nHeight, hdcDest, nXDest, nYDest, dwRop3);
    ::BitBlt(hDC1, 0, 0, nWidth, nHeight, hDCMask, xMask, yMask, DSTERASE);

    // 3. make masked Foreground bitmap

    // 3.1 make bitmap
    HDC hDC2 = ::CreateCompatibleDC(hdcDest);
    ASSERT ( hDC2 );
    HBITMAP hBitmap3 = ::CreateCompatibleBitmap(hdcDest, nWidth, nHeight);
    HBITMAP hOldBitmap3 = (HBITMAP)::SelectObject(hDC2, hBitmap3);
    ASSERT ( hOldBitmap3 );

    // 3.2 draw src bitmap and mask
    dwRop3 = FORE_ROP3(dwRop);
    ::BitBlt(hDC2, 0, 0, nWidth, nHeight, hdcDest, nXDest, nYDest, SRCCOPY);
    ::BitBlt(hDC2, 0, 0, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, dwRop3);
    ::BitBlt(hDC2, 0, 0, nWidth, nHeight, hDCMask, xMask, yMask, SRCAND);

    // 4. combine two bitmap and copy it to hdcDest
    ::BitBlt(hDC1, 0, 0, nWidth, nHeight, hDC2, 0, 0, SRCPAINT);
    ::BitBlt(hdcDest, nXDest, nYDest, nWidth, nHeight, hDC1, 0, 0, SRCCOPY);

    // 5. restore all object
    ::SelectObject(hDCMask, hOldMaskBitmap);
    ::SelectObject(hDC1, hOldBitmap2);
    ::SelectObject(hDC2, hOldBitmap3);

    // 6. delete all temp object
    DeleteObject(hBitmap2);
    DeleteObject(hBitmap3);

    DeleteDC(hDC1);
    DeleteDC(hDC2);
    DeleteDC(hDCMask);

    return TRUE;
}


// Render bitmap in device context
void
CFaceContent::Draw( CDC* pDC, double dRotation, CRect* pDest, bool bClip, LPPOINT aMaskPolygon, int nMaskPoints, bool bDrawCaption /*=true*/ )
{
	ASSERT( m_bLoaded );
	ASSERT( !bClip || (aMaskPolygon != NULL && nMaskPoints >= 3) );
	if (!m_bLoaded)
	{
		return;
	}
	if (bClip && !aMaskPolygon)
	{
		bClip = false;
		CDbg::Out( 0, "Error: clipping specified but no mask polygon!\n" );
	}
	if (bClip && nMaskPoints < 3)
	{
		bClip = false;
		CDbg::Out( 0, "Error: mask polygon with only %d points specified, 3 is minimum!\n", nMaskPoints );
	}
	bool bRotating = (dRotation != 0.0);
	PLWinBmp *pBmp = m_pBmp;
	int x = pDest->left;
	int y = pDest->top;
	int width = pDest->Width();
	int height = pDest->Height();
	if (bRotating)
	{
		// Use circular complement - we go clockwise, library routines counter-clockwise
		double dLibRotation = fmod( 360 - dRotation, 360 );
		// Try to find a cached entry
		int nCache = this->GetCacheMatch( pDest->Width(), pDest->Height(), dRotation );
		if (nCache < 0)
		{
			nCache = this->GetFreeCacheIndex();
			m_aRotated[nCache].height = pDest->Height();
			m_aRotated[nCache].width = pDest->Width();
			m_aRotated[nCache].rotation = dRotation;
			PLWinBmp bmRotateSource;
			PLFilterResizeBilinear filter( pDest->Width(), pDest->Height() );
			bmRotateSource.CreateFilteredCopy( *m_pBmp, filter );
			BITMAPINFOHEADER *pHdr = bmRotateSource.GetBMI();
			HANDLE hbm = GetRotatedBitmap( *pHdr,
				(RGBQUAD*)bmRotateSource.GetBits(),
				(float)CFace::DegToRadans( dLibRotation ),
				RGB(255,255,255) );
			if (hbm != NULL)
			{
				m_aRotated[nCache].pBmp = new PLWinBmp();
				BITMAPINFO &bmInfo = *(LPBITMAPINFO)hbm ;
				m_aRotated[nCache].pBmp->CreateFromHDIBBitmap( &bmInfo.bmiHeader );
				::GlobalUnlock( hbm );
				::GlobalFree( hbm );
				m_aRotated[nCache].valid = true;
			}
			else
			{
				// FIXME This is really a severe error condition
				ASSERT(0);
				m_pBmp->StretchDraw( pDC->m_hDC, pDest->left, pDest->top, pDest->Width(), pDest->Height() );
				return;
			}
		} // No cache entry found
		// Draw it
		// Note that the rotated bitmap will typically have a larger size but the same center
		SIZE bmsize = m_aRotated[nCache].pBmp->GetSize();
		x = pDest->left + (pDest->Width() - bmsize.cx) / 2;
		y = pDest->top + (pDest->Height() - bmsize.cy) / 2;
		width = bmsize.cx;
		height = bmsize.cy;
		pBmp = m_aRotated[nCache].pBmp;

		if (CDbg::m_Level > 1)
		{
			//{DEBUG}
			CString sz;
			UINT Align = pDC->GetTextAlign();
			pDC->SetTextAlign( TA_CENTER | TA_TOP );
			sz.Format( "%s rot %.1lf C@%d,%d", (LPCTSTR)this->m_szLoadedPath, dRotation, x, y );
			pDC->TextOut( x, y, sz );
			pDC->SetTextAlign( TA_LEFT | TA_TOP );
			sz.Format( "%s rot %.1lf TL@%d,%d", (LPCTSTR)this->m_szLoadedPath, dRotation, pDest->left, pDest->top );
			pDC->TextOut( pDest->left, pDest->top, sz );
			pDC->SetTextAlign( TA_RIGHT | TA_BOTTOM );
			sz.Format( "%s rot %.1lf BR@%d,%d", (LPCTSTR)this->m_szLoadedPath, dRotation, pDest->bottom, pDest->right );
			pDC->TextOut( pDest->bottom, pDest->right, sz );
			pDC->SetTextAlign( Align );
			//{/DEBUG}
		}
	}
	// If no clipping, we're done
	if (!bClip && !bRotating)
	{
		pBmp->StretchDraw( pDC->m_hDC, x, y, width, height );
		if (!this->m_szCaption.IsEmpty())
		{
			CSize captionSize;
			// Use 1/3 the height of m for margin
			CString szMarginTest = "m";
			captionSize = pDC->GetTextExtent( szMarginTest );
			pDC->SetTextAlign( TA_LEFT | TA_TOP );
			int TextTop = y + height + captionSize.cy / 3;
			captionSize = pDC->GetTextExtent( m_szCaption );
			pDC->TextOut( x + width/2 - captionSize.cx / 2, TextTop, m_szCaption );
		}
	}
	else
	{
		// FIXME draw rotated caption
		// Build bitmap
		CDC memDC;
		memDC.CreateCompatibleDC( pDC );
		CBitmap bmMask;
		bmMask.CreateCompatibleBitmap( pDC, width, height );
		CBitmap *pbmOldBitmap = memDC.SelectObject( &bmMask );
		// Adjust polygon offsets
		LPPOINT apAdjusted = new POINT[nMaskPoints];
		int nPoint;
		for (nPoint = 0; nPoint < nMaskPoints; nPoint++)
		{
			apAdjusted[nPoint].x = aMaskPolygon[nPoint].x - x;
			apAdjusted[nPoint].y = aMaskPolygon[nPoint].y - y;
		}
		// Draw polygon filled white
		CPen penHollow;
		penHollow.CreateStockObject( NULL_PEN );
		CBrush brBlack;
		brBlack.CreateStockObject( WHITE_BRUSH );
		CPen* ppenOld = memDC.SelectObject( &penHollow );
		CBrush* pbrOld = memDC.SelectObject( &brBlack );
		// Fill black
		memDC.FillSolidRect( 0, 0, width, height, RGB(0,0,0) );
		// Draw white mask
		memDC.Polygon( apAdjusted, nMaskPoints );
		memDC.SelectObject( pbrOld );
		memDC.SelectObject( ppenOld );
		CDC memPicDC;
		memPicDC.CreateCompatibleDC( pDC );
		// Create device-dependent bitmap
		CBitmap bmImage;
		bmImage.CreateCompatibleBitmap( pDC, width, height );
		CBitmap* pbmOldImage2 = memPicDC.SelectObject( &bmImage );
		pBmp->StretchDraw( memPicDC.m_hDC, 0, 0, width, height );

		// 0xdd0228 = SDno
		// SRCINVERT = DSx
		// 0x990066 = DSxn
		if (bClip)
		{
			// Combine destination and source with black = transparent
			memPicDC.BitBlt( 0, 0, width, height, &memDC, 0, 0, SRCAND ); // DSa
			// Get surrounding area
			memDC.BitBlt( 0, 0, width, height, pDC, x, y, SRCERASE ); // SDna
			// Merge together
			memPicDC.BitBlt( 0, 0, width, height, &memDC, 0, 0, SRCPAINT ); // DSo
			// Output result
			pDC->BitBlt( x, y, width, height, &memPicDC, 0, 0, SRCCOPY );
		}
		else
		{
			// Combine destination and source with black = transparent
			memPicDC.BitBlt( 0, 0, width, height, &memDC, 0, 0, SRCAND ); // DSa
			// Get surrounding area
			memDC.BitBlt( 0, 0, width, height, pDC, x, y, SRCERASE );
			// Merge together
			memPicDC.BitBlt( 0, 0, width, height, &memDC, 0, 0, SRCPAINT );
			// Output result
			pDC->BitBlt( x, y, width, height, &memPicDC, 0, 0, SRCCOPY );
		}
		// FIXME This does not handle rotation...
		if (!this->m_szCaption.IsEmpty())
		{
			CSize captionSize;
			// Use 1/3 the height of m for margin
			CString szMarginTest = "m";
			captionSize = pDC->GetTextExtent( szMarginTest );
			pDC->SetTextAlign( TA_LEFT | TA_TOP );
			int TextTop = y + height + captionSize.cy / 3;
			captionSize = pDC->GetTextExtent( m_szCaption );
			pDC->TextOut( x + width/2 - captionSize.cx / 2, TextTop, m_szCaption );
		}
		//pDC->BitBlt( x, y, width, height, &memDC, 0, 0, SRCAND );
		// Select our bitmap into memory DC
		//pBmp->StretchDraw( memDC.m_hDC, 0, 0, width, height );
		// Now do the mask blt
		//MyMaskBlt( pDC->m_hDC, x, y, width, height, memPicDC.m_hDC, 0, 0, (HBITMAP)bmMask.m_hObject, 0, 0, SRCCOPY );
		//pDC->BitBlt( x, y, width, height, &memPicDC, 0, 0, SRCPAINT );
		memPicDC.SelectObject( pbmOldImage2 );
		memDC.SelectObject( pbmOldBitmap );
		// Clean up
		bmMask.DeleteObject();
		bmImage.DeleteObject();
		memDC.DeleteDC();
		delete apAdjusted;
	}
} // CFaceContent::Draw()

// Get serialized format
CString
CFaceContent::GetSerialized( CShape* pShape )
{
	CString sz;
	if (m_szLoadedPath.IsEmpty())
	{
		return sz;
	}
	CString szCaption = m_szCaption;
	szCaption.Replace( "^", "&caret;" );
	szCaption.Replace( "\r\n", "^" );
	szCaption.Replace( "\"", "&dquot;" );
	sz.Format( "file=\"%s\" caption=\"%s\"", 
		(LPCTSTR)pShape->SaveTransformPath( m_szLoadedPath ),
		(LPCTSTR)szCaption );
	return sz;
} // CFaceContent::GetSerialized()

// Restore from serialization
int
CFaceContent::FromSerialized( LPCTSTR lpSerialization, CShape* pShape )
{
	ASSERT( lpSerialization != NULL );
	LPCTSTR lpFile, lpCaption;
	lpFile = strstr( lpSerialization, "file=\"" );
	if (!lpSerialization)
	{
		return -1;
	} // Invalid format
	lpFile += 6;
	CString sz;
	sz = lpFile;
	sz = sz.SpanExcluding( "\"" );
	lpFile += sz.GetLength();
	lpFile++; // Skip quote
	int nLoadResult = LoadFile( pShape->LoadTransformPath( sz ) );
	if (nLoadResult >= 0)
	{
		lpCaption = strstr( lpFile, "caption=\"" );
		if (lpCaption != NULL)
		{
			lpCaption += 9;
			sz = lpCaption;
			sz = sz.SpanExcluding( "\"" );
			sz.Replace( "&dquot;", "\"" );
			sz.Replace( "^", "\r\n" );
			sz.Replace( "&dcaret;", "^" );
			this->SetCaption( sz );
		}
	}
	return nLoadResult;
} // CFaceContent::FromSerialized()

// Add to reference count
void
CFaceContent::AddReference()
{
	m_nReferenceCount++;
} // CFaceContent::AddReference()

// Decrement reference count, return number remaining
int
CFaceContent::RemoveReference()
{
	if (m_nReferenceCount > 0)
	{
		m_nReferenceCount--;
	}
	return m_nReferenceCount;
} // CFaceContent::RemoveReference()

// Get size of loaded bitmap
SIZE CFaceContent::GetSize() 
{ 
	return m_pBmp->GetSize(); 
}

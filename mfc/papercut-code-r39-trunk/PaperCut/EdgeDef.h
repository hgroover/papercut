/*!

	@file	 EdgeDef.h

	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id$

	EDGEDEF struct definition. This is used to define shape primitives for creation of 
	predefined shapes.

*/

#if !defined(EDGEDEF_H_INCLUDED)
#define EDGEDEF_H_INCLUDED

#include <afxtempl.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Primitive types used in ShapeCreation.cpp
typedef struct tagEdgeDef {
	int nEdges;
	LPCTSTR *aszName;
	double *adLength;
	double *adEndAngles;
} EDGEDEF;

typedef struct tagFaceDef {
	LPCTSTR szType;
	LPCTSTR szName;
	LPCTSTR szBaseEdge;
	float fOrientation; // Degrees from left end of base edge to face perpendicular
} FACEDEF;

typedef CMap<CString,LPCTSTR,EDGEDEF*,EDGEDEF*> EDGEDEFDICT;


#endif // !defined(EDGEDEF_H_INCLUDED)

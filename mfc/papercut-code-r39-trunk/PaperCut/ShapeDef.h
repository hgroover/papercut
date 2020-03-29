/*!

	@file	 ShapeDef.h
	
	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: ShapeDef.h 12 2006-03-10 06:10:30Z henry_groover $

  Declaration of class for containing a single shapedef record from papercut.shapedef
  Shape definitions are used to create shape primitives such as Tetrahedron

*/

#pragma once

class CShape;
class CShapeDef;


// Shape definition class - this represents a single shapedef record from papercut.shapedef
// Shape definitions are used to create shape primitives such as Tetrahedron
class CShapeDef
{
public:
	CShapeDef(void);
	~CShapeDef(void);

	CArray<CString,LPCTSTR> m_aMenuTags;
	CArray<CString,LPCTSTR> m_aAltNames;
	CArray<CString,LPCTSTR> m_aJoins;
	// Define face definitions for H(exagon), P(entagon), S(quare), T(riangle), etc.
	// Face definitions consisting of one face, comma-delimited, per entry, with entries delimited by ;
	// e.g. a,1.0,90.0;b,1.0,90.0;c,1.0,90.0 for an equilateral triangle abc
	CMap<CString,LPCTSTR,CString,LPCTSTR> m_mapFaceDefs;
	// Define actual faces used in shape
	CArray<CString,LPCTSTR> m_aFaces;

	// Name used to uniquely identify shape
	CString m_szName;

	// Brief description, e.g. "26 faces, 8 of which are equilateral triangles, the rest are squares"
	CString m_szBrief;

	// Load from file. Return number of definitions read or -1 if file not present or read error
	static int LoadFromFile( LPCTSTR szSourceFile, CMap<CString,LPCTSTR,CShapeDef*,CShapeDef*>& map );

	// Initialize shape from definition in map. Return false if not found
	static bool CreateShape( CShape& shp, LPCTSTR szName, CMap<CString,LPCTSTR,CShapeDef*,CShapeDef*>& map );

	// Create shape
	void CreateShape(CShape& shp);

};

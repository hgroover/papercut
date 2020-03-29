/*!

	@file	 ShapeDef.cpp
	
	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: ShapeDef.cpp 12 2006-03-10 06:10:30Z henry_groover $

  Implementation of class for containing a single shapedef record from papercut.shapedef
  Shape definitions are used to create shape primitives such as Tetrahedron

*/

#include "stdafx.h"
#include ".\shapedef.h"

#include "PaperCutDoc.h"

#include "Shape.h"
#include "Face.h"

CShapeDef::CShapeDef(void)
{
}

CShapeDef::~CShapeDef(void)
{
}

// Load from file. Return number of definitions read or -1 if file not present or read error
int CShapeDef::LoadFromFile( LPCTSTR szSourceFile, CMap<CString,LPCTSTR,CShapeDef*,CShapeDef*>& map )
{
	int nReturnValue = -1;
	int nDefinitionsRead = 0;
	try
	{
		CFile f( szSourceFile, CFile::modeRead | CFile::shareDenyNone );
		CArray<CString,LPCTSTR> a;
		// Get contents into an array, stripping out comments and blank lines
		if (CPaperCutDoc::ReadFileIntoArray( &f, a ))
		{
			CShapeDef *pDef = NULL;
			// Parser state [pseudo-]machine
			// This is not a precedence-free grammar - we have certain requirements such as
			// braces starting on a line by themselves. We handle brace transitions separately.
			// We're only keeping track of statements expected to be found at specified levels.
			enum {
				// Define basic statement types for state machine
				SD_UNDEFINED,
				SD_SHAPEDEF,
				SD_MENU,
				SD_BRIEF,
				SD_ALTNAMES,
				SD_FACETYPE,
				SD_FACES,
				SD_JOINS,
				SD_POSTCREATE,
			};
			typedef struct tagShapedefStruct {
				const char *szStatement;
				int Level;
				int Statement;
			} SHAPEDEFSTRUCT;
#define MAXLEVELS	4
#define MAXSTATEMENTLEVELS	(MAXLEVELS-1)
			SHAPEDEFSTRUCT fsa[] = {
				{ "shapedef", 0, SD_SHAPEDEF },

				{ "menu", 1, SD_MENU },
				{ "brief", 1, SD_BRIEF },
				{ "altnames", 1, SD_ALTNAMES },
				{ "facetype", 1, SD_FACETYPE },
				{ "faces", 1, SD_FACES },
				{ "joins", 1, SD_JOINS },
				{ "postcreate", 1, SD_POSTCREATE },

				{ NULL, 0, 0 }
			};
			// We currently only allow one level of nesting
			// At level 0 we should only encounter shapedef statements
			// Level 1 is normal for the content of a shapedef statement
			// Level 2 is normal for the content of a statement within a shapedef's contents, e.g. menu
			int nStartOffset = -1;
			int Level = 0; // Current nesting level
			int Statement = SD_UNDEFINED;
			CString szShapeName;
			CString szFaceDefName;
			CString szFaceDefSep;
			for (int n = 0; n < a.GetSize(); n++)
			{
				// Handle curly braces
				if (a[n]=="{")
				{
					Level++;
					if (Level >= MAXLEVELS)
					{
						AfxMessageBox( "Error: maximum levels exceeded in reading shapedefs" );
						return -1;
					}
					continue;
				}
				else if (a[n]=="}")
				{
					Level--;
					if (Level < 0)
					{
						AfxMessageBox( "Error: mismatched {} in shapedefs" );
						return -1;
					}
					// Check for end of statement
					if (pDef != NULL && Level == 0)
					{
						map.SetAt( szShapeName, pDef );
						nDefinitionsRead++;
						pDef = NULL;
					}
					continue;
				}
				// Check for statements
				CString szToken = a[n].SpanExcluding(" \t");
				for (int nStatement = 0; fsa[nStatement].szStatement != NULL; nStatement++)
				{
					if (fsa[nStatement].Level > Level) break;
					if (fsa[nStatement].Level < Level) continue;
					if (fsa[nStatement].szStatement[0] == '*')
					{
						Statement = fsa[nStatement].Statement;
						break;
					}
					if (!szToken.CompareNoCase( fsa[nStatement].szStatement ))
					{
						Statement = fsa[nStatement].Statement;
						break;
					}
				}
				// Handle statements
				switch (Statement)
				{
				case SD_SHAPEDEF:
					if (Level == 0)
					{
						szShapeName = a[n].Mid( szToken.GetLength() + 1 );
						pDef = new CShapeDef();
						pDef->m_szName = szShapeName;
					}
					break;
				case SD_BRIEF:
					if (pDef != NULL)
					{
						if (!pDef->m_szBrief.IsEmpty())
						{
							pDef->m_szBrief += ' ';
						}
						// Support single-line brief description
						if (Level == 1)
						{
							pDef->m_szBrief += a[n].Mid( szToken.GetLength() + 1 );
						}
						// Multi-line description
						else
						{
							pDef->m_szBrief += a[n];
						}
					}
					break;
				case SD_MENU:
					if (pDef != NULL && Level > 1)
					{
						pDef->m_aMenuTags.Add( a[n] );
					}
					break;
				case SD_ALTNAMES:
					if (pDef != NULL && Level > 1)
					{
						pDef->m_aAltNames.Add( a[n] );
					}
					break;
				case SD_FACETYPE:
					if (pDef != NULL)
					{
						if (Level == 1)
						{
							szFaceDefName = a[n].Mid( szToken.GetLength() + 1 );
							pDef->m_mapFaceDefs.SetAt(szFaceDefName, "");
							szFaceDefSep.Empty();
						}
						else
						{
							CString sz = pDef->m_mapFaceDefs[szFaceDefName];
							sz += szFaceDefSep;
							sz += a[n];
							szFaceDefSep = ";";
							pDef->m_mapFaceDefs[szFaceDefName] = sz;
						}
					}
					break;
				case SD_FACES:
					if (pDef != NULL && Level > 1)
					{
						pDef->m_aFaces.Add( a[n] );
					}
					break;
				case SD_JOINS:
					if (pDef != NULL && Level > 1)
					{
						pDef->m_aJoins.Add( a[n] );
					}
					break;
				case SD_POSTCREATE:
					break;
				}
			}
			nReturnValue = nDefinitionsRead;
		}
		f.Close();
	}
	catch (...)
	{
		return -1;
		//OutputDebugString( ex.ReportError(
	}
	return nReturnValue;
}

// Create shape from definition in map
bool CShapeDef::CreateShape( CShape& shp, LPCTSTR szName, CMap<CString,LPCTSTR,CShapeDef*,CShapeDef*>& map )
{
	CShapeDef* pDef;
	if (!map.Lookup( szName, pDef ))
	{
		// Definition not found
		return false;
	}
	pDef->CreateShape( shp );
	return true;
}

// Split a string, delimited, into array
void SplitString( CArray<CString,LPCTSTR>& a, LPCTSTR sz, LPCTSTR szDelims )
{
	a.RemoveAll();
	CString szBuff = sz;
	LPTSTR lp = szBuff.GetBuffer();
	for (char *Token = strtok( lp, szDelims ); Token != NULL; Token = strtok( NULL, szDelims ))
	{
		if (Token[0] != '\0')
		{
			a.Add( Token );
		}
	}
}

// Create shape
void CShapeDef::CreateShape(CShape& shp)
{
	shp.ClearAllFaces();
	shp.SetName( this->m_szName );

	// Create faces
	// We have:
	//this->m_aFaces[0] = "T,TA,c,90.0";
	//this->m_mapFaceDefs["T"] = "a,1.0,90.0;b,1.0,90.0;c,1.0,90.0";
	CArray<CString,LPCTSTR> aFaceParms, aFaceTypeDef, aEdgeDef, aEdgeNames;
	int nFace;
	for (nFace = 0; nFace < this->m_aFaces.GetCount(); nFace++)
	{
		// Split up face definition elements
		SplitString( aFaceParms, this->m_aFaces[nFace], ", \t" );
		if (aFaceParms.GetCount() < 4)
		{
			AfxMessageBox( "Invalid face parameter count" );
			return;
		}
		// Set up array of edge lengths and edge names for use with CreatePolygon()
		CString szFaceTypeDef;
		if (!this->m_mapFaceDefs.Lookup( aFaceParms[0], szFaceTypeDef ))
		{
			AfxMessageBox( "Invalid face type reference" );
			return;
		}
		SplitString( aFaceTypeDef, szFaceTypeDef, ";" );
		int SideCount = aFaceTypeDef.GetCount();
		if (SideCount < 3)
		{
			AfxMessageBox( "Invalid face type definition (sides < 3)" );
			return;
		}
		const char **apszEdgeNames = new const char*[SideCount];
		double *adEdges = new double[SideCount];
		double *adAngles = new double[SideCount];
		aEdgeNames.RemoveAll();
		for (int nEdge = 0; nEdge < SideCount; nEdge++)
		{
			SplitString( aEdgeDef, aFaceTypeDef[nEdge], "," );
			if (aEdgeDef.GetSize() < 2)
			{
				AfxMessageBox( "Invalid edge definition" );
				return;
			}
			aEdgeNames.Add( aEdgeDef[0] );
			apszEdgeNames[nEdge] = aEdgeNames[nEdge];
			adEdges[nEdge] = atof( aEdgeDef[1] );
			// Angle is optional - default to 0
			if (aEdgeDef.GetSize() > 2)
			{
				adAngles[nEdge] = atof( aEdgeDef[2] );
			}
			else
			{
				adAngles[nEdge] = 0.0;
			}
		}

		CFace* pf = new CFace( &shp, aFaceParms[1] );
		pf->CreatePolygon( apszEdgeNames, adEdges, adAngles, SideCount );
		pf->SetBaseEdgeName( aFaceParms[2] );
		pf->m_fOrientation = atof( aFaceParms[3] );
		shp.m_mapFaces.SetAt( pf->m_szFaceName, pf );

		aEdgeNames.RemoveAll();

		delete [] apszEdgeNames;
		delete [] adEdges;
		delete [] adAngles;
	} // for all faces

	// Join edges
	shp.JoinFromList( this->m_aJoins );

	// Create map of unified vertices
	shp.MapVertices();

	// Set default page width and proportion
	shp.SetDefaultWidth();

}

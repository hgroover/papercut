/*!

	@file	 ImportOFF.cpp
	
	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id$

  Implementation of CPaperCutDoc::ImportOFF()

*/

#include "stdafx.h"
#include "PaperCut.h"

#include "PaperCutDoc.h"
#include "ThreePoint.h"
#include "ThreePlane.h"
#include "DlgInputScale.h"

// Read a line from a FILE. Remove line termination and return buffer pointer or NULL if failed (EOF)
static char *ReadLine( FILE *pf, char *Buff, int BuffSize )
{
	if (!fgets( Buff, BuffSize, pf ))
	{
		return NULL;
	}
	int NonTerminals = strcspn( Buff, "\r\n\x1a" );
	Buff[NonTerminals] = '\0';
	return Buff;
}

// Find FIRST index from ,num,num, list which is NOT the specified value
// and the greater of
// (minimum of those greater than exclude)
// (first of those less than exclude). -1 if none
static int FindOtherIndex( LPCTSTR list, int exclude, int& numFound, int* returnList, int returnListSize )
{
	CString sz( list );
	numFound = 0;
	if (sz.IsEmpty())
	{
		return -1;
	}
	char temp[256];
	strncpy( temp, list, sizeof(temp) );
	temp[sizeof(temp)-1] = '\0';
	char *num = strtok( temp, "," );
	int candidateLess = -1;
	int candidateMore = -1;
	while (num != NULL)
	{
		int n = atoi( num );
		if (n != exclude)
		{
			if (numFound < returnListSize)
			{
				returnList[numFound] = n;
			}
			numFound++;
			if (n > exclude)
			{
				if (n > candidateMore)
				{
					candidateMore = n;
				}
			}
			else
			{
				if (candidateLess == -1)
				{
					candidateLess = n;
				}
			}
		}
		num = strtok( NULL, "," );
	}
	return max( candidateLess, candidateMore );
}

// Create canonical edge key with vertices graded by ordinal
static CString EdgeKey( int v1, int v2 )
{
	CString s;
	static char *EDGE_NAME_FMT = "%d_%d";
	if (v1 < v2)
	{
		s.Format( EDGE_NAME_FMT, v1, v2 );
	}
	else
	{
		s.Format( EDGE_NAME_FMT, v2, v1 );
	}
	return s;
}

struct tIndexMap {
	int vi[4][2];
	int fi;
	int clockwise; // 1 for clockwise, 0 for counter-clockwise, -1 for unset
	CFace* pf;
};
typedef struct tIndexMap vindexMap_t;

// Global refs to error logging info
static CString dummy;
static int dummy2;
static CString& s_errorLog = dummy;
static int& s_errorCount = dummy2;
static int& s_warningCount = dummy2;

// Respecify face
static void SetFace( int fi, ThreePlane* face, ThreePoint* v, int &vcount, int v1, int v2, int v3, int v4 )
{
	face[fi].vtx(0) = v[v1];
	face[fi].vtx(1) = v[v2];
	face[fi].vtx(2) = v[v3];
	if (vcount > 3)
	{
		face[fi].AddVertex( v[v4] );
	}
	face[fi].Calc();

	CString szTemp;

	// Check for zero-length sides and eliminate iff we have four faces
	int zeroLengthCount = 0;
	int firstBadFace;
	int nv;
	for (nv = 0; nv < vcount; nv++)
	{
		if (face[fi].GetSeg(nv) == 0.0)
		{
			if (zeroLengthCount == 0)
			{
				firstBadFace =0;
			}
			zeroLengthCount++;
		}
	}
	if (zeroLengthCount == 1 && vcount == 4)
	{
		switch (firstBadFace)
		{
		case 0:
			v1 = v2; v2 = v3; v3 = v4;
			break;
		case 1:
			v2 = v3; v3 = v4;
			break;
		case 2:
			v3 = v4;
			break;
		}
		vcount--;
		face[fi].Reset();
		face[fi].vtx(0) = v[v1];
		face[fi].vtx(1) = v[v2];
		face[fi].vtx(2) = v[v3];
		face[fi].Calc();
		szTemp.Format( "Face %d segment %d has length 0 - eliminating nonexistent leg!\r\n", fi, firstBadFace );
		s_errorLog += szTemp;
		s_warningCount++;
	}
	else if (zeroLengthCount > 0)
	{
		szTemp.Format( "Face %d has %d zero-length segments - cannot continue!\r\n", fi, zeroLengthCount );
		s_errorLog += szTemp;
		s_errorCount++;
	}

}

// Flip a single entry and respecify face
static void Flip( int fi, ThreePlane* face, ThreePoint* v, tIndexMap& vmap )
{
	// Current order is based on v1 v2 v3 [v4] which are vertex indices
	// It will flip to v1 [v4] v3 v2
	// We store vertices as
	// v1 v2
	// v2 v3
	// v3 v1
	// or for four vertices
	// v1 v2
	// v2 v3
	// v3 v4
	// v4 v1
	// and we need to flip to
	// v1 v3
	// v3 v2
	// v2 v1
	// or for four vertices
	// v1 v4
	// v4 v3
	// v3 v2
	// v2 v1
	int v1, v2, v3, v4;
	v4 = 0;
	v1 = vmap.vi[0][0];
	int vcount = face[fi].GetNumVertices();
	int vLast = vcount - 1;
	// Establish the new identities. v1 remains unchanged.
	v2 = vmap.vi[vLast][0];
	v3 = vmap.vi[vLast-1][0];
	if (vcount > 3)
	{
		v4 = vmap.vi[vLast-2][0];
	}
	vmap.vi[0][0] = v1;
	vmap.vi[0][1] = v2;
	vmap.vi[1][0] = v2;
	vmap.vi[1][1] = v3;
	vmap.vi[2][0] = v3;
	vmap.vi[2][1] = v1;
	if (vcount > 3)
	{
		vmap.vi[2][1] = v4;
		vmap.vi[3][0] = v4;
		vmap.vi[3][1] = v1;
	}

	face[fi].Reset();
	SetFace( fi, face, v, vcount, v1, v2, v3, v4 );

}

// Import shape from OFF. Returns number of faces in shape or 0 if error, with errors logged to errorLog
int CPaperCutDoc::ImportOFF( LPCTSTR file, CString& errorLog, int& warningCount, int& errorCount )
{
	errorLog.Empty();
	warningCount = 0;
	errorCount = 0;

	// Set global references
	s_errorLog = errorLog;
	s_warningCount = warningCount;
	s_errorCount = errorCount;

	// Open file and read OFF header:
	/*****
	Note that importing OFF is tricky when it comes to creating a papercut shape.
	Papercut assumes that the winding order of edges on every face is consistent 
	with respect to the topological "outside" of the shape.
	We take the mean of all x, y and z values as a centerpoint (the vertex epicenter),
	and determine the predominant orientation, then apply that to one face.
	We propagate that orientation to all faces.

	Example file cube.off (in test dir):
OFF
8 6 0
1.000000 1.000000 -1.000000
1.000000 -1.000000 -1.000000
-1.000000 -1.000000 -1.000000
-1.000000 1.000000 -1.000000
1.000000 0.999999 1.000000
0.999999 -1.000001 1.000000
-1.000000 -1.000000 1.000000
-1.000000 1.000000 1.000000
4 3 2 1 0 
4 5 6 7 4 
4 1 5 4 0 
4 2 6 5 1 
4 3 7 6 2 
4 7 3 0 4 
	(end example)
	*****/
	char szBuff[512];
	FILE *fInput = fopen( file, "rt" );
	if (fInput == NULL)
	{
		errorLog.Format( "Error: cannot open file %s", file );
		errorCount++;
		return 0;
	}

	// Start errorlog with header
	errorLog.Format( "Reading from %s\r\n", file );

	int retVal = 0;
	bool validFile = true;

	// Read header and check for valid OFF
	if (ReadLine( fInput, szBuff, sizeof( szBuff ) ) == NULL)
	{
		validFile = false;
		errorCount++;
		errorLog += "Error: could not read OFF header\r\n";
	}
	if (validFile && strcmp( szBuff, "OFF" ) != 0)
	{
		validFile = false;
		errorCount++;
		errorLog += "Error: not a valid OFF file, expected OFF, got ";
		errorLog.Append( szBuff, 3 );
		errorLog += " in header\r\n";
	}
	// Get vertex and face count (ignore third parameter)
	if (validFile && ReadLine( fInput, szBuff, sizeof( szBuff ) ) == NULL)
	{
		validFile = false;
		errorCount++;
		errorLog += "Error: unable to read header2\r\n";
	}
	int vertexCount, faceCount, fieldCount;
	fieldCount = sscanf( szBuff, "%d %d", &vertexCount, &faceCount );
	if (validFile && (fieldCount < 2 || vertexCount < 3 || faceCount < 1))
	{
		validFile = false;
		errorCount++;
		CString szTemp;
		szTemp.Format( "Error: header parameters invalid (%d,%d,%d)\r\n", fieldCount, vertexCount, faceCount );
		errorLog += szTemp;
	}

	// Allocate vertices
	ThreePoint *v;
	ThreePlane *face;
	try
	{
		v = new ThreePoint[vertexCount];
		face = new ThreePlane[faceCount];
	}
	catch (...)
	{
		validFile = false;
		errorCount++;
		CString szTemp;
		szTemp.Format( "Error: allocate failed for %u vertices, %u faces (file is too big or invalid)\r\n", vertexCount, faceCount );
		errorLog += szTemp;
		fclose( fInput );
		return 0;
	}

	// Allocate map for vertex matching. We grade each vertex index pair since
	// OFF does not specify a canonical winding order
	// Initialize with -1 since not all faces have 4 vertices
	vindexMap_t *vindexMap = new vindexMap_t[faceCount];
	memset( vindexMap, -1, faceCount * sizeof( vindexMap_t ) );
	CString szTemp;

	// Read vertices
	int n;
	for (n = 0; n < vertexCount && validFile; n++)
	{
		if (ReadLine( fInput, szBuff, sizeof( szBuff ) ) == NULL)
		{
			szTemp.Format( "Error: unexpected end of file reading vertex %d\r\n", n+1 );
			errorCount++;
			errorLog += szTemp;
			validFile = false;
			break;
		}
		// Get x, y, and z
		double x, y, z;
		if (sscanf( szBuff, "%lf %lf %lf", &x, &y, &z ) < 3)
		{
			szTemp.Format( "Error: could not parse coordinates from vertex %d (%s)\r\n", n+1, szBuff );
			errorLog += szTemp;
			errorCount++;
			validFile = false;
			break;
		}
		v[n].Set( x, y, z );
	}

	// Read faces
	for (n = 0; n < faceCount && validFile; n++)
	{
		if (ReadLine( fInput, szBuff, sizeof( szBuff ) ) == NULL)
		{
			szTemp.Format( "Error: unexpected end of file reading face %d\r\n", n+1 );
			errorLog += szTemp;
			errorCount++;
			validFile = false;
			break;
		}
		// We only handle three or four vertices
		// Format is <vertex count> <v1> <v2> <v3> [<v4>]
		int vcount, v1, v2, v3, v4;
		fieldCount = sscanf( szBuff, "%d %d %d %d %d", &vcount, &v1, &v2, &v3, &v4 );
		if (fieldCount < 3 || fieldCount != vcount + 1)
		{
			szTemp.Format( "Error: invalid field count reading face %d\r\n", n+1 );
			errorLog += szTemp;
			errorCount++;
			validFile = false;
			break;
		}
		if (vcount > 4)
		{
			szTemp.Format( "Error: unsupported vertex count for face %d - max vertices = 4\r\n", n+1 );
			errorLog += szTemp;
			errorCount++;
			validFile = false;
			break;
		}
		// Check for invalid indices
		if (v1 < 0 || v2 < 0 || v3 < 0 || (vcount>3 && v4 < 0))
		{
			szTemp.Format( "Error: negative index(es) for face %d (%s)\r\n", n+1, szBuff );
			errorLog += szTemp;
			errorCount++;
			validFile = false;
			break;
		}
		if (v1 >= vertexCount || v2 >= vertexCount || v3 >= vertexCount || (vcount>3 && v4 >= vertexCount))
		{
			szTemp.Format( "Error: out of bounds index(es) for face %d (%s)\r\n", n+1, szBuff );
			errorLog += szTemp;
			errorCount++;
			validFile = false;
			break;
		}
		// vcount may be adjusted downwards if we have a zero-length side
		SetFace( n, face, v, vcount, v1, v2, v3, v4 );
		vindexMap[n].fi = n;
		vindexMap[n].pf = NULL;
		vindexMap[n].vi[0][0] = v1;
		vindexMap[n].vi[0][1] = v2;
		vindexMap[n].vi[1][0] = v2;
		vindexMap[n].vi[1][1] = v3;
		vindexMap[n].vi[2][0] = v3;
		vindexMap[n].vi[2][1] = v1;
		if (vcount > 3)
		{
			vindexMap[n].vi[2][1] = v4;
			vindexMap[n].vi[3][0] = v4;
			vindexMap[n].vi[3][1] = v1;
		}
		// FIXME determine shape topology as described in comments at start of function
		// FIXME for now, we'll just assume the first face is clockwise and propagate that
		// FIXME orientation to all other faces.
		if (n == 0)
		{
			vindexMap[n].clockwise = 1;
		}
	}

	// Done with file
	fclose( fInput );

	// Create shape
	if (validFile && errorCount == 0)
	{
		int n2;
		CMap<CString,LPCTSTR,CString,LPCTSTR> mapEdgeToFace;

		// Normalize edge lengths so that avg is 1
		double segmentSum = 0.0;
		int segmentCount = 0;
		for (n = 0; n < faceCount; n++)
		{
			segmentCount += face[n].GetNumVertices();
			segmentSum += face[n].GetSumOfSegs();
		}
		double segmentMean = segmentSum / segmentCount;
		szTemp.Format( "Info: mean of %d faces with a total of %d segments is length %.3f\r\n",
			faceCount, segmentCount, segmentMean );
		errorLog += szTemp;
		if ((segmentMean < 0.5 || segmentMean > 5.0) && segmentMean > 0.0)
		{
			double scaleFactor = 1.0 / segmentMean;
			// Get confirmation - user may be importing several shapes which must be kept to scale
			DlgInputScale dlgConfirm;
			szTemp.Format( "%0.6f", scaleFactor );
			dlgConfirm.SetText( szTemp );
			if (dlgConfirm.DoModal() == IDOK)
			{
				scaleFactor = atof( dlgConfirm.GetText() );
				if (scaleFactor > 0.0)
				{
					for (n = 0; n < faceCount; n++)
					{
						face[n].Scale( scaleFactor );
					}
				}
				else
				{
					errorLog += "Warning: scaling was indicated by segment range but scale of 0 was entered - no scaling performed!\r\n";
					warningCount++;
				}
			}
		}

		// vindexMap[] is currently one entry per face with vertex indices.
		// The vertex1_vertex2 key generated by EdgeKey() is the index to a ,face1,face2, list
		for (n = 0; n < faceCount; n++)
		{
			CString edgeName, faceNum, faceList;
			faceNum.Format( "%d,", n );
			for (n2 = 0; n2 < 4; n2++)
			{
				if (vindexMap[n].vi[n2][0] == -1) 
				{
					continue;
				}
				// Construct map index, always with the lesser index first
				edgeName = EdgeKey( vindexMap[n].vi[n2][0], vindexMap[n].vi[n2][1] );
				if (!mapEdgeToFace.Lookup( edgeName, faceList ))
				{
					faceList = ",";
				}
				faceList += faceNum;
				mapEdgeToFace.SetAt( edgeName, faceList );
			}
		}

		// Normalize orientation
		for (n = 0; n < faceCount; n++)
		{
			// Can't do anything if orientation is not set
			if (vindexMap[n].clockwise < 0)
			{
				continue;
			}
			int otherFaceIndex;
			CString edgeName;
			for (n2 = 0; n2 < face[n].GetNumVertices(); n2++)
			{
				edgeName = EdgeKey( vindexMap[n].vi[n2][0], vindexMap[n].vi[n2][1] );
				int numFound;
				int faceIndexList[16];
				if (FindOtherIndex( mapEdgeToFace[edgeName], n, numFound, faceIndexList, sizeof(faceIndexList)/sizeof(faceIndexList[0]) ) < 0)
				{
					continue;
				}
				// Propagate orientation to all connected faces
				int n3;
				for (n3 = 0; n3 < numFound; n3++)
				{
					otherFaceIndex = faceIndexList[n3];
					if (vindexMap[otherFaceIndex].clockwise < 0)
					{
						// Copy orientation
						vindexMap[otherFaceIndex].clockwise = vindexMap[n].clockwise;
						// Find the shared edge vertices. Note that the pair arrangement
						// of vertices for edge definitions also corresponds to the winding
						// order of vertices.
						int sharedVertex0 = vindexMap[n].vi[n2][0];
						int sharedVertex1 = vindexMap[n].vi[n2][1];
						int sn2;
						int flipState = -1; // Unset
						for (sn2 = 0; sn2 < face[otherFaceIndex].GetNumVertices(); sn2++)
						{
							if (vindexMap[otherFaceIndex].vi[sn2][0] == sharedVertex1 &&
								vindexMap[otherFaceIndex].vi[sn2][1] == sharedVertex0)
							{
								// Orientation of same-winding segments is flipped, so we're ok
								flipState = 0;
								break;
							}
							if (vindexMap[otherFaceIndex].vi[sn2][0] != sharedVertex0 ||
								vindexMap[otherFaceIndex].vi[sn2][1] != sharedVertex1)
							{
								// not a match
								continue;
							}
							flipState = 1;
							break;
						}
						// If flipState == 1, we need to flip
						// if flipState == -1, report it as a warning
						if (flipState == -1)
						{
							szTemp.Format( "Warning: unable to find matching vertices for face %d edge %s in face %d\r\n", 
									n, (LPCTSTR)edgeName, otherFaceIndex );
							errorLog += szTemp;
							warningCount++;
							vindexMap[otherFaceIndex].clockwise = -1;
						}
						else if (flipState == 1)
						{
							szTemp.Format( "Info: flipping orientation of face %d to clockwise=%d\r\n", 
									otherFaceIndex, vindexMap[n].clockwise ^ 1 );
							errorLog += szTemp;
							vindexMap[otherFaceIndex].clockwise ^= 1;
							Flip( otherFaceIndex, face, v, vindexMap[otherFaceIndex] );
						}
					}
				}
			}
		}

		// Create shape
		this->m_Shape.ClearAllFaces();
		// Add faces individually
		CString faceName;
		CString edgeName[4];
		LPCTSTR edgeNames[4];
		double edgeLength[4];
		double angle[4];
		int numEdges;
		int nEdge, edgeIndex;
		for (n = 0; n < faceCount; n++)
		{
			// This was done when we imported
			//// Calculate angles and sides
			//face[n].Calc();
			// Set up names
			faceName.Format( "%02d", n+1 );
			numEdges = face[n].GetNumVertices();
			int nextEdgeIndex = 0;
			for (nEdge = edgeIndex = 0; nEdge < numEdges; nEdge++, edgeIndex = nextEdgeIndex)
			{
				// Get next index
				nextEdgeIndex = face[n].GetNextComponentIndex( edgeIndex );
				// Edge names already have the vertex indices graded ascendingly
				// and edges themselves are in winding order (no crossovers).
				// However, edge names may not correspond to vertex placement
				// consistently.
				edgeName[edgeIndex] = EdgeKey( vindexMap[n].vi[edgeIndex][0], vindexMap[n].vi[edgeIndex][1] );
				edgeNames[edgeIndex] = edgeName[edgeIndex];
				edgeLength[edgeIndex] = face[n].GetSeg( edgeIndex );
				// Angles passed to CreatePolygon() follow legs...
				angle[edgeIndex] = face[n].GetAngleDeg( nextEdgeIndex );
			}
			// FIXME find a sensible way to set base edges
			int baseIndex = 0;
			vindexMap[n].pf = m_Shape.AddFace( faceName, numEdges, edgeLength, angle, edgeNames, edgeNames[baseIndex] );
		}

		// Finalize adding faces
		this->m_Shape.Finalize();

		// Join edges based on vertex index matching
		// We have a map with key [vindex1-vindex2] and value ",faceindex,faceindex,"
		//CDbg::m_Level++; ///< FIXME Remove this
		for (n = 0; n < faceCount; n++)
		{
			int otherFaceIndex;
			CString edgeName;
			for (n2 = 0; n2 < face[n].GetNumVertices(); n2++)
			{
				edgeName = EdgeKey( vindexMap[n].vi[n2][0], vindexMap[n].vi[n2][1] );
				// Note that we may have three or more planes sharing a common edge.
				// We may also have one segment joined to two or more.
				// PaperCut doesn't currently support this, but we'll return the first
				// other face greater than n.
				int numFound;
				int faceIndexList[16];
				otherFaceIndex = FindOtherIndex( mapEdgeToFace[edgeName], n, numFound, faceIndexList, sizeof(faceIndexList)/sizeof(faceIndexList[0]) );
				if (otherFaceIndex == -1)
				{
					warningCount++;
					szTemp.Format( "Warning: could not find other face index for face[%d]=%s list=%s numfound=%d edge=%s\r\n", 
						n,
						(LPCTSTR)vindexMap[n].pf->m_szFaceName,
						(LPCTSTR)mapEdgeToFace[edgeName],
						numFound,
						(LPCTSTR)edgeName ); 
					errorLog += szTemp;
					continue;
				}
				if (otherFaceIndex < n)
				{
					szTemp.Format( "Info: Skipping join for predecessor face %s:%s (index %d, current index %d, list=%s)\r\n",
						(LPCTSTR)vindexMap[otherFaceIndex].pf->m_szFaceName,
						(LPCTSTR)edgeName,
						otherFaceIndex, n,
						(LPCTSTR)mapEdgeToFace[edgeName] );
					CDbg::Out( szTemp );
					// Join predecessor iff it is not already joined
					if (vindexMap[otherFaceIndex].pf->GetEdge(edgeName) != NULL)
					{
						// This will be shown iff there are other warnings or errors
						errorLog += szTemp;
						continue;
					}
				}
				if (numFound > 1)
				{
					szTemp.Format( "Warning: found %d other faces for face[%d]:edge %s:%s list=%s more than two faces joined is not supported\r\n",
							numFound,
							n,
							(LPCTSTR)vindexMap[otherFaceIndex].pf->m_szFaceName,
							(LPCTSTR)edgeName,
							(LPCTSTR)mapEdgeToFace[edgeName]
							);
					errorLog += szTemp;
					warningCount++;
					continue;
				}
				szTemp.Format( "Info: OFF imp: join %s:%s-%s:%s (indices %d,%d)\r\n",
					(LPCTSTR)vindexMap[n].pf->m_szFaceName, (LPCTSTR)edgeName,
					(LPCTSTR)vindexMap[otherFaceIndex].pf->m_szFaceName, (LPCTSTR)edgeName,
					n, otherFaceIndex );
				CDbg::Out( szTemp );
				// This will be displayed iff there are other errors/warnings
				errorLog += szTemp;
				vindexMap[n].pf->JoinFaceEdge( edgeName, vindexMap[otherFaceIndex].pf, edgeName );
			}
		}
		//CDbg::m_Level--; ///< FIXME remove this
		// Return number of faces
		retVal = faceCount;
	}

	// Clean up
	delete [] v;
	delete [] face;
	delete [] vindexMap;

	return retVal;
}

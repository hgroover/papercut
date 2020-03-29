/*!

*/
// BumpBuild.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


char szBuff[ 16384 ];
#define	MAXLINELENGTH	(sizeof(szBuff)-1)

void ShowSyntax( const char *errorMessage = "" )
{
		printf( "%sSyntax: bumpbuild [options] filename tokenname\n"
				"  where filename is the .h file containing a token to increment\n"
				"  and a token in the form #define token value is defined with\n"
				"    an integer value assigned.\n", errorMessage );
		printf( "Valid options:\n"
				"	--refresh	Refresh timestamp but do not increment versions\n" );
}

int main(int argc, char* argv[])
{
	printf( "BumpBuild v1.02 Copyright (C) 2002-2006 Henry A. Groover  All rights reserved\n" );
	char *fileName = NULL;
	char *tokenName = NULL;
	bool refreshOnly = false;
	if (argc < 3)
	{
		ShowSyntax();
		return -1;
	}

	// Process options
	int n;
	for (n = 1; n < argc; n++)
	{
		// Check for options
		if (argv[n][0] == '-')
		{
			if (!stricmp( argv[n], "--refresh" ))
			{
				printf( "Version number will not be incremented - only refreshing timestamp.\n" );
				refreshOnly = true;
			}
			else
			{
				ShowSyntax( "Unexpected option\n" );
				return -1;
			}
		} // option
		else
		{
			if (fileName == NULL)
			{
				fileName = argv[n];
			}
			else if (tokenName == NULL)
			{
				tokenName = argv[n];
			}
			else
			{
				ShowSyntax( "Warning: ignoring extra argument %s\n" );
			}
		} // filename or token
	}

	// Check for required values
	if (fileName == NULL)
	{
		ShowSyntax( "Error: filename not specified\n" );
		return -1;
	}
	if (tokenName == NULL)
	{
		ShowSyntax( "Error: token name and filename not specified\n" );
		return -1;
	}

	FILE *pf = fopen( fileName, "rt" );
	if (!pf)
	{
		printf( "Error: cannot open %s\n", fileName );
		return -1;
	}

	char szOutFileName[ 256 ];
	sprintf( szOutFileName, "%s.tmp", fileName );
	FILE *pfOut = fopen( szOutFileName, "wt" );
	if (!pfOut)
	{
		printf( "Error: cannot open temporary file %s\n", szOutFileName );
		fclose( pf );
		return -1;
	}

	printf( "Reading %s, scanning for #define.*%s...\n", fileName, tokenName );
	int nLine = 1;
	static const char *pszTimestamp = "BUILD_TIMESTAMP";
	while (fgets( szBuff, MAXLINELENGTH, pf ))
	{
		char *pszDef = strstr( szBuff, "#define" );
		if (pszDef == szBuff)
		{
			char *pszTarget = strstr( &szBuff[7], tokenName );
			if (pszTarget)
			{
				// Skip to actual number
				pszTarget += strlen( tokenName );
				// Skip whitespace
				pszTarget += strspn( pszTarget, " \t" );
				unsigned long lValue = atol( pszTarget );
				if (refreshOnly)
				{
					printf( "Found %s at line %d: leaving value %lu unchanged (--refresh specified)\n", tokenName, nLine, lValue );
				}
				else
				{
					lValue++;
					printf( "Found %s at line %d: new value %lu\n", tokenName, nLine, lValue );
					// Rewrite value and line termination
					sprintf( pszTarget, "%lu\n", lValue );
				}
			} // Found it
			else {
				if (pszTarget = strstr( &szBuff[7], pszTimestamp ))
				{
					// Skip past token name
					pszTarget += strlen( pszTimestamp );
					// Skip whitespace
					pszTarget += strspn( pszTarget, " \t" );
					// Blast in current time
					time_t now = time( NULL );
					sprintf( pszTarget, "%lu\n", now );
					char szTime[256];
					struct tm* p = localtime( &now );
					strftime( szTime, sizeof( szTime ), "%x %X", p );
					printf( "Rewrote %s as %s (%lu)\n", pszTimestamp, szTime, now );
				} // Found BUILD_TIMESTAMP
			} // Check for BUILD_TIMESTAMP
		} // Found a #define at the start
		fputs( szBuff, pfOut );
		nLine++;
	} // not eof

	printf( "Done, %d lines processed\n", nLine );

	fclose( pf );
	fclose( pfOut );

	printf( "Deleting %s\n", fileName );
	if (unlink( fileName ) != 0)
	{
		printf( "Error: failed to delete!\n" );
		return -1;
	}
	printf( "Renaming %s to %s\n", szOutFileName, fileName );
	if (rename( szOutFileName, fileName ) != 0)
	{
		printf( "Error: unable to rename!\n" );
		return -1;
	}

	printf( "Getting file timestamp...\n" );
	HANDLE hFile = ::CreateFile(
		fileName,
		GENERIC_READ | GENERIC_WRITE,
		0 /* no sharing */,
		NULL /* no security */,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL );

	if (hFile == INVALID_HANDLE_VALUE)
	{
		printf( "Error: cannot open!\n" );
	}
	else {
		FILETIME ftAccess, ftWrite;
		if (::GetFileTime( hFile, NULL, &ftAccess, &ftWrite ))
		{
			LARGE_INTEGER li;
			li.LowPart = ftWrite.dwLowDateTime;
			li.HighPart = ftWrite.dwHighDateTime;
			// Subtract two seconds
			//       nanoseconds*100
			//         microseconds
			//      milliseconds
			//        seconds.......
			li.QuadPart -=  20000000;
			ftWrite.dwLowDateTime = li.LowPart;
			ftWrite.dwHighDateTime = li.HighPart;
			::SetFileTime( hFile, NULL, &ftWrite, &ftWrite );
			printf( "Changed file time successfully to now - 2 seconds\n" );
		} // Got access time
		::CloseHandle( hFile );
	}

	printf( "Done.\n" );
	return 0;
}

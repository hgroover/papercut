// $Id: PaperCutDeploy.cpp 15 2006-03-30 15:34:42Z henry_groover $
// PaperCutDeploy.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "../papercut/versiondef.h"

char szBuildVer[] = VER_STR;
int nBuildNumber = BUILD_NUM;
time_t BuildTimestamp = BUILD_TIMESTAMP;

// Deploy version
char szDeployVersion[] = "1.0.20071028.2215";
size_t BuffSize = 0x40000;
char *CopyBuff = NULL;

// Required file constants - must index aszFiles[]
enum {
	FIRST_REQUIRED_FILE=0,
	// Start required file list
	// These entries must be accessible from the start directory
	SOURCE_MSI=FIRST_REQUIRED_FILE,
	SOURCE_EXE,
	SOURCE_README,
	SOURCE_ZIPLIST,
	// End required file list
	NUM_REQUIRED_FILES,
	EXE_7ZIP,
	SFX_7ZIP,
	PATH_7ZIP,
	DEST_BASEPATH,
	DEST_COMMENT,
	DEST_MSI,
	DEST_README,
	DEST_SOURCEZIP,
	SOURCE_RELEASEDIR,
	CWD_FROM_RELEASE,
	TEMP_ARCHIVE,
	DEST_EXE,
	// Miscellaneous pairs of files to copy
	SOURCE_SHAPEFORMAT,
	DEST_SHAPEFORMAT,
	SOURCE_TODO,
	DEST_TODO
};
char *aszFiles[] = {
	"PaperCutSetup/Release/PapercutSetup.msi",
	"PaperCut/Release/PaperCut.exe",
	"PaperCut/readme.txt",
	"zip.list",
	"", // Unused - NUM_REQUIRED_FILES
	"7z.exe",
	"7zs.sfx",
	"c:/program files/7-zip/",
	"../../web/papercut-home/release/",
	"comment.txt",
	"setup.msi",
	"readme.txt",
	"source.zip",
	"PapercutSetup/Release",
	"..\\..",
	"papercut.7z",
	"setup.exe", // DEST_EXE
	"PaperCut/ShapeFileFormat.txt",
	"../../web/papercut-home/software/ShapeFileFormat.txt",
	"PaperCut/ToDo.txt",
	"../../web/papercut-home/ToDo.txt"
};

// sprintf() template for sfx.cfg
char *szSfxCfg_siii = ";!@Install@!UTF-8!\r\n\
Title=\"Papercut Setup self-extractor\"\r\n\
BeginPrompt=\"Do you want to install Papercut %s %d.%d.%d ?\"\r\n\
RunProgram=\"Setup.exe\"\r\n\
;!@InstallEnd@!\r\n";


// Copy file from source to dest, return bytes copied or -1 if error
int LCopyFile( char *szSource, char *szDest )
{
	FILE *pfSource;
	FILE *pfDest;
	if (CopyBuff == NULL)
	{
		printf( "Error: Copy buffer not allocated!\n" );
		return -1;
	}
	pfSource = fopen( szSource, "rb" );
	if (pfSource == NULL)
	{
		printf( "Error: unable to open %s\n", szSource );
		return -1;
	}
	pfDest = fopen( szDest, "wb" );
	if (pfDest == NULL)
	{
		fclose( pfSource );
		printf( "Error: unable to open %s\n", szDest );
		return -1;
	}
	int BytesCopied = 0;
	size_t BytesRead;
	printf( "%s -> %s", szSource, szDest );
	while (!feof( pfSource ))
	{
		BytesRead = fread( CopyBuff, 1, BuffSize, pfSource );
		fwrite( CopyBuff, 1, BytesRead, pfDest );
		BytesCopied += BytesRead;
	}
	fclose( pfSource );
	fclose( pfDest );
	printf( " (%d bytes copied)\n", BytesCopied );
	return BytesCopied;
}

int _tmain(int argc, _TCHAR* argv[])
{
	printf( "PaperCutDeploy version %s\n", szDeployVersion );
	printf( "PaperCut version: %s.%d\n", szBuildVer, nBuildNumber );
	int VerMajor, VerMinor;
	if (sscanf( VER_STR, "%d.%d", &VerMajor, &VerMinor ) < 2)
	{
		printf( "Error: unable to parse version major and minor from %s\n", VER_STR );
		return -1;
	}

	// Sanity check for required files
	// Groucho: there ain't no sanity clause...
	int nRequired;
	printf( "Checking for required files...\n" );
	for (nRequired = FIRST_REQUIRED_FILE; nRequired < NUM_REQUIRED_FILES; nRequired++)
	{
		printf( "\t[%d] %s... ", nRequired, aszFiles[nRequired] );
		if (_access( aszFiles[nRequired], 00 ) != 0)
		{
			printf( "Error: missing required file %s\n", aszFiles[nRequired] );
			return -1;
		}
		printf( "OK\n" );
	}

	FILE *pf;
	char *szFile;
	// Check for PaperCutSetup\Release\PapercutSetup.msi
	printf( "Checking file timestamps... " );
	szFile = aszFiles[SOURCE_MSI];
	pf = fopen( szFile, "rb" );
	if (pf == NULL)
	{
		printf( "Error: cannot open %s\n", szFile );
		return -1;
	}
	struct _stat MsiStat;
	int fh = _fileno( pf );
	if (_fstat( fh, &MsiStat ) != 0)
	{
		fclose( pf );
		printf( "Error: cannot stat %s\n", szFile );
		return -1;
	}
	// Compare MsiStat.st_mtime to exe
	fclose( pf );
	// Check for PaperCut\Release\PaperCut.exe
	struct _stat ExeStat;
	szFile = aszFiles[SOURCE_EXE];
	pf = fopen( szFile, "rb" );
	if (pf == NULL)
	{
		printf( "Error: cannot access %s\n", szFile );
		return -1;
	}
	if (_fstat( _fileno( pf ), &ExeStat ) != 0)
	{
		fclose( pf );
		printf( "Error: cannot stat %s\n", szFile );
		return -1;
	}
	fclose( pf );

	// Make sure msi is newer than exe
	if (MsiStat.st_mtime < ExeStat.st_mtime)
	{
		int Diff = ExeStat.st_mtime - MsiStat.st_mtime;
		printf( "Error: exe is newer than msi by %d seconds\n", Diff );
		return -1;
	}
	float dDiff = (MsiStat.st_mtime - ExeStat.st_mtime) / 60.0;
	printf( "OK - %.2f minutes difference\n", dDiff ); 

	// Check for c:\program files\7-zip\7z.exe
	printf( "Checking for 7-zip required files... " );
	char sz7Zip[256];
	char sz7ZipExe[256];
	strcpy( sz7Zip, aszFiles[PATH_7ZIP] );
	strcpy( sz7ZipExe, sz7Zip );
	char *psz7ZipFile = sz7Zip;
	psz7ZipFile += strlen( sz7Zip );
	strcpy( psz7ZipFile, aszFiles[EXE_7ZIP] );
	if (_access( szFile, 00 ) != 0)
	{
		printf( "Error: 7-zip doesn't appear to be installed in %s\n", sz7Zip );
		return -1;
	}

	// We also need 7zs.sfx in the same directory and PapercutSetup\PaperCutSFX.cfg
	strcpy( psz7ZipFile, aszFiles[SFX_7ZIP] );
	if (_access( sz7Zip, 00 ) != 0)
	{
		printf( "Error: cannot find %s\n", sz7Zip );
		return -1;
	}
	printf( "OK\n" );

	// Check for PapercutSetup/PapercutSFX.cfg
	/************
	** no longer used **
	szFile = "PapercutSetup/PapercutSFX.cfg";
	if (_access( szFile, 00 ) != 0)
	{
		printf( "Error: cannot access %s\n", szFile );
		return -1;
	}
	************/

	// Check for PaperCut\readme.txt
	printf( "Checking for version-specific comments... " );
	szFile = aszFiles[SOURCE_README];
	pf = fopen( szFile, "rt" );
	if (pf == NULL)
	{
		printf( "Error: unable to open %s\n", szFile );
		return -1;
	}

	// Parse PaperCut\Readme.txt for version comments.
	// Skip leading whitespace and look for the first line starting with szBuildVer.nBuildNumber
	// e.g. 1.00.284 followed by whitespace and/or EOL,
	// or 1.01.001
	// Any non-whitespace lines not starting with + constitute the version comment, e.g.
	// This version has been tested on Win98, but who cares?
	char szComment[4096] = { 0 };
	char szBuff[1024];
	char *lpStart;
	char szCompare[64];
	// szBuildVer will be "1.00" or "1.01"
	sprintf( szCompare, "%s.%03d", szBuildVer, nBuildNumber );
	int InVersion = 0;
	printf( "Searching for %s\n", szCompare );
	while (fgets( szBuff, sizeof( szBuff ), pf) != NULL)
	{
		// Skip leading whitespace
		lpStart = szBuff + strspn( szBuff, " \t\r\n" );

		// Clobber trailing LF
		char *pszEnd = strpbrk( lpStart, "\r\n" );
		if (pszEnd != NULL) *pszEnd = '\0';

		// If reading version, check for +
		if (InVersion)
		{
			if (*lpStart == '+')
			{
				break;
			}
			if (*lpStart && strlen( szComment ) + strlen( lpStart ) < sizeof( szComment ) - 64)
			{
				strcat( szComment, lpStart );
				strcat( szComment, "\r\n" );
			}
		}
		else
		{
			if (*lpStart != '\0')
			{
				if (strncmp( lpStart, szCompare, strlen( szCompare ) ) == 0)
				{
					InVersion = 1;
				}
			}
		}
	}
	fclose( pf );
	if (szComment[0] != '\0')
	{
		printf( "OK - comments follow:\n---[ start comments ]----\n%s===[ end comments ]===\n", szComment );
	}
	else
	{
		printf( "OK - no comments\n" );
	}

	// Now construct base path
	sprintf( szBuff, "%spapercut-%s-%d-%d-%d-", 
			aszFiles[DEST_BASEPATH], 
			asz_build_types[PCUT_RELEASE_TYPE],
			VerMajor, VerMinor, BUILD_NUM );
	char *pszFilename = szBuff;
	pszFilename += strlen( szBuff );

	// If comment exists, create it
	if (szComment[0] != '\0')
	{
		strcpy( pszFilename, aszFiles[DEST_COMMENT] );
		pf = fopen( szBuff, "wb" );
		if (pf != NULL)
		{
			fwrite( szComment, sizeof(char), strlen(szComment), pf );
			fclose( pf );
		}
	}

	printf( "Copying files... ");
	CopyBuff = (char *)malloc( BuffSize );
	if (CopyBuff == NULL)
	{
		printf( "Error: cannot allocate buffer for %d bytes\n", BuffSize );
		return -1;
	}
	// Copy files PapercutSetup.msi and readme.txt
	strcpy( pszFilename, aszFiles[DEST_MSI] );
	if (LCopyFile( aszFiles[SOURCE_MSI], szBuff ) < 0)
	{
		return -1;
	}
	strcpy( pszFilename, aszFiles[DEST_README] );
	if (LCopyFile( aszFiles[SOURCE_README], szBuff ) < 0)
	{
		return -1;
	}
	printf( "OK\n" );

	// Copy ShapeFileFormat.txt
	if (LCopyFile( aszFiles[SOURCE_SHAPEFORMAT], aszFiles[DEST_SHAPEFORMAT] ) < 0)
	{
		return -1;
	}

	// Create archive in current directory
	char szCmd[1024];
	strcpy( psz7ZipFile, aszFiles[EXE_7ZIP] );
	_unlink( aszFiles[TEMP_ARCHIVE] );
	_chdir( aszFiles[SOURCE_RELEASEDIR] );
	sprintf( szCmd, "\"%s\" a %s\\%s *.*", sz7Zip, aszFiles[CWD_FROM_RELEASE], aszFiles[TEMP_ARCHIVE] );
	system( szCmd );
	_chdir( aszFiles[CWD_FROM_RELEASE] );

	// Catenate 7zip sfx installer module + PapercutSetup/PapercutSFX.cfg + archive

	//struct _stat SrcStat, DestStat;
	//_fstat( _fileno( pf ), &SrcStat );
	FILE *pfDest, *pfSrc;
	strcpy( pszFilename, aszFiles[DEST_EXE] );
	pfDest = fopen( szBuff, "wb" );
	if (pfDest == NULL)
	{
		printf( "Error: cannot create %s\n", szBuff );
		free( CopyBuff );
		return -1;
	}
	strcpy( psz7ZipFile, aszFiles[SFX_7ZIP] );
	pfSrc = fopen( sz7Zip, "rb" );
	if (pfSrc == NULL)
	{
		printf( "Error: cannot open %s\n", sz7Zip );
		free( CopyBuff );
		fclose( pfDest );
		return -1;
	}
	size_t ReadSize;
	do
	{
		ReadSize = fread( CopyBuff, 1, BuffSize, pfSrc );
		fwrite( CopyBuff, 1, ReadSize, pfDest );
	} while (!feof( pfSrc ));
	fclose( pfSrc );

	// Write sfx config data
	// Flush before and after, because thou shalt not mix fprintf() / fwrite()...
	fflush( pfDest );
	fprintf( pfDest, szSfxCfg_siii, asz_build_types[PCUT_RELEASE_TYPE], VerMajor, VerMinor, BUILD_NUM );
	fflush( pfDest );

	pfSrc = fopen( aszFiles[TEMP_ARCHIVE], "rb" );
	if (pfSrc == NULL)
	{
		printf( "Error: cannot open %s\n", aszFiles[TEMP_ARCHIVE] );
		free( CopyBuff );
		fclose( pfDest );
		return -1;
	}
	do
	{
		ReadSize = fread( CopyBuff, 1, BuffSize, pfSrc );
		fwrite( CopyBuff, 1, ReadSize, pfDest );
	} while (!feof( pfSrc ));
	fclose( pfSrc );

	fclose( pfDest );

	// Source deployment - create archive using input file
	strcpy( pszFilename, aszFiles[DEST_SOURCEZIP] );
	printf( "Creating source archive %s\n", szBuff );
	strcpy( psz7ZipFile, aszFiles[EXE_7ZIP] );
	sprintf( szCmd, "\"%s\" u -tzip %s @%s", 
		sz7Zip, szBuff, aszFiles[SOURCE_ZIPLIST] );
	printf( "Executing:\n%s\n", szCmd );
	system( szCmd );

	// Clean up
	_unlink( aszFiles[TEMP_ARCHIVE] );

	free( CopyBuff );

	printf( "Deployment complete.\n" );

	return 0;
}


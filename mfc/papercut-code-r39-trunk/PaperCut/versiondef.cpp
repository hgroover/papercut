/*!

	@file	 versiondef.cpp
	
	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: versiondef.cpp 16 2006-04-15 06:39:12Z henry_groover $
*/

#include <time.h>

#include "versiondef.h"

char szBuildVer[] = VER_STR;
int nBuildNumber = BUILD_NUM;
time_t BuildTimestamp = BUILD_TIMESTAMP;

/***********************************************
* Placeholders to force this file to get touched
*
#define	BUILD_NUM	1
#define	BUILD_TIMESTAMP	1193623062

************************************************/
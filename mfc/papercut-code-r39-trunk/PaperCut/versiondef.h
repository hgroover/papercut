/*!

	@file	 versiondef.h
	
	Distributed under Open Software License 3.0 http://www.opensource.org/licenses/osl-3.0.php

  @verbatim

  $Id: versiondef.h 16 2006-04-15 06:39:12Z henry_groover $
*/

#define VER_STR	"1.01"
#define	BUILD_NUM	1
#define	BUILD_TIMESTAMP	1193623062
enum { PCUT_BUILD_ALPHA=0, PCUT_BUILD_BETA, PCUT_BUILD_GAMMA };
#define PCUT_RELEASE_TYPE PCUT_BUILD_ALPHA
static char * asz_build_types[] = {
	"alpha",
	"beta",
	"gamma"
};

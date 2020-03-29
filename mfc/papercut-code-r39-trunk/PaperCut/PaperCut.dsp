# Microsoft Developer Studio Project File - Name="PaperCut" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=PaperCut - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "PaperCut.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PaperCut.mak" CFG="PaperCut - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PaperCut - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "PaperCut - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PaperCut - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O2 /I "..\paintlib\paintlib\common" /I "..\paintlib\paintlib\win\paintlib" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 paintlib.lib pdflib.lib /nologo /subsystem:windows /debug /machine:I386 /libpath:"..\lib\generic" /libpath:"..\lib\release"
# SUBTRACT LINK32 /nodefaultlib

!ELSEIF  "$(CFG)" == "PaperCut - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\paintlib\paintlib\common" /I "..\paintlib\paintlib\win\paintlib" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 paintlib.lib pdflib.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"..\lib\generic" /libpath:"..\lib\debug"

!ENDIF 

# Begin Target

# Name "PaperCut - Win32 Release"
# Name "PaperCut - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\AskShapeExtension.cpp
# End Source File
# Begin Source File

SOURCE=.\ChildFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\Dbg.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgSelectStellation.cpp
# End Source File
# Begin Source File

SOURCE=.\DockDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Edge.cpp
# End Source File
# Begin Source File

SOURCE=.\EditFace.cpp
# End Source File
# Begin Source File

SOURCE=.\Face.cpp
# End Source File
# Begin Source File

SOURCE=.\FaceContent.cpp
# End Source File
# Begin Source File

SOURCE=.\GlobalPreferences.cpp
# End Source File
# Begin Source File

SOURCE=.\LayoutSummaryDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\LogPoint.cpp
# End Source File
# Begin Source File

SOURCE=.\LogRect.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\MultiVertex.cpp
# End Source File
# Begin Source File

SOURCE=.\PageGroup.cpp
# End Source File
# Begin Source File

SOURCE=.\PaperCut.cpp
# End Source File
# Begin Source File

SOURCE=.\hlp\PaperCut.hpj

!IF  "$(CFG)" == "PaperCut - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__PAPER="hlp\AfxCore.rtf"	"hlp\AfxPrint.rtf"	"hlp\$(TargetName).hm"	
# Begin Custom Build - Making help file...
OutDir=.\Release
TargetName=PaperCut
InputPath=.\hlp\PaperCut.hpj
InputName=PaperCut

"$(OutDir)\$(InputName).hlp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	start /wait hcw /C /E /M "hlp\$(InputName).hpj" 
	if errorlevel 1 goto :Error 
	if not exist "hlp\$(InputName).hlp" goto :Error 
	copy "hlp\$(InputName).hlp" $(OutDir) 
	goto :done 
	:Error 
	echo hlp\$(InputName).hpj(1) : error: 
	type "hlp\$(InputName).log" 
	:done 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "PaperCut - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__PAPER="hlp\AfxCore.rtf"	"hlp\AfxPrint.rtf"	"hlp\$(TargetName).hm"	
# Begin Custom Build - Making help file...
OutDir=.\Debug
TargetName=PaperCut
InputPath=.\hlp\PaperCut.hpj
InputName=PaperCut

"$(OutDir)\$(InputName).hlp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	start /wait hcw /C /E /M "hlp\$(InputName).hpj" 
	if errorlevel 1 goto :Error 
	if not exist "hlp\$(InputName).hlp" goto :Error 
	copy "hlp\$(InputName).hlp" $(OutDir) 
	goto :done 
	:Error 
	echo hlp\$(InputName).hpj(1) : error: 
	type "hlp\$(InputName).log" 
	:done 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PaperCut.odl
# End Source File
# Begin Source File

SOURCE=.\PaperCut.rc
# End Source File
# Begin Source File

SOURCE=.\PaperCutDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\PaperCutView.cpp
# End Source File
# Begin Source File

SOURCE=.\Preferences.cpp
# End Source File
# Begin Source File

SOURCE=.\PrefTabsDrawing.cpp
# End Source File
# Begin Source File

SOURCE=.\PrefTabsLayout.cpp
# End Source File
# Begin Source File

SOURCE=.\Shape.cpp
# End Source File
# Begin Source File

SOURCE=.\ShapeLayout.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\versiondef.cpp

!IF  "$(CFG)" == "PaperCut - Win32 Release"

# PROP Intermediate_Dir "Release"
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "PaperCut - Win32 Debug"

# PROP Intermediate_Dir "Debug"
# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Vertex.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\AskShapeExtension.h
# End Source File
# Begin Source File

SOURCE=.\ChildFrm.h
# End Source File
# Begin Source File

SOURCE=.\Dbg.h
# End Source File
# Begin Source File

SOURCE=.\DlgSelectStellation.h
# End Source File
# Begin Source File

SOURCE=.\DockDlg.h
# End Source File
# Begin Source File

SOURCE=.\Edge.h
# End Source File
# Begin Source File

SOURCE=.\EditFace.h
# End Source File
# Begin Source File

SOURCE=.\Face.h
# End Source File
# Begin Source File

SOURCE=.\FaceContent.h
# End Source File
# Begin Source File

SOURCE=.\GlobalPreferences.h
# End Source File
# Begin Source File

SOURCE=.\LayoutSummaryDlg.h
# End Source File
# Begin Source File

SOURCE=.\LogPoint.h
# End Source File
# Begin Source File

SOURCE=.\LogRect.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\MultiVertex.h
# End Source File
# Begin Source File

SOURCE=.\PageGroup.h
# End Source File
# Begin Source File

SOURCE=.\PaperCut.h
# End Source File
# Begin Source File

SOURCE=.\PaperCutDoc.h
# End Source File
# Begin Source File

SOURCE=.\PaperCutView.h
# End Source File
# Begin Source File

SOURCE=.\Preferences.h
# End Source File
# Begin Source File

SOURCE=.\PrefTabsDrawing.h
# End Source File
# Begin Source File

SOURCE=.\PrefTabsLayout.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h

!IF  "$(CFG)" == "PaperCut - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Making help include file...
TargetName=PaperCut
InputPath=.\Resource.h

"hlp\$(TargetName).hm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	echo. >"hlp\$(TargetName).hm" 
	echo // Commands (ID_* and IDM_*) >>"hlp\$(TargetName).hm" 
	makehm ID_,HID_,0x10000 IDM_,HIDM_,0x10000 resource.h >>"hlp\$(TargetName).hm" 
	echo. >>"hlp\$(TargetName).hm" 
	echo // Prompts (IDP_*) >>"hlp\$(TargetName).hm" 
	makehm IDP_,HIDP_,0x30000 resource.h >>"hlp\$(TargetName).hm" 
	echo. >>"hlp\$(TargetName).hm" 
	echo // Resources (IDR_*) >>"hlp\$(TargetName).hm" 
	makehm IDR_,HIDR_,0x20000 resource.h >>"hlp\$(TargetName).hm" 
	echo. >>"hlp\$(TargetName).hm" 
	echo // Dialogs (IDD_*) >>"hlp\$(TargetName).hm" 
	makehm IDD_,HIDD_,0x20000 resource.h >>"hlp\$(TargetName).hm" 
	echo. >>"hlp\$(TargetName).hm" 
	echo // Frame Controls (IDW_*) >>"hlp\$(TargetName).hm" 
	makehm IDW_,HIDW_,0x50000 resource.h >>"hlp\$(TargetName).hm" 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "PaperCut - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Making help include file...
TargetName=PaperCut
InputPath=.\Resource.h

"hlp\$(TargetName).hm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	echo. >"hlp\$(TargetName).hm" 
	echo // Commands (ID_* and IDM_*) >>"hlp\$(TargetName).hm" 
	makehm ID_,HID_,0x10000 IDM_,HIDM_,0x10000 resource.h >>"hlp\$(TargetName).hm" 
	echo. >>"hlp\$(TargetName).hm" 
	echo // Prompts (IDP_*) >>"hlp\$(TargetName).hm" 
	makehm IDP_,HIDP_,0x30000 resource.h >>"hlp\$(TargetName).hm" 
	echo. >>"hlp\$(TargetName).hm" 
	echo // Resources (IDR_*) >>"hlp\$(TargetName).hm" 
	makehm IDR_,HIDR_,0x20000 resource.h >>"hlp\$(TargetName).hm" 
	echo. >>"hlp\$(TargetName).hm" 
	echo // Dialogs (IDD_*) >>"hlp\$(TargetName).hm" 
	makehm IDD_,HIDD_,0x20000 resource.h >>"hlp\$(TargetName).hm" 
	echo. >>"hlp\$(TargetName).hm" 
	echo // Frame Controls (IDW_*) >>"hlp\$(TargetName).hm" 
	makehm IDW_,HIDW_,0x50000 resource.h >>"hlp\$(TargetName).hm" 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Shape.h
# End Source File
# Begin Source File

SOURCE=.\ShapeLayout.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\versiondef.h

!IF  "$(CFG)" == "PaperCut - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Incrementing build version $(ProjDir)\versiondef.h
OutDir=.\Release
ProjDir=.
InputPath=.\versiondef.h

"versiondef.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	BumpBuild $(ProjDir)\versiondef.h BUILD_NUM 
	cl /c /Fo$(OutDir)\ /Zi /DNDEBUG /D_WINDOWS /DWIN32 /D_AFXDLL /D_MBCS versiondef.cpp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "PaperCut - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Incrementing build version $(ProjDir)\versiondef.h
OutDir=.\Debug
ProjDir=.
InputPath=.\versiondef.h

"versiondef.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	BumpBuild $(ProjDir)\versiondef.h BUILD_NUM 
	cl /c /Fo$(OutDir)\ /Zi /D_DEBUG /D_WINDOWS /DWIN32 /D_AFXDLL /D_MBCS versiondef.cpp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Vertex.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\cursor_p.cur
# End Source File
# Begin Source File

SOURCE=.\res\cursor_w.cur
# End Source File
# Begin Source File

SOURCE=".\res\henry-faded.bmp"
# End Source File
# Begin Source File

SOURCE=.\res\PaperCut.ico
# End Source File
# Begin Source File

SOURCE=.\res\PaperCut.rc2
# End Source File
# Begin Source File

SOURCE=.\res\PaperCutDoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\stellate.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# End Group
# Begin Group "Help Files"

# PROP Default_Filter "cnt;rtf"
# Begin Source File

SOURCE=.\hlp\AfxCore.rtf
# End Source File
# Begin Source File

SOURCE=.\hlp\AfxPrint.rtf
# End Source File
# Begin Source File

SOURCE=.\hlp\AppExit.bmp
# End Source File
# Begin Source File

SOURCE=".\hlp\assembly-cutting1.bmp"
# End Source File
# Begin Source File

SOURCE=.\hlp\Bullet.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\CurArw2.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\CurArw4.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\CurHelp.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\EditCopy.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\EditCut.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\EditPast.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\EditUndo.bmp
# End Source File
# Begin Source File

SOURCE=".\hlp\explorer-3files-selected.bmp"
# End Source File
# Begin Source File

SOURCE=.\hlp\FileNew.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\FileOpen.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\FilePrnt.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\FileSave.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\HlpSBar.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\HlpTBar.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\PaperCut.cnt

!IF  "$(CFG)" == "PaperCut - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Copying contents file...
OutDir=.\Release
InputPath=.\hlp\PaperCut.cnt
InputName=PaperCut

"$(OutDir)\$(InputName).cnt" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy "hlp\$(InputName).cnt" $(OutDir)

# End Custom Build

!ELSEIF  "$(CFG)" == "PaperCut - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Copying contents file...
OutDir=.\Debug
InputPath=.\hlp\PaperCut.cnt
InputName=PaperCut

"$(OutDir)\$(InputName).cnt" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy "hlp\$(InputName).cnt" $(OutDir)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\hlp\PaperCut.hm
# End Source File
# Begin Source File

SOURCE=.\hlp\RecFirst.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\RecLast.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\RecNext.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\RecPrev.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\Scmax.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\ScMenu.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\Scmin.bmp
# End Source File
# Begin Source File

SOURCE=".\hlp\size-slider.bmp"
# End Source File
# Begin Source File

SOURCE=".\hlp\tetra-3pics-resized.bmp"
# End Source File
# Begin Source File

SOURCE=".\hlp\tetra-3pics-viewonly.bmp"
# End Source File
# Begin Source File

SOURCE=".\hlp\tetra-3pics-withtext.bmp"
# End Source File
# Begin Source File

SOURCE=".\hlp\tetra-plain-screenshot.bmp"
# End Source File
# Begin Source File

SOURCE=".\hlp\tetra-print-preview.bmp"
# End Source File
# End Group
# Begin Source File

SOURCE=.\PaperCut.reg
# End Source File
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# Begin Source File

SOURCE=.\ShapeFileFormat.txt
# End Source File
# Begin Source File

SOURCE=.\ToDo.txt
# End Source File
# End Target
# End Project

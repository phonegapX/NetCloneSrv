# Microsoft Developer Studio Project File - Name="KdNetCloneSrv" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=KdNetCloneSrv - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "KdNetCloneSrv.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "KdNetCloneSrv.mak" CFG="KdNetCloneSrv - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "KdNetCloneSrv - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "KdNetCloneSrv - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "KdNetCloneSrv - Win32 Release"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /c
# ADD CPP /MT /W3 /GX- /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /c
# SUBTRACT CPP /nologo
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /subsystem:windows /pdb:none /machine:I386
# SUBTRACT LINK32 /nologo

!ELSEIF  "$(CFG)" == "KdNetCloneSrv - Win32 Debug"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "KdNetCloneSrv - Win32 Release"
# Name "KdNetCloneSrv - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Startup"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Startup\common.cpp
# End Source File
# Begin Source File

SOURCE=.\Startup\common.h
# End Source File
# Begin Source File

SOURCE=.\Startup\DHCP.CPP
# End Source File
# Begin Source File

SOURCE=.\Startup\PXE.cpp
# End Source File
# Begin Source File

SOURCE=.\Startup\TFTP.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\KdNetCloneSrv.cpp
# End Source File
# Begin Source File

SOURCE=.\KdNetCloneSrv.rc
# End Source File
# Begin Source File

SOURCE=.\KdNetCloneSrvDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\NetCardListDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\ServerDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\KdNetCloneSrv.h
# End Source File
# Begin Source File

SOURCE=.\KdNetCloneSrvDlg.h
# End Source File
# Begin Source File

SOURCE=.\NetCardListDialog.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\ServerDlg.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\img1.bin
# End Source File
# Begin Source File

SOURCE=.\res\KdNetCloneSrv.ico
# End Source File
# Begin Source File

SOURCE=.\res\KdNetCloneSrv.rc2
# End Source File
# End Group
# Begin Source File

SOURCE=.\res\ServerData\GhostSrv.exe
# End Source File
# Begin Source File

SOURCE=".\res\ClientData\netghos-c.img"
# End Source File
# Begin Source File

SOURCE=".\res\ClientData\netghos-d.img"
# End Source File
# Begin Source File

SOURCE=".\res\ClientData\netghos-e.img"
# End Source File
# Begin Source File

SOURCE=".\res\ClientData\netghos-f.img"
# End Source File
# Begin Source File

SOURCE=".\res\ClientData\netghos-hd.img"
# End Source File
# Begin Source File

SOURCE=".\res\ClientData\netghost-c.pxe"
# End Source File
# Begin Source File

SOURCE=".\res\ClientData\netghost-d.pxe"
# End Source File
# Begin Source File

SOURCE=".\res\ClientData\netghost-e.pxe"
# End Source File
# Begin Source File

SOURCE=".\res\ClientData\netghost-f.pxe"
# End Source File
# Begin Source File

SOURCE=".\res\ClientData\netghost-hd.pxe"
# End Source File
# Begin Source File

SOURCE=.\res\ServerData\ProtectHook.dll
# End Source File
# End Target
# End Project

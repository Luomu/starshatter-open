# Microsoft Developer Studio Project File - Name="NetEx" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=NetEx - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "NetEx.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "NetEx.mak" CFG="NetEx - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "NetEx - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "NetEx - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "NetEx - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "../FoundationEx" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "SYNC_THREADS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "NetEx - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "../FoundationEx" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "SYNC_THREADS" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "NetEx - Win32 Release"
# Name "NetEx - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\HttpClient.cpp
# End Source File
# Begin Source File

SOURCE=.\HttpServer.cpp
# End Source File
# Begin Source File

SOURCE=.\HttpServlet.cpp
# End Source File
# Begin Source File

SOURCE=.\HttpServletExec.cpp
# End Source File
# Begin Source File

SOURCE=.\NetAddr.cpp
# End Source File
# Begin Source File

SOURCE=.\NetClient.cpp
# End Source File
# Begin Source File

SOURCE=.\NetGram.cpp
# End Source File
# Begin Source File

SOURCE=.\NetHost.cpp
# End Source File
# Begin Source File

SOURCE=.\NetLayer.cpp
# End Source File
# Begin Source File

SOURCE=.\NetLink.cpp
# End Source File
# Begin Source File

SOURCE=.\NetMsg.cpp
# End Source File
# Begin Source File

SOURCE=.\NetPeer.cpp
# End Source File
# Begin Source File

SOURCE=.\NetServer.cpp
# End Source File
# Begin Source File

SOURCE=.\NetSock.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\HttpClient.h
# End Source File
# Begin Source File

SOURCE=.\HttpServer.h
# End Source File
# Begin Source File

SOURCE=.\HttpServlet.h
# End Source File
# Begin Source File

SOURCE=.\HttpServletExec.h
# End Source File
# Begin Source File

SOURCE=.\NetAddr.h
# End Source File
# Begin Source File

SOURCE=.\NetClient.h
# End Source File
# Begin Source File

SOURCE=.\NetGram.h
# End Source File
# Begin Source File

SOURCE=.\NetHost.h
# End Source File
# Begin Source File

SOURCE=.\NetLayer.h
# End Source File
# Begin Source File

SOURCE=.\NetLink.h
# End Source File
# Begin Source File

SOURCE=.\NetMsg.h
# End Source File
# Begin Source File

SOURCE=.\NetPeer.h
# End Source File
# Begin Source File

SOURCE=.\NetServer.h
# End Source File
# Begin Source File

SOURCE=.\NetSock.h
# End Source File
# End Group
# End Target
# End Project

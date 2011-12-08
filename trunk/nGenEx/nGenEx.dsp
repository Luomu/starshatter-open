# Microsoft Developer Studio Project File - Name="nGenEx" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=nGenEx - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "nGenEx.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "nGenEx.mak" CFG="nGenEx - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "nGenEx - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "nGenEx - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "nGenEx - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\parser" /I "..\FoundationEx" /I "..\zlib" /I "..\LibPng" /I "..\Opcode\OpcodeLib" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /FR /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "nGenEx - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "nGenEx___Win32_Debug"
# PROP BASE Intermediate_Dir "nGenEx___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\parser" /I "..\FoundationEx" /I "..\zlib" /I "..\LibPng" /I "..\Opcode\OpcodeLib" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FR /YX /FD /GZ /c
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

# Name "nGenEx - Win32 Release"
# Name "nGenEx - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\ActiveWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\Archive.cpp
# End Source File
# Begin Source File

SOURCE=..\FoundationEx\ArrayList.cpp
# End Source File
# Begin Source File

SOURCE=.\AviFile.cpp
# End Source File
# Begin Source File

SOURCE=.\Bitmap.cpp
# End Source File
# Begin Source File

SOURCE=.\Bmp.cpp
# End Source File
# Begin Source File

SOURCE=.\Bolt.cpp
# End Source File
# Begin Source File

SOURCE=.\Button.cpp
# End Source File
# Begin Source File

SOURCE=.\Camera.cpp
# End Source File
# Begin Source File

SOURCE=.\CameraView.cpp
# End Source File
# Begin Source File

SOURCE=.\Color.cpp
# End Source File
# Begin Source File

SOURCE=.\ComboBox.cpp
# End Source File
# Begin Source File

SOURCE=.\ComboList.cpp
# End Source File
# Begin Source File

SOURCE=.\ContentBundle.cpp
# End Source File
# Begin Source File

SOURCE=.\D3DXImage.cpp
# End Source File
# Begin Source File

SOURCE=.\DataLoader.cpp
# End Source File
# Begin Source File

SOURCE=.\EditBox.cpp
# End Source File
# Begin Source File

SOURCE=.\Encrypt.cpp
# End Source File
# Begin Source File

SOURCE=.\EventDispatch.cpp
# End Source File
# Begin Source File

SOURCE=.\FadeView.cpp
# End Source File
# Begin Source File

SOURCE=.\Fix.cpp
# End Source File
# Begin Source File

SOURCE=.\Font.cpp
# End Source File
# Begin Source File

SOURCE=.\FontMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\FormatUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\FormDef.cpp
# End Source File
# Begin Source File

SOURCE=.\FormWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\Game.cpp
# End Source File
# Begin Source File

SOURCE=.\Geometry.cpp
# End Source File
# Begin Source File

SOURCE=.\Graphic.cpp
# End Source File
# Begin Source File

SOURCE=.\ImageBox.cpp
# End Source File
# Begin Source File

SOURCE=.\ImgView.cpp
# End Source File
# Begin Source File

SOURCE=.\Joystick.cpp
# End Source File
# Begin Source File

SOURCE=.\Keyboard.cpp
# End Source File
# Begin Source File

SOURCE=.\Layout.cpp
# End Source File
# Begin Source File

SOURCE=.\Light.cpp
# End Source File
# Begin Source File

SOURCE=.\ListBox.cpp
# End Source File
# Begin Source File

SOURCE=.\Locale.cpp
# End Source File
# Begin Source File

SOURCE=.\MachineInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\MCIWave.cpp
# End Source File
# Begin Source File

SOURCE=..\FoundationEx\MemDebug.cpp
# End Source File
# Begin Source File

SOURCE=.\Menu.cpp
# End Source File
# Begin Source File

SOURCE=.\Mouse.cpp
# End Source File
# Begin Source File

SOURCE=.\MouseController.cpp
# End Source File
# Begin Source File

SOURCE=.\MultiController.cpp
# End Source File
# Begin Source File

SOURCE=..\Parser\Parser.cpp
# End Source File
# Begin Source File

SOURCE=.\ParseUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\Particles.cpp
# End Source File
# Begin Source File

SOURCE=.\PCX.CPP
# End Source File
# Begin Source File

SOURCE=.\Physical.cpp
# End Source File
# Begin Source File

SOURCE=.\PngImage.cpp
# End Source File
# Begin Source File

SOURCE=.\Polygon.cpp
# End Source File
# Begin Source File

SOURCE=.\Projector.cpp
# End Source File
# Begin Source File

SOURCE=.\Random.cpp
# End Source File
# Begin Source File

SOURCE=..\Parser\Reader.cpp
# End Source File
# Begin Source File

SOURCE=.\Res.cpp
# End Source File
# Begin Source File

SOURCE=.\RichTextBox.cpp
# End Source File
# Begin Source File

SOURCE=.\Scene.cpp
# End Source File
# Begin Source File

SOURCE=.\Screen.cpp
# End Source File
# Begin Source File

SOURCE=.\ScrollWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\Sha1.cpp
# End Source File
# Begin Source File

SOURCE=.\Shadow.cpp
# End Source File
# Begin Source File

SOURCE=.\Skin.cpp
# End Source File
# Begin Source File

SOURCE=.\Slider.cpp
# End Source File
# Begin Source File

SOURCE=.\Solid.cpp
# End Source File
# Begin Source File

SOURCE=.\Sound.cpp
# End Source File
# Begin Source File

SOURCE=.\SoundCard.cpp
# End Source File
# Begin Source File

SOURCE=.\SoundD3D.cpp
# End Source File
# Begin Source File

SOURCE=.\Sprite.cpp
# End Source File
# Begin Source File

SOURCE=..\Parser\Term.cpp
# End Source File
# Begin Source File

SOURCE=.\TexCubeDX9.cpp
# End Source File
# Begin Source File

SOURCE=.\TexDX9.cpp
# End Source File
# Begin Source File

SOURCE=..\FoundationEx\Text.cpp
# End Source File
# Begin Source File

SOURCE=..\Parser\Token.cpp
# End Source File
# Begin Source File

SOURCE=.\Video.cpp
# End Source File
# Begin Source File

SOURCE=.\VideoDX9.cpp
# End Source File
# Begin Source File

SOURCE=.\VideoDX9Enum.cpp
# End Source File
# Begin Source File

SOURCE=.\VideoDX9VertexBuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\VideoFactory.cpp
# End Source File
# Begin Source File

SOURCE=.\VideoSettings.cpp
# End Source File
# Begin Source File

SOURCE=.\Water.cpp
# End Source File
# Begin Source File

SOURCE=.\WebBrowser.cpp
# End Source File
# Begin Source File

SOURCE=.\Window.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\ActiveWindow.h
# End Source File
# Begin Source File

SOURCE=.\Archive.h
# End Source File
# Begin Source File

SOURCE=..\FoundationEx\ArrayList.h
# End Source File
# Begin Source File

SOURCE=.\AviFile.h
# End Source File
# Begin Source File

SOURCE=.\Bitmap.h
# End Source File
# Begin Source File

SOURCE=.\Bmp.h
# End Source File
# Begin Source File

SOURCE=.\Bolt.h
# End Source File
# Begin Source File

SOURCE=.\Button.h
# End Source File
# Begin Source File

SOURCE=.\Camera.h
# End Source File
# Begin Source File

SOURCE=.\CameraView.h
# End Source File
# Begin Source File

SOURCE=.\Color.h
# End Source File
# Begin Source File

SOURCE=.\ComboBox.h
# End Source File
# Begin Source File

SOURCE=.\ComboList.h
# End Source File
# Begin Source File

SOURCE=.\ContentBundle.h
# End Source File
# Begin Source File

SOURCE=.\D3DXImage.h
# End Source File
# Begin Source File

SOURCE=.\DataLoader.h
# End Source File
# Begin Source File

SOURCE=..\FoundationEx\Dictionary.h
# End Source File
# Begin Source File

SOURCE=..\FoundationEx\Dictionary.inl
# End Source File
# Begin Source File

SOURCE=.\Director.h
# End Source File
# Begin Source File

SOURCE=.\EditBox.h
# End Source File
# Begin Source File

SOURCE=.\Encrypt.h
# End Source File
# Begin Source File

SOURCE=.\EventDispatch.h
# End Source File
# Begin Source File

SOURCE=.\EventTarget.h
# End Source File
# Begin Source File

SOURCE=.\EventTgt.h
# End Source File
# Begin Source File

SOURCE=.\Fix.h
# End Source File
# Begin Source File

SOURCE=.\Font.h
# End Source File
# Begin Source File

SOURCE=.\FontMgr.h
# End Source File
# Begin Source File

SOURCE=.\FormatUtil.h
# End Source File
# Begin Source File

SOURCE=.\FormDef.h
# End Source File
# Begin Source File

SOURCE=.\FormWindow.h
# End Source File
# Begin Source File

SOURCE=.\Game.h
# End Source File
# Begin Source File

SOURCE=.\Geometry.h
# End Source File
# Begin Source File

SOURCE=.\Graphic.h
# End Source File
# Begin Source File

SOURCE=.\IA3D.H
# End Source File
# Begin Source File

SOURCE=.\ImageBox.h
# End Source File
# Begin Source File

SOURCE=.\ImgView.h
# End Source File
# Begin Source File

SOURCE=.\Joystick.h
# End Source File
# Begin Source File

SOURCE=.\Keyboard.h
# End Source File
# Begin Source File

SOURCE=.\Layout.h
# End Source File
# Begin Source File

SOURCE=.\Light.h
# End Source File
# Begin Source File

SOURCE=..\FoundationEx\List.inl
# End Source File
# Begin Source File

SOURCE=.\ListBox.h
# End Source File
# Begin Source File

SOURCE=.\Locale.h
# End Source File
# Begin Source File

SOURCE=.\MachineInfo.h
# End Source File
# Begin Source File

SOURCE=.\MCIWave.h
# End Source File
# Begin Source File

SOURCE=..\FoundationEx\MemDebug.h
# End Source File
# Begin Source File

SOURCE=.\Menu.h
# End Source File
# Begin Source File

SOURCE=.\MotionController.h
# End Source File
# Begin Source File

SOURCE=.\Mouse.h
# End Source File
# Begin Source File

SOURCE=.\MouseController.h
# End Source File
# Begin Source File

SOURCE=.\MultiController.h
# End Source File
# Begin Source File

SOURCE=..\Parser\Parser.h
# End Source File
# Begin Source File

SOURCE=.\Particles.h
# End Source File
# Begin Source File

SOURCE=.\Pcx.h
# End Source File
# Begin Source File

SOURCE=.\Physical.h
# End Source File
# Begin Source File

SOURCE=.\Polygon.h
# End Source File
# Begin Source File

SOURCE=.\Projector.h
# End Source File
# Begin Source File

SOURCE=.\Random.h
# End Source File
# Begin Source File

SOURCE=..\Parser\Reader.h
# End Source File
# Begin Source File

SOURCE=.\Res.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\RichTextBox.h
# End Source File
# Begin Source File

SOURCE=.\Scene.h
# End Source File
# Begin Source File

SOURCE=.\Screen.h
# End Source File
# Begin Source File

SOURCE=.\ScrollWindow.h
# End Source File
# Begin Source File

SOURCE=.\Sha1.h
# End Source File
# Begin Source File

SOURCE=.\Shadow.h
# End Source File
# Begin Source File

SOURCE=.\Skin.h
# End Source File
# Begin Source File

SOURCE=.\Slider.h
# End Source File
# Begin Source File

SOURCE=.\Solid.h
# End Source File
# Begin Source File

SOURCE=.\Sound.h
# End Source File
# Begin Source File

SOURCE=.\SoundCard.h
# End Source File
# Begin Source File

SOURCE=.\SoundD3D.h
# End Source File
# Begin Source File

SOURCE=.\Sprite.h
# End Source File
# Begin Source File

SOURCE=..\Parser\Term.h
# End Source File
# Begin Source File

SOURCE=.\TexCubeDX9.h
# End Source File
# Begin Source File

SOURCE=.\TexDX9.h
# End Source File
# Begin Source File

SOURCE=..\FoundationEx\Text.h
# End Source File
# Begin Source File

SOURCE=..\FoundationEx\ThreadSync.h
# End Source File
# Begin Source File

SOURCE=.\TimeSnap.h
# End Source File
# Begin Source File

SOURCE=..\Parser\Token.h
# End Source File
# Begin Source File

SOURCE=.\Types.h
# End Source File
# Begin Source File

SOURCE="..\..\Program Files\Microsoft Visual Studio\VC98\Include\SYS\TYPES.H"
# End Source File
# Begin Source File

SOURCE=.\Universe.h
# End Source File
# Begin Source File

SOURCE=.\Video.h
# End Source File
# Begin Source File

SOURCE=.\VideoDX9.h
# End Source File
# Begin Source File

SOURCE=.\VideoDX9Enum.h
# End Source File
# Begin Source File

SOURCE=.\VideoDX9VertexBuffer.h
# End Source File
# Begin Source File

SOURCE=.\VideoFactory.h
# End Source File
# Begin Source File

SOURCE=.\VideoSettings.h
# End Source File
# Begin Source File

SOURCE=.\View.h
# End Source File
# Begin Source File

SOURCE=.\Water.h
# End Source File
# Begin Source File

SOURCE=.\Wave.h
# End Source File
# Begin Source File

SOURCE=.\WebBrowser.h
# End Source File
# Begin Source File

SOURCE=.\Window.h
# End Source File
# End Group
# End Target
# End Project

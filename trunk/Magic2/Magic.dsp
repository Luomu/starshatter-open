# Microsoft Developer Studio Project File - Name="Magic" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Magic - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Magic.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Magic.mak" CFG="Magic - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Magic - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Magic - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Magic - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GX /O2 /I "../pnglib" /I "../Opcode/OpcodeLib" /I "../libpng" /I "../Magic2" /I "../FoundationEx" /I "../nGenEx" /I "../zlib" /I "../parser" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "FOUNDATION_USE_MFC" /D "_AFXDLL" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 ..\Opcode\OpcodeLib\Release\OpcodeLib.lib ..\ngenex\release\ngenex.lib ..\zlib\release\zlib.lib ..\libpng\release\libpng.lib wsock32.lib dinput.lib dsound.lib d3d9.lib d3dx9.lib dxguid.lib winmm.lib version.lib ogg_static.lib vorbis_static.lib vorbisfile_static.lib vfw32.lib /nologo /subsystem:windows /incremental:yes /map /machine:I386 /nodefaultlib:"LIBC"

!ELSEIF  "$(CFG)" == "Magic - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../Opcode/OpcodeLib" /I "../libpng" /I "../Magic2" /I "../FoundationEx" /I "../nGenEx" /I "../zlib" /I "../parser" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "FOUNDATION_USE_MFC" /D "_AFXDLL" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ..\Opcode\OpcodeLib\Debug\OpcodeLib.lib ..\zlib\debug\zlib.lib ..\libpng\debug\libpng.lib wsock32.lib dinput.lib dsound.lib d3d9.lib d3dx9.lib dxguid.lib winmm.lib version.lib ogg_static.lib vorbis_static.lib vorbisfile_static.lib vfw32.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"LIBCMT" /nodefaultlib:"LIBCD" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "Magic - Win32 Release"
# Name "Magic - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\nGenEx\ActiveWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\AlphaInverse.cpp
# End Source File
# Begin Source File

SOURCE=.\AlphaPalette.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\Archive.cpp
# End Source File
# Begin Source File

SOURCE=..\FoundationEx\ArrayList.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\AviFile.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\Bitmap.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\Bmp.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\Bolt.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\Button.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\Camera.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\CameraView.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\Color.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\ComboBox.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\ComboList.cpp
# End Source File
# Begin Source File

SOURCE=.\Command.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\D3DXImage.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\DataLoader.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\EditBox.cpp
# End Source File
# Begin Source File

SOURCE=.\Editor.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\Encrypt.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\EventDispatch.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\FadeView.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\Fix.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\Font.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\FontMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\FormatUtil.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\FormDef.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\FormWindow.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\Game.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\Geometry.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\Graphic.cpp
# End Source File
# Begin Source File

SOURCE=.\Grid.cpp
# End Source File
# Begin Source File

SOURCE=.\GridProps.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\ImageBox.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\ImgView.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\Joystick.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\Keyboard.cpp
# End Source File
# Begin Source File

SOURCE=.\l3ds.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\Layout.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\Light.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\ListBox.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\MachineInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\Magic.cpp
# End Source File
# Begin Source File

SOURCE=.\Magic.rc
# End Source File
# Begin Source File

SOURCE=.\MagicDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\MagicView.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\MaterialDialog.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\MCIWave.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\Menu.cpp
# End Source File
# Begin Source File

SOURCE=.\ModelFile3DS.cpp
# End Source File
# Begin Source File

SOURCE=.\ModelFileMAG.cpp
# End Source File
# Begin Source File

SOURCE=.\ModelFileOBJ.cpp
# End Source File
# Begin Source File

SOURCE=.\ModelView.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\Mouse.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\MouseController.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\MultiController.cpp
# End Source File
# Begin Source File

SOURCE=..\Parser\Parser.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\ParseUtil.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\Particles.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\PCX.CPP
# End Source File
# Begin Source File

SOURCE=..\nGenEx\Physical.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\PngImage.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\Polygon.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\Projector.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\Random.cpp
# End Source File
# Begin Source File

SOURCE=..\Parser\Reader.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\Res.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\RichTextBox.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\Scene.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\Screen.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\ScrollWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\Selection.cpp
# End Source File
# Begin Source File

SOURCE=.\Selector.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\Sha1.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\Shadow.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\Slider.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\Solid.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\Sound.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\SoundCard.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\SoundD3D.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\Sprite.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\SurfacePropertiesDialog.cpp
# End Source File
# Begin Source File

SOURCE=..\Parser\Term.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\TexDX9.cpp
# End Source File
# Begin Source File

SOURCE=..\FoundationEx\Text.cpp
# End Source File
# Begin Source File

SOURCE=.\TextureMapDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\Thumbnail.cpp
# End Source File
# Begin Source File

SOURCE=..\Parser\Token.cpp
# End Source File
# Begin Source File

SOURCE=.\UVMapView.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\Video.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\VideoDX9.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\VideoDX9Enum.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\VideoDX9VertexBuffer.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\VideoFactory.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\VideoSettings.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\Window.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\nGenEx\AviFile.h
# End Source File
# Begin Source File

SOURCE=.\Command.h
# End Source File
# Begin Source File

SOURCE=.\Editor.h
# End Source File
# Begin Source File

SOURCE=.\Grid.h
# End Source File
# Begin Source File

SOURCE=.\GridProps.h
# End Source File
# Begin Source File

SOURCE=.\l3ds.h
# End Source File
# Begin Source File

SOURCE=.\Magic.h
# End Source File
# Begin Source File

SOURCE=.\MagicDoc.h
# End Source File
# Begin Source File

SOURCE=.\MagicView.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\MaterialDialog.h
# End Source File
# Begin Source File

SOURCE=.\ModelFile3DS.h
# End Source File
# Begin Source File

SOURCE=.\ModelFileMAG.h
# End Source File
# Begin Source File

SOURCE=.\ModelFileOBJ.h
# End Source File
# Begin Source File

SOURCE=.\ModelView.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\Selection.h
# End Source File
# Begin Source File

SOURCE=.\Selector.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\SurfacePropertiesDialog.h
# End Source File
# Begin Source File

SOURCE=.\TextureMapDialog.h
# End Source File
# Begin Source File

SOURCE=.\Thumbnail.h
# End Source File
# Begin Source File

SOURCE=.\UVMapView.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\Magic.ico
# End Source File
# Begin Source File

SOURCE=.\res\Magic.rc2
# End Source File
# Begin Source File

SOURCE=.\res\MagicDoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# End Group
# End Target
# End Project

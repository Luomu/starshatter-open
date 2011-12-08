# Microsoft Developer Studio Project File - Name="Stars" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Stars - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Stars.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Stars.mak" CFG="Stars - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Stars - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Stars - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Stars - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "../FoundationEx" /I "../nGenEx" /I "../NetEx" /I "../Parser" /I "../Opcode/OpcodeLib" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 ..\Opcode\OpcodeLib\Release\OpcodeLib.lib ..\ngenex\release\ngenex.lib ..\netex\release\netex.lib ..\zlib\release\zlib.lib ..\libpng\release\libpng.lib wsock32.lib dinput.lib dsound.lib d3d9.lib d3dx9.lib dxguid.lib winmm.lib version.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib ogg_static.lib vorbis_static.lib vorbisfile_static.lib Vfw32.lib /nologo /subsystem:windows /profile /map:"Game/Stars.map" /debug /machine:I386 /out:"Game/Stars.exe"

!ELSEIF  "$(CFG)" == "Stars - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "../FoundationEx" /I "../nGenEx" /I "../NetEx" /I "../Parser" /I "../Opcode/OpcodeLib" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ..\Opcode\OpcodeLib\Debug\OpcodeLib.lib ..\ngenex\debug\ngenex.lib ..\netex\debug\netex.lib ..\zlib\debug\zlib.lib ..\libpng\debug\libpng.lib wsock32.lib dinput.lib dsound.lib d3d9.lib d3dx9.lib dxguid.lib winmm.lib version.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib ogg_static.lib vorbis_static.lib vorbisfile_static.lib Vfw32.lib /nologo /subsystem:windows /map /debug /machine:I386 /out:"Game/Stars_d.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "Stars - Win32 Release"
# Name "Stars - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Asteroid.cpp
# End Source File
# Begin Source File

SOURCE=.\AudDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\AudioConfig.cpp
# End Source File
# Begin Source File

SOURCE=.\Authorization.cpp
# End Source File
# Begin Source File

SOURCE=.\AwardDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\AwardShowDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Callsign.cpp
# End Source File
# Begin Source File

SOURCE=.\CameraDirector.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\CameraView.cpp

!IF  "$(CFG)" == "Stars - Win32 Release"

# ADD CPP /FAcs

!ELSEIF  "$(CFG)" == "Stars - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Campaign.cpp
# End Source File
# Begin Source File

SOURCE=.\CampaignMissionFighter.cpp
# End Source File
# Begin Source File

SOURCE=.\CampaignMissionRequest.cpp
# End Source File
# Begin Source File

SOURCE=.\CampaignMissionStarship.cpp
# End Source File
# Begin Source File

SOURCE=.\CampaignPlanAssignment.cpp
# End Source File
# Begin Source File

SOURCE=.\CampaignPlanEvent.cpp
# End Source File
# Begin Source File

SOURCE=.\CampaignPlanMission.cpp
# End Source File
# Begin Source File

SOURCE=.\CampaignPlanMovement.cpp
# End Source File
# Begin Source File

SOURCE=.\CampaignPlanStrategic.cpp
# End Source File
# Begin Source File

SOURCE=.\CampaignSaveGame.cpp
# End Source File
# Begin Source File

SOURCE=.\CampaignSituationReport.cpp
# End Source File
# Begin Source File

SOURCE=.\CarrierAI.cpp
# End Source File
# Begin Source File

SOURCE=.\CmdDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\CmdForceDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\CmdIntelDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\CmdMissionsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\CmdMsgDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\CmdOrdersDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\CmdTheaterDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\CmdTitleDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\CmpCompleteDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\CmpFileDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\CmpLoadDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\CmpnScreen.cpp
# End Source File
# Begin Source File

SOURCE=.\CmpSceneDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\CmpSelectDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\CombatAction.cpp
# End Source File
# Begin Source File

SOURCE=.\Combatant.cpp
# End Source File
# Begin Source File

SOURCE=.\CombatAssignment.cpp
# End Source File
# Begin Source File

SOURCE=.\CombatEvent.cpp
# End Source File
# Begin Source File

SOURCE=.\CombatGroup.cpp
# End Source File
# Begin Source File

SOURCE=.\CombatRoster.cpp
# End Source File
# Begin Source File

SOURCE=.\CombatUnit.cpp
# End Source File
# Begin Source File

SOURCE=.\CombatZone.cpp
# End Source File
# Begin Source File

SOURCE=.\Component.cpp
# End Source File
# Begin Source File

SOURCE=.\Computer.cpp
# End Source File
# Begin Source File

SOURCE=.\ConfirmDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Contact.cpp
# End Source File
# Begin Source File

SOURCE=.\CtlDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\DebriefDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Debris.cpp
# End Source File
# Begin Source File

SOURCE=.\DetailSet.cpp
# End Source File
# Begin Source File

SOURCE=.\DisplayView.cpp
# End Source File
# Begin Source File

SOURCE=.\Drive.cpp
# End Source File
# Begin Source File

SOURCE=.\DriveSprite.cpp
# End Source File
# Begin Source File

SOURCE=.\Drone.cpp
# End Source File
# Begin Source File

SOURCE=.\DropShipAI.cpp
# End Source File
# Begin Source File

SOURCE=.\Element.cpp
# End Source File
# Begin Source File

SOURCE=.\EngDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ExceptionHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\ExitDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Explosion.cpp
# End Source File
# Begin Source File

SOURCE=.\Farcaster.cpp
# End Source File
# Begin Source File

SOURCE=.\FighterAI.cpp
# End Source File
# Begin Source File

SOURCE=.\FighterTacticalAI.cpp
# End Source File
# Begin Source File

SOURCE=.\FirstTimeDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\FlightComp.cpp
# End Source File
# Begin Source File

SOURCE=.\FlightDeck.cpp
# End Source File
# Begin Source File

SOURCE=.\FlightPlanner.cpp
# End Source File
# Begin Source File

SOURCE=.\FltDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\Font.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\FormDef.cpp
# End Source File
# Begin Source File

SOURCE=.\Galaxy.cpp
# End Source File
# Begin Source File

SOURCE=.\GameScreen.cpp
# End Source File
# Begin Source File

SOURCE=.\Grid.cpp
# End Source File
# Begin Source File

SOURCE=.\GroundAI.cpp
# End Source File
# Begin Source File

SOURCE=.\Hangar.cpp
# End Source File
# Begin Source File

SOURCE=.\HardPoint.cpp
# End Source File
# Begin Source File

SOURCE=.\Hoop.cpp
# End Source File
# Begin Source File

SOURCE=.\HUDSounds.cpp
# End Source File
# Begin Source File

SOURCE=.\HUDView.cpp
# End Source File
# Begin Source File

SOURCE=.\Instruction.cpp
# End Source File
# Begin Source File

SOURCE=.\Intel.cpp
# End Source File
# Begin Source File

SOURCE=.\JoyDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\KeyDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\KeyMap.cpp
# End Source File
# Begin Source File

SOURCE=.\LandingGear.cpp
# End Source File
# Begin Source File

SOURCE=.\LoadDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\LoadScreen.cpp
# End Source File
# Begin Source File

SOURCE=.\Main.cpp
# End Source File
# Begin Source File

SOURCE=.\MapView.cpp
# End Source File
# Begin Source File

SOURCE=.\MenuDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MenuScreen.cpp
# End Source File
# Begin Source File

SOURCE=.\MenuView.cpp
# End Source File
# Begin Source File

SOURCE=.\Mfd.cpp
# End Source File
# Begin Source File

SOURCE=.\Mission.cpp
# End Source File
# Begin Source File

SOURCE=.\MissionEvent.cpp
# End Source File
# Begin Source File

SOURCE=.\MissionTemplate.cpp
# End Source File
# Begin Source File

SOURCE=.\ModConfig.cpp
# End Source File
# Begin Source File

SOURCE=.\ModDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ModInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\ModInfoDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\Mouse.cpp
# End Source File
# Begin Source File

SOURCE=.\MsnDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MsnEditDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MsnEditNavDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MsnElemDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MsnEventDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MsnNavDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MsnObjDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MsnPkgDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MsnSelectDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MsnWepDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MusicDirector.cpp
# End Source File
# Begin Source File

SOURCE=.\MusicTrack.cpp
# End Source File
# Begin Source File

SOURCE=.\NavAI.cpp
# End Source File
# Begin Source File

SOURCE=.\NavDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\NavLight.cpp
# End Source File
# Begin Source File

SOURCE=.\NavSystem.cpp
# End Source File
# Begin Source File

SOURCE=.\NetAddrDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\NetAdminChat.cpp
# End Source File
# Begin Source File

SOURCE=.\NetAdminServer.cpp
# End Source File
# Begin Source File

SOURCE=.\NetAuth.cpp
# End Source File
# Begin Source File

SOURCE=.\NetBrokerClient.cpp
# End Source File
# Begin Source File

SOURCE=.\NetChat.cpp
# End Source File
# Begin Source File

SOURCE=.\NetClientConfig.cpp
# End Source File
# Begin Source File

SOURCE=.\NetClientDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\NetData.cpp
# End Source File
# Begin Source File

SOURCE=.\NetFileServlet.cpp
# End Source File
# Begin Source File

SOURCE=.\NetGame.cpp
# End Source File
# Begin Source File

SOURCE=.\NetGameClient.cpp
# End Source File
# Begin Source File

SOURCE=.\NetGameServer.cpp
# End Source File
# Begin Source File

SOURCE=.\NetLobby.cpp
# End Source File
# Begin Source File

SOURCE=.\NetLobbyClient.cpp
# End Source File
# Begin Source File

SOURCE=.\NetLobbyDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\NetLobbyServer.cpp
# End Source File
# Begin Source File

SOURCE=.\NetPacket.cpp
# End Source File
# Begin Source File

SOURCE=.\NetPassDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\NetPlayer.cpp
# End Source File
# Begin Source File

SOURCE=.\NetServerConfig.cpp
# End Source File
# Begin Source File

SOURCE=.\NetServerDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\NetUnitDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\NetUser.cpp
# End Source File
# Begin Source File

SOURCE=.\NetUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\NPClientWraps.cpp
# End Source File
# Begin Source File

SOURCE=.\OptDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\Particles.cpp
# End Source File
# Begin Source File

SOURCE=.\PlanScreen.cpp
# End Source File
# Begin Source File

SOURCE=.\Player.cpp
# End Source File
# Begin Source File

SOURCE=.\PlayerDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Power.cpp
# End Source File
# Begin Source File

SOURCE=.\QuantumDrive.cpp
# End Source File
# Begin Source File

SOURCE=.\QuantumFlash.cpp
# End Source File
# Begin Source File

SOURCE=.\QuantumView.cpp
# End Source File
# Begin Source File

SOURCE=.\QuitView.cpp
# End Source File
# Begin Source File

SOURCE=.\RadioHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\RadioMessage.cpp
# End Source File
# Begin Source File

SOURCE=.\RadioTraffic.cpp
# End Source File
# Begin Source File

SOURCE=.\RadioView.cpp
# End Source File
# Begin Source File

SOURCE=.\RadioVox.cpp
# End Source File
# Begin Source File

SOURCE=.\RLoc.cpp
# End Source File
# Begin Source File

SOURCE=.\SeekerAI.cpp
# End Source File
# Begin Source File

SOURCE=.\Sensor.cpp
# End Source File
# Begin Source File

SOURCE=.\Shield.cpp
# End Source File
# Begin Source File

SOURCE=.\ShieldRep.cpp
# End Source File
# Begin Source File

SOURCE=.\Ship.cpp
# End Source File
# Begin Source File

SOURCE=.\ShipAI.cpp
# End Source File
# Begin Source File

SOURCE=.\ShipCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\ShipDesign.cpp
# End Source File
# Begin Source File

SOURCE=.\ShipKiller.cpp
# End Source File
# Begin Source File

SOURCE=.\ShipSolid.cpp
# End Source File
# Begin Source File

SOURCE=.\Shot.cpp
# End Source File
# Begin Source File

SOURCE=.\Sim.cpp

!IF  "$(CFG)" == "Stars - Win32 Release"

# ADD CPP /FAcs

!ELSEIF  "$(CFG)" == "Stars - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\SimEvent.cpp
# End Source File
# Begin Source File

SOURCE=.\SimObject.cpp
# End Source File
# Begin Source File

SOURCE=.\Sky.cpp
# End Source File
# Begin Source File

SOURCE=.\Stars.rc
# End Source File
# Begin Source File

SOURCE=.\StarServer.cpp
# End Source File
# Begin Source File

SOURCE=.\Starshatter.cpp
# End Source File
# Begin Source File

SOURCE=.\StarshipAI.cpp
# End Source File
# Begin Source File

SOURCE=.\StarshipTacticalAI.cpp
# End Source File
# Begin Source File

SOURCE=.\StarSystem.cpp
# End Source File
# Begin Source File

SOURCE=.\SteerAI.cpp
# End Source File
# Begin Source File

SOURCE=.\System.cpp
# End Source File
# Begin Source File

SOURCE=.\SystemDesign.cpp
# End Source File
# Begin Source File

SOURCE=.\TacRefDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\TacticalAI.cpp
# End Source File
# Begin Source File

SOURCE=.\TacticalView.cpp
# End Source File
# Begin Source File

SOURCE=.\Terrain.cpp
# End Source File
# Begin Source File

SOURCE=.\TerrainApron.cpp
# End Source File
# Begin Source File

SOURCE=.\TerrainClouds.cpp
# End Source File
# Begin Source File

SOURCE=.\TerrainHaze.cpp
# End Source File
# Begin Source File

SOURCE=.\TerrainPatch.cpp
# End Source File
# Begin Source File

SOURCE=.\TerrainRegion.cpp
# End Source File
# Begin Source File

SOURCE=.\Thruster.cpp
# End Source File
# Begin Source File

SOURCE=.\TrackIR.cpp
# End Source File
# Begin Source File

SOURCE=.\Trail.cpp
# End Source File
# Begin Source File

SOURCE=.\VidDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\nGenEx\Water.cpp
# End Source File
# Begin Source File

SOURCE=.\Weapon.cpp
# End Source File
# Begin Source File

SOURCE=.\WeaponDesign.cpp
# End Source File
# Begin Source File

SOURCE=.\WeaponGroup.cpp
# End Source File
# Begin Source File

SOURCE=.\Weather.cpp
# End Source File
# Begin Source File

SOURCE=.\WepView.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Asteroid.h
# End Source File
# Begin Source File

SOURCE=.\AudDlg.h
# End Source File
# Begin Source File

SOURCE=.\AudioConfig.h
# End Source File
# Begin Source File

SOURCE=.\Authorization.h
# End Source File
# Begin Source File

SOURCE=.\AwardDlg.h
# End Source File
# Begin Source File

SOURCE=.\AwardShowDlg.h
# End Source File
# Begin Source File

SOURCE=.\BaseScreen.h
# End Source File
# Begin Source File

SOURCE=.\Callsign.h
# End Source File
# Begin Source File

SOURCE=.\CameraDirector.h
# End Source File
# Begin Source File

SOURCE=.\Campaign.h
# End Source File
# Begin Source File

SOURCE=.\CampaignMissionFighter.h
# End Source File
# Begin Source File

SOURCE=.\CampaignMissionRequest.h
# End Source File
# Begin Source File

SOURCE=.\CampaignMissionStarship.h
# End Source File
# Begin Source File

SOURCE=.\CampaignPlan.h
# End Source File
# Begin Source File

SOURCE=.\CampaignPlanAssignment.h
# End Source File
# Begin Source File

SOURCE=.\CampaignPlanEvent.h
# End Source File
# Begin Source File

SOURCE=.\CampaignPlanMission.h
# End Source File
# Begin Source File

SOURCE=.\CampaignPlanMovement.h
# End Source File
# Begin Source File

SOURCE=.\CampaignPlanStrategic.h
# End Source File
# Begin Source File

SOURCE=.\CampaignSituationReport.h
# End Source File
# Begin Source File

SOURCE=.\CmdDlg.h
# End Source File
# Begin Source File

SOURCE=.\CmdForceDlg.h
# End Source File
# Begin Source File

SOURCE=.\CmdIntelDlg.h
# End Source File
# Begin Source File

SOURCE=.\CmdMissionsDlg.h
# End Source File
# Begin Source File

SOURCE=.\CmdMsgDlg.h
# End Source File
# Begin Source File

SOURCE=.\CmdOrdersDlg.h
# End Source File
# Begin Source File

SOURCE=.\CmdTheaterDlg.h
# End Source File
# Begin Source File

SOURCE=.\CmdTitleDlg.h
# End Source File
# Begin Source File

SOURCE=.\CmpCompleteDlg.h
# End Source File
# Begin Source File

SOURCE=.\CmpFileDlg.h
# End Source File
# Begin Source File

SOURCE=.\CmpLoadDlg.h
# End Source File
# Begin Source File

SOURCE=.\CmpnScreen.h
# End Source File
# Begin Source File

SOURCE=.\CmpSceneDlg.h
# End Source File
# Begin Source File

SOURCE=.\CmpSelectDlg.h
# End Source File
# Begin Source File

SOURCE=.\CombatAction.h
# End Source File
# Begin Source File

SOURCE=.\CombatEvent.h
# End Source File
# Begin Source File

SOURCE=.\CombatGroup.h
# End Source File
# Begin Source File

SOURCE=.\CombatRoster.h
# End Source File
# Begin Source File

SOURCE=.\CombatUnit.h
# End Source File
# Begin Source File

SOURCE=.\CombatZone.h
# End Source File
# Begin Source File

SOURCE=.\Component.h
# End Source File
# Begin Source File

SOURCE=.\ConfirmDlg.h
# End Source File
# Begin Source File

SOURCE=.\CtlDlg.h
# End Source File
# Begin Source File

SOURCE=..\nGenEx\DataLoader.h
# End Source File
# Begin Source File

SOURCE=.\DebriefDlg.h
# End Source File
# Begin Source File

SOURCE=.\Debris.h
# End Source File
# Begin Source File

SOURCE=.\DetailSet.h
# End Source File
# Begin Source File

SOURCE=.\DisplayView.h
# End Source File
# Begin Source File

SOURCE=.\DriveSprite.h
# End Source File
# Begin Source File

SOURCE=.\Drone.h
# End Source File
# Begin Source File

SOURCE=.\DropShipAI.h
# End Source File
# Begin Source File

SOURCE=.\Element.h
# End Source File
# Begin Source File

SOURCE=.\EngDlg.h
# End Source File
# Begin Source File

SOURCE=.\ExitDlg.h
# End Source File
# Begin Source File

SOURCE=.\Explosion.h
# End Source File
# Begin Source File

SOURCE=.\FighterAI.h
# End Source File
# Begin Source File

SOURCE=.\FighterTacticalAI.h
# End Source File
# Begin Source File

SOURCE=.\FlightPlanner.h
# End Source File
# Begin Source File

SOURCE=.\FltDlg.h
# End Source File
# Begin Source File

SOURCE=.\Galaxy.h
# End Source File
# Begin Source File

SOURCE=.\GameScreen.h
# End Source File
# Begin Source File

SOURCE=.\Grid.h
# End Source File
# Begin Source File

SOURCE=.\GroundAI.h
# End Source File
# Begin Source File

SOURCE=.\Hoop.h
# End Source File
# Begin Source File

SOURCE=.\HUDSounds.h
# End Source File
# Begin Source File

SOURCE=.\HUDView.h
# End Source File
# Begin Source File

SOURCE=.\Instruction.h
# End Source File
# Begin Source File

SOURCE=.\Intel.h
# End Source File
# Begin Source File

SOURCE=.\JoyDlg.h
# End Source File
# Begin Source File

SOURCE=.\KeyDlg.h
# End Source File
# Begin Source File

SOURCE=.\KeyMap.h
# End Source File
# Begin Source File

SOURCE=.\LoadDlg.h
# End Source File
# Begin Source File

SOURCE=.\LoadScreen.h
# End Source File
# Begin Source File

SOURCE=.\MapView.h
# End Source File
# Begin Source File

SOURCE=.\MenuDlg.h
# End Source File
# Begin Source File

SOURCE=.\MenuScreen.h
# End Source File
# Begin Source File

SOURCE=.\MenuView.h
# End Source File
# Begin Source File

SOURCE=.\Mfd.h
# End Source File
# Begin Source File

SOURCE=.\Mission.h
# End Source File
# Begin Source File

SOURCE=.\MissionEvent.h
# End Source File
# Begin Source File

SOURCE=.\MissionTemplate.h
# End Source File
# Begin Source File

SOURCE=.\ModConfig.h
# End Source File
# Begin Source File

SOURCE=.\ModDlg.h
# End Source File
# Begin Source File

SOURCE=.\ModInfo.h
# End Source File
# Begin Source File

SOURCE=.\ModInfoDlg.h
# End Source File
# Begin Source File

SOURCE=.\MsnDlg.h
# End Source File
# Begin Source File

SOURCE=.\MsnEditDlg.h
# End Source File
# Begin Source File

SOURCE=.\MsnElemDlg.h
# End Source File
# Begin Source File

SOURCE=.\MsnEventDlg.h
# End Source File
# Begin Source File

SOURCE=.\MsnNavDlg.h
# End Source File
# Begin Source File

SOURCE=.\MsnObjDlg.h
# End Source File
# Begin Source File

SOURCE=.\MsnPkgDlg.h
# End Source File
# Begin Source File

SOURCE=.\MsnSelectDlg.h
# End Source File
# Begin Source File

SOURCE=.\MsnWepDlg.h
# End Source File
# Begin Source File

SOURCE=.\MusicDirector.h
# End Source File
# Begin Source File

SOURCE=.\MusicTrack.h
# End Source File
# Begin Source File

SOURCE=.\NavAI.h
# End Source File
# Begin Source File

SOURCE=.\NavDlg.h
# End Source File
# Begin Source File

SOURCE=.\NavLight.h
# End Source File
# Begin Source File

SOURCE=.\NetAddrDlg.h
# End Source File
# Begin Source File

SOURCE=.\NetAdminChat.h
# End Source File
# Begin Source File

SOURCE=.\NetAdminServer.h
# End Source File
# Begin Source File

SOURCE=.\NetAuth.h
# End Source File
# Begin Source File

SOURCE=.\NetBrokerClient.h
# End Source File
# Begin Source File

SOURCE=.\NetChat.h
# End Source File
# Begin Source File

SOURCE=.\NetClientConfig.h
# End Source File
# Begin Source File

SOURCE=.\NetClientDlg.h
# End Source File
# Begin Source File

SOURCE=.\NetData.h
# End Source File
# Begin Source File

SOURCE=.\NetFileServlet.h
# End Source File
# Begin Source File

SOURCE=.\NetGame.h
# End Source File
# Begin Source File

SOURCE=.\NetGameClient.h
# End Source File
# Begin Source File

SOURCE=.\NetGameServer.h
# End Source File
# Begin Source File

SOURCE=.\NetLobby.h
# End Source File
# Begin Source File

SOURCE=.\NetLobbyClient.h
# End Source File
# Begin Source File

SOURCE=.\NetLobbyDlg.h
# End Source File
# Begin Source File

SOURCE=.\NetLobbyServer.h
# End Source File
# Begin Source File

SOURCE=.\NetPacket.h
# End Source File
# Begin Source File

SOURCE=.\NetPassDlg.h
# End Source File
# Begin Source File

SOURCE=.\NetPlayer.h
# End Source File
# Begin Source File

SOURCE=.\NetServerConfig.h
# End Source File
# Begin Source File

SOURCE=.\NetServerDlg.h
# End Source File
# Begin Source File

SOURCE=.\NetUnitDlg.h
# End Source File
# Begin Source File

SOURCE=.\NetUtil.h
# End Source File
# Begin Source File

SOURCE=.\NPClientWraps.h
# End Source File
# Begin Source File

SOURCE=.\OptDlg.h
# End Source File
# Begin Source File

SOURCE=.\PlanScreen.h
# End Source File
# Begin Source File

SOURCE=.\Player.h
# End Source File
# Begin Source File

SOURCE=.\PlayerDlg.h
# End Source File
# Begin Source File

SOURCE=.\Power.h
# End Source File
# Begin Source File

SOURCE=.\QuantumFlash.h
# End Source File
# Begin Source File

SOURCE=.\QuantumView.h
# End Source File
# Begin Source File

SOURCE=.\RadioHandler.h
# End Source File
# Begin Source File

SOURCE=.\RadioMessage.h
# End Source File
# Begin Source File

SOURCE=.\RadioView.h
# End Source File
# Begin Source File

SOURCE=.\RadioVox.h
# End Source File
# Begin Source File

SOURCE=.\RLoc.h
# End Source File
# Begin Source File

SOURCE=.\SeekerAI.h
# End Source File
# Begin Source File

SOURCE=.\ShieldRep.h
# End Source File
# Begin Source File

SOURCE=.\ShipCtrl.h
# End Source File
# Begin Source File

SOURCE=.\ShipKiller.h
# End Source File
# Begin Source File

SOURCE=.\StarServer.h
# End Source File
# Begin Source File

SOURCE=.\Starshatter.h
# End Source File
# Begin Source File

SOURCE=.\StarshipAI.h
# End Source File
# Begin Source File

SOURCE=.\StarshipTacticalAI.h
# End Source File
# Begin Source File

SOURCE=.\StarSystem.h
# End Source File
# Begin Source File

SOURCE=.\SteerAI.h
# End Source File
# Begin Source File

SOURCE=.\SystemDesign.h
# End Source File
# Begin Source File

SOURCE=.\TacRefDlg.h
# End Source File
# Begin Source File

SOURCE=.\TacticalAI.h
# End Source File
# Begin Source File

SOURCE=.\Terrain.h
# End Source File
# Begin Source File

SOURCE=.\TerrainApron.h
# End Source File
# Begin Source File

SOURCE=.\TerrainClouds.h
# End Source File
# Begin Source File

SOURCE=.\TerrainHaze.h
# End Source File
# Begin Source File

SOURCE=.\TerrainLayer.h
# End Source File
# Begin Source File

SOURCE=.\TerrainPatch.h
# End Source File
# Begin Source File

SOURCE=.\Thruster.h
# End Source File
# Begin Source File

SOURCE=.\TrackIR.h
# End Source File
# Begin Source File

SOURCE=.\Trail.h
# End Source File
# Begin Source File

SOURCE=.\VidDlg.h
# End Source File
# Begin Source File

SOURCE=.\WeaponDesign.h
# End Source File
# Begin Source File

SOURCE=.\Weather.h
# End Source File
# Begin Source File

SOURCE=.\WepView.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\Stars.ico
# End Source File
# End Group
# End Target
# End Project

/*  Project Starshatter 4.6
    Destroyer Studios LLC
    Copyright © 1997-2006. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         TrackIR.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    TrackIR head tracker interface class
*/

#include "MemDebug.h"
#include "TrackIR.h"
#include "NPClient.h"
#include "NPClientWraps.h"

#include "Game.h"
#include "Text.h"

// +--------------------------------------------------------------------+

const double TRACK_TOP     =  -8000;
const double TRACK_BOTTOM  =   8000;
const double TRACK_LEFT    =  16000;
const double TRACK_RIGHT   = -16000;
const double TRACK_XYZ     =  24000;

// +--------------------------------------------------------------------+

static Text GetDllFromRegistry()
{
   Text  dllLoc;
   BYTE  dllBuf[1024];
   DWORD dllLen = 0;
   HKEY  hkey   = 0;

   ZeroMemory(dllBuf, sizeof(dllBuf));

   RegOpenKeyEx(HKEY_CURRENT_USER,
                "Software\\NaturalPoint\\NATURALPOINT\\NPClient Location",
                0,
                KEY_QUERY_VALUE,
                &hkey);

   if (hkey) {
      dllLen  = 1024;

      LONG result =
      RegQueryValueEx(hkey,
                      "Path",
                      NULL,
                      NULL,
                      dllBuf,
                      &dllLen);

      if (result == ERROR_SUCCESS && dllLen > 0)
         dllLoc = (const char*) dllBuf;

      RegCloseKey(hkey);
   }

   return dllLoc;
}

static const char* NPErrString(NPRESULT r)
{
   switch (r) {
   case NP_OK:                      return "OK";
   case NP_ERR_DEVICE_NOT_PRESENT:  return "Device not present";
   case NP_ERR_UNSUPPORTED_OS:      return "Unsupported O/S";
   case NP_ERR_INVALID_ARG:         return "Invalid argument";
   case NP_ERR_DLL_NOT_FOUND:       return "NaturalPoint DLL not found";
   case NP_ERR_NO_DATA:             return "No data available";
   case NP_ERR_INTERNAL_DATA:       return "Internal error";
   }

   return "Unknown error code";
}

// +--------------------------------------------------------------------+

TrackIR::TrackIR() : 
   running(false), frame_signature(0),
   az(0), el(0), x(0), y(0), z(0)
{
   Print("*** NaturalPoint Game Client Initialization ***\n");

   // Hook up the NaturalPoint game client DLL using the wrapper module
   NPRESULT result;
   Text dllPath = GetDllFromRegistry();

   // Initialize the NPClient interface
   result = NPClient_Init(dllPath);
   if (result == NP_OK) {
      Print("NPClient - Initialize successful.\n");
   }
   else {
      Print("NPClient - Unable to initialize interface: %s\n", NPErrString(result));
      return;
   }

   // Register the app's window handle
   result = NP_RegisterWindowHandle(Game::GetHWND());

   if (result == NP_OK) {
      Print("NPClient - Window handle registration successful.\n");
   }
   else {
      Print("NPClient - Error registering window handle: %s\n", NPErrString(result));
      return;
   }

   // Query the NaturalPoint software version
   unsigned short wNPClientVer;
   result = NP_QueryVersion( &wNPClientVer );

   if (result == NP_OK) {
      Print("NPClient - NaturalPoint software version: %d.%02d\n", (wNPClientVer >> 8), (wNPClientVer & 0x00FF));
   }
   else {
      Print("NPClient - Error querying NaturalPoint software version: %s\n", NPErrString(result));
   }


   // It is *required* that your application registers the Developer ID 
   // assigned by NaturalPoint!

   // Your assigned developer ID needs to be inserted below!    
   #define NP_DEVELOPER_ID 6401

   // NOTE : The title of your project must show up 
   // in the list of supported titles shown in the Profiles
   // tab of the TrackIR software, if it does not then the
   // TrackIR software will *not* transmit data to your
   // application. If your title is not present in the list, 
   // you may need to have the TrackIR software perform a
   // game list update (to download support for new Developer IDs)
   // using the menu item under the "Help" or "Update" menu.

   NP_RegisterProgramProfileID(NP_DEVELOPER_ID);

   unsigned int DataFields = 0;
   DataFields |= NPPitch;
   DataFields |= NPYaw;

   NP_RequestData(DataFields);

   result = NP_StopCursor();
   if (result == NP_OK)
      Print("NPClient - Cursor stopped.\n");
   else
      Print("NPClient - Error stopping cursor: %s\n", NPErrString(result));


   result = NP_StartDataTransmission();
   if (result == NP_OK) {
      Print("NPClient - Data transmission started.\n");
      running = true;
   }
   else {
      Print("NPClient - Error starting data transmission: %s\n", NPErrString(result));
   }

}

TrackIR::~TrackIR()
{
   if (running) {
      Print("NaturalPoint Game Client Shutdown\n");

      NP_StopDataTransmission();
      NP_UnregisterWindowHandle();
   }
}

// +--------------------------------------------------------------------+

DWORD
TrackIR::ExecFrame()
{
   TRACKIRDATA tid;

   // Go get the latest data
   NPRESULT result = NP_GetData( &tid );

   if (result == NP_OK) {
      // Got data to process ...
      running = true;

      // compare the last frame signature to the current one
      // if they are not the same then new data has arrived since then

      if (tid.wNPStatus == NPSTATUS_REMOTEACTIVE) {
         if (frame_signature != tid.wPFrameSignature) {
            double pitch = tid.fNPPitch;
            double yaw   = tid.fNPYaw;

            if (pitch < 0) {
               el = pitch / TRACK_TOP;
            }
            else {
               el = -pitch / TRACK_BOTTOM;
            }

            if (yaw < 0) {
               az = yaw / TRACK_RIGHT;
            }
            else {
               az = -yaw / TRACK_LEFT;
            }

            x = tid.fNPX / TRACK_XYZ * -1;
            y = tid.fNPY / TRACK_XYZ;
            z = tid.fNPZ / TRACK_XYZ * -1;

            if (z < -0.25) z = -0.25;

            frame_signature = tid.wPFrameSignature;
         }
         else {
            // Either there is no tracking data, the user has
            // paused the trackIR, or the call happened before
            // the TrackIR was able to update the interface
            // with new data

            az *= 0.75;
            el *= 0.75;
            x  *= 0.75;
            y  *= 0.75;
            z  *= 0.75;

            result = NP_ERR_NO_DATA;
         }
      }
      else {
         // The user has set the device out of trackIR Enhanced Mode
         // and into Mouse Emulation mode with the hotkey
         result  = NP_ERR_NO_DATA;
         running = false;
      }
   }

   return result;
}

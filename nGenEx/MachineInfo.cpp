/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib
    FILE:         MachineInfo.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Collect and Display Machine, OS, and Driver Information
*/

#include "MemDebug.h"
#include "MachineInfo.h"
#include "Timesnap.h"

#define DIRECTINPUT_VERSION 0x0700

#include <stdio.h>
#include <ddraw.h>
#include <d3d9.h>
#include <dinput.h>
#include <winver.h>

// +--------------------------------------------------------------------+

void Print(const char* fmt, ...);

static int cpu_class=-1;
static int cpu_speed=-1;
static int platform=-1;
static int dx_version=-1;
static int total_ram=-1;

static OSVERSIONINFO os_ver      = { sizeof(OSVERSIONINFO) };
static SYSTEM_INFO   cpu_info;
static MEMORYSTATUS  mem_info    = { sizeof(MEMORYSTATUS) };

// +--------------------------------------------------------------------+

const char*
MachineInfo::GetShortDescription()
{
   static char desc[256];

   static const char* cpu_names[] = {
      "8088", 
      "8086", 
      "80286",
      "80386",
      "80486",
      "Pentium",
      "Pentium II",
      "Pentium 3",
      "Pentium 4"
   };

   static const char* os_names[] = {
      "DOS",
      "Windows 95",
      "Windows 98",
      "Windows NT",
      "Windows 2000",
      "Windows XP"
   };

   int cpu_index = GetCpuClass();
   if (cpu_index < 0)      cpu_index = 0;
   else if (cpu_index > 8) cpu_index = 8;

   int os_index = GetPlatform();
   if (os_index < 0)      os_index = 0;
   else if (os_index > 5) os_index = 5;

   sprintf(desc, "%s %d MHz %d MB RAM %s",
      cpu_names[cpu_index],
      GetCpuSpeed(),
      GetTotalRam(),
      os_names[os_index]);

   return desc;
}

// +--------------------------------------------------------------------+

static void DescribeCpuMake();
static void DescribeOwner95();
static void DescribeOwnerNT();
static void DescribeDrivers95(const char* sType);
static void DescribeDriversNT(const char* sType);
static void DescribeDriverVersion(const char* file);
static void DescribeDXVersion(const char* component);

// wait for at least target_time,
// return the exact amount of time actually waited:

static double SpinWait(double target_time)
{
   double actual_time = 0;

   LARGE_INTEGER  ifreq;
   LARGE_INTEGER  cnt1;
   LARGE_INTEGER  cnt2;

   QueryPerformanceFrequency(&ifreq);
   double freq = (double) ifreq.QuadPart;

   QueryPerformanceCounter(&cnt1);

   do {
      QueryPerformanceCounter(&cnt2);

      double delta = (double) (cnt2.QuadPart - cnt1.QuadPart);
      actual_time  = delta / freq;
   }
   while (actual_time < target_time);

   return actual_time;
}

static double CalcCpuSpeed()
{
   DWORD clock1 = 0;
   DWORD clock2 = 0;

   TIMESNAP(clock1);

   double seconds = SpinWait(0.1);
   
   TIMESNAP(clock2);

   double clocks = clock2 - clock1;

   return (clocks/seconds);
}

/****************************************************************************
 *
 *      GetDXVersion
 *
 *  This function returns
 *              0       Insufficient DirectX installed
 *              9       At least DirectX 9 installed.
 *
 ****************************************************************************/

DWORD GetDXVersion()
{
   HRESULT       hr      = 0;
   HINSTANCE     DDHinst = 0;
   LPDIRECT3D9   d3d9    = 0;
   OSVERSIONINFO osVer   = { sizeof(OSVERSIONINFO) };

   // First get the windows platform

   if (!GetVersionEx(&osVer))
     return 0;

   // NT versions do not support DirectX 9
   if (osVer.dwPlatformId == VER_PLATFORM_WIN32_NT) {
      if (osVer.dwMajorVersion <= 4)
         return 0;
   }

   DDHinst = LoadLibrary("D3D9.DLL");
   if (DDHinst == 0) {
     return 0;
   }

   FreeLibrary(DDHinst);
   return 9;
}


// +--------------------------------------------------------------------+

int
MachineInfo::GetCpuClass()
{
   if (cpu_class < 0) {
      GetSystemInfo(&cpu_info);

      if (cpu_info.wProcessorArchitecture != PROCESSOR_ARCHITECTURE_INTEL ||
          cpu_info.dwProcessorType        <  PROCESSOR_INTEL_PENTIUM)
          Print("INCOMPATIBLE CPU TYPE!\n");

      if (GetPlatform() < OS_WINNT)
         cpu_class = CPU_P5;

      else
         cpu_class = cpu_info.wProcessorLevel;
   }

   return cpu_class;
}

// +--------------------------------------------------------------------+

int
MachineInfo::GetCpuSpeed()
{
   if (cpu_speed < 0) {
      cpu_speed = (int) (CalcCpuSpeed() / 1e6);
   }

   return cpu_speed;
}

// +--------------------------------------------------------------------+

int
MachineInfo::GetTotalRam()
{
   if (total_ram < 0) {
      GlobalMemoryStatus(&mem_info);
      total_ram = (int) (mem_info.dwTotalPhys/(1024*1024)) + 1;
   }

   return total_ram;
}

// +--------------------------------------------------------------------+

int
MachineInfo::GetPlatform()
{
   if (platform < 0) {
      GetVersionEx(&os_ver);

      switch (os_ver.dwPlatformId) {
      default:
      case VER_PLATFORM_WIN32s: {
         char msg[256];
         sprintf(msg, "Invalid Operating System Platform: %d\n", os_ver.dwPlatformId);
         Print(msg);
         }
         break;

      case VER_PLATFORM_WIN32_WINDOWS:
         if (os_ver.dwMajorVersion == 4 && os_ver.dwMinorVersion == 0)
            platform = OS_WIN95;
         else
            platform = OS_WIN98;
         break;

      case VER_PLATFORM_WIN32_NT:
         if (os_ver.dwMajorVersion == 4)
            platform = OS_WINNT;
         else if (os_ver.dwMajorVersion == 5 && os_ver.dwMinorVersion ==  0)
            platform = OS_WIN2K;
         else if (os_ver.dwMajorVersion >= 5)
            platform = OS_WINXP;

         else {
            platform = OS_INVALID;

            Print("Invalid Operating System Platform (NT-series): %d.%d\n", os_ver.dwMajorVersion, os_ver.dwMinorVersion);
         }

         break;
      }
   }

   return platform;
}

// +--------------------------------------------------------------------+

int
MachineInfo::GetDirectXVersion()
{
   if (dx_version < 0) {
      dx_version = GetDXVersion();
   }

   return dx_version;
}

// +--------------------------------------------------------------------+

void
MachineInfo::DescribeMachine()
{
   GetPlatform();
   GetCpuClass();

   Print("+====================================================================+\n");
   Print("|                                                                    |\n");

   char txt[256];

   switch (platform) {
   case OS_WIN95:
      sprintf(txt, "Windows 95  version %d.%d.%d %s",
              os_ver.dwMajorVersion,
              os_ver.dwMinorVersion,
              LOWORD(os_ver.dwBuildNumber),
              os_ver.szCSDVersion);
      break;

   case OS_WIN98:
      sprintf(txt, "Windows 98  version %d.%d.%d %s",
              os_ver.dwMajorVersion,
              os_ver.dwMinorVersion,
              LOWORD(os_ver.dwBuildNumber),
              os_ver.szCSDVersion);
      break;

   case OS_WINNT:
      sprintf(txt, "Windows NT %d.%d (Build %d) %s",
              os_ver.dwMajorVersion,
              os_ver.dwMinorVersion,
              os_ver.dwBuildNumber,
              os_ver.szCSDVersion);
      break;

   case OS_WIN2K:
      sprintf(txt, "Windows 2000 %d.%d (Build %d) %s",
              os_ver.dwMajorVersion,
              os_ver.dwMinorVersion,
              os_ver.dwBuildNumber,
              os_ver.szCSDVersion);

   case OS_WINXP:
      sprintf(txt, "Windows XP %d.%d (Build %d) %s",
              os_ver.dwMajorVersion,
              os_ver.dwMinorVersion,
              os_ver.dwBuildNumber,
              os_ver.szCSDVersion);
      break;

   default:
      sprintf(txt, "Unknown Operating System Platform");
      break;
   }

   Print("| %-66s |\n", txt);
   Print("|                                                                    |\n");

   if (platform == OS_WIN95 || platform == OS_WIN98)
      DescribeOwner95();
   else
      DescribeOwnerNT();

   sprintf(txt, "CPUs Detected: %d    CPU Level: %d.%d.%d    CPU Speed: %d",
           cpu_info.dwNumberOfProcessors,
           cpu_info.wProcessorLevel,
           cpu_info.wProcessorRevision >> 8,
           cpu_info.wProcessorRevision & 0xff,
           GetCpuSpeed() + 1);

   Print("| %-66s |\n", txt);
   DescribeCpuMake();

   GlobalMemoryStatus(&mem_info);
   total_ram      = (int) (mem_info.dwTotalPhys/(1024*1024)) + 1;
   int swap_max   = (int) (mem_info.dwTotalPageFile/(1024*1024));
   int swap_avail = (int) (mem_info.dwAvailPageFile/(1024*1024));

   sprintf(txt, "%d MB RAM    %d MB Max Swap    %d MB Avail Swap", 
      total_ram, swap_max, swap_avail);


   Print("| %-66s |\n", txt);

   Print("|                                                                    |\n");
   Print("| DirectX %d installed.                                               |\n",
      GetDirectXVersion());
   DescribeDXVersion("DDRAW");
   DescribeDXVersion("D3DIM");
   DescribeDXVersion("DINPUT");
   DescribeDXVersion("DPLAY");
   DescribeDXVersion("DSOUND");
   DescribeDXVersion("DMUSIC");
   DescribeDXVersion("DSHOW");
   Print("|                                                                    |\n");


   if (platform == OS_WIN95 || platform == OS_WIN98) {
      DescribeDrivers95("Display");
      DescribeDrivers95("Media");
      DescribeDrivers95("Monitor");
      DescribeDrivers95("Multimedia");
   }
   else {
      DescribeDriversNT("");
   }

   Print("+====================================================================+\n");
   Print("\n");
}

// +--------------------------------------------------------------------+

static void DescribeCpuMake()
{
   HKEY  hkWin;
   char  sProcessor[256] = "";
   char  sMMXInfo[256]   = "";
   char  sVendor[256]    = "";
   DWORD dwSize;

   if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                    "Hardware\\Description\\System\\CentralProcessor\\0",
                    0,
                    KEY_READ,
                    &hkWin) == ERROR_SUCCESS) {

      dwSize = 256;
      RegQueryValueEx(hkWin,
                      "Identifier",
                      NULL,
                      NULL,
                      (LPBYTE) sProcessor,
                      &dwSize);

      dwSize = 256;
      RegQueryValueEx(hkWin,
                      "MMXIdentifier",
                      NULL,
                      NULL,
                      (LPBYTE) sMMXInfo,
                      &dwSize);

      dwSize = 256;
      RegQueryValueEx(hkWin,
                      "VendorIdentifier",
                      NULL,
                      NULL,
                      (LPBYTE) sVendor,
                      &dwSize);

      RegCloseKey(hkWin);
   }

   if (sProcessor[0])   Print("| %-66s |\n", sProcessor);
   if (sMMXInfo[0])     Print("| %-66s |\n", sMMXInfo);
   if (sVendor[0])      Print("| %-66s |\n", sVendor);

   if (sProcessor[0] || sMMXInfo[0] || sVendor[0])
      Print("|                                                                    |\n");
}

// +--------------------------------------------------------------------+

static void DescribeOwner95()
{
   HKEY  hkWin;
   char  sRegisteredOwner[256] = "";
   char  sRegisteredOrganization[256] = "";
   DWORD dwSize;

   if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                    "SOFTWARE\\Microsoft\\Windows\\CurrentVersion",
                    0,
                    KEY_READ,
                    &hkWin) == ERROR_SUCCESS) {

      dwSize = 256;
      RegQueryValueEx(hkWin,
                      "RegisteredOwner",
                      NULL,
                      NULL,
                      (LPBYTE) sRegisteredOwner,
                      &dwSize);

      dwSize = 256;
      RegQueryValueEx(hkWin,
                      "RegisteredOrganization",
                      NULL,
                      NULL,
                      (LPBYTE) sRegisteredOrganization,
                      &dwSize);

      RegCloseKey(hkWin);
   }
   else {
      Print("Could not access registered owner\n");
   }

   if (sRegisteredOwner[0]) {
      char  txt[256];
      sprintf(txt, "Registered Owner: %s, %s", sRegisteredOwner, sRegisteredOrganization);
      Print("| %-66s |\n", txt);
      Print("|                                                                    |\n");
   }
}

static void DescribeOwnerNT()
{
   HKEY  hkWin;
   char  sRegisteredOwner[256] = "";
   char  sRegisteredOrganization[256] = "";
   DWORD dwSize;

   if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                    "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
                    0,
                    KEY_READ,
                    &hkWin) == ERROR_SUCCESS) {

      dwSize = 256;
      RegQueryValueEx(hkWin,
                      "RegisteredOwner",
                      NULL,
                      NULL,
                      (LPBYTE) sRegisteredOwner,
                      &dwSize);

      dwSize = 256;
      RegQueryValueEx(hkWin,
                      "RegisteredOrganization",
                      NULL,
                      NULL,
                      (LPBYTE) sRegisteredOrganization,
                      &dwSize);

      RegCloseKey(hkWin);
   }
   else {
      Print("Could not access registered owner\n");
   }

   if (sRegisteredOwner[0]) {
      char  txt[256];
      sprintf(txt, "Registered Owner: %s, %s", sRegisteredOwner, sRegisteredOrganization);
      Print("| %-66s |\n", txt);
      Print("|                                                                    |\n");
   }
}

// +--------------------------------------------------------------------+

static void DescribeDrivers95(const char* sType)
{
   HKEY  hkWin, hkSub;
   int   nKey = 0;
   char  sKey[256];
   char  sSub[256];
   char  sDriver[256];
   char  txt[256];
   DWORD dwSize;
   int   worked;

   // describe the video driver(s):
   do {
      worked = 0;

      sprintf(sKey, "System\\CurrentControlSet\\Services\\Class\\%s\\%04X", sType, nKey);
      sprintf(sSub, "System\\CurrentControlSet\\Services\\Class\\%s\\%04X\\DEFAULT", sType, nKey);

      if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                       sKey,
                       0,
                       KEY_READ,
                       &hkWin) == ERROR_SUCCESS) {

         dwSize = 256;
         RegQueryValueEx(hkWin,
                         "DriverDesc",
                         NULL,
                         NULL,
                         (LPBYTE) sDriver,
                         &dwSize);

         if (sDriver[0]) {
            sprintf(txt, "*  %s", sDriver);
            Print("| %-66s |\n", txt);
            worked = 1;
         }

         // try to find the driver file name:
         if (worked) {
            ZeroMemory(sDriver, sizeof(sDriver));

            dwSize = 256;
            DWORD err = RegQueryValueEx(hkWin, "Driver", NULL, NULL, (LPBYTE) sDriver, &dwSize);

            if (err != ERROR_SUCCESS) {
            dwSize = 256;
            err = RegQueryValueEx(hkWin, "DeviceDriver", NULL, NULL, (LPBYTE) sDriver, &dwSize);
            }

            if (err != ERROR_SUCCESS) {
            dwSize = 256;
            err = RegQueryValueEx(hkWin, "drv", NULL, NULL, (LPBYTE) sDriver, &dwSize);
            }

            if (err != ERROR_SUCCESS) {
               if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                sSub,
                                0,
                                KEY_READ,
                                &hkSub) == ERROR_SUCCESS) {

                  dwSize = 256;
                  err = RegQueryValueEx(hkSub, "drv", NULL, NULL, (LPBYTE) sDriver, &dwSize);

                  RegCloseKey(hkSub);
               }
            }

            // if we found it, try to display version info:
            if (err == ERROR_SUCCESS) {
               DescribeDriverVersion(sDriver);
            }

            Print("|                                                                    |\n");
         }

         RegCloseKey(hkWin);
      }

      nKey++;
   }
   while (worked);
}

static void DescribeDriversNT(const char* sType)
{
   Print("|                                                                    |\n");

   HKEY  hkWin;
   char  sVideo[256] = "";
   char  sDriver[256] = "";
   DWORD dwSize;

   // find the pointer to the video driver:
   if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                    "HARDWARE\\DEVICEMAP\\VIDEO",
                    0,
                    KEY_READ,
                    &hkWin) == ERROR_SUCCESS) {

      dwSize = 256;
      RegQueryValueEx(hkWin,
                      "\\Device\\Video0",
                      NULL,
                      NULL,
                      (LPBYTE) sVideo,
                      &dwSize);

      RegCloseKey(hkWin);
   }

   // follow the pointer and get the driver description:
   if (dwSize && sVideo[0]) {
      const char* sLeader = "\\REGISTRY\\Machine\\";
      int         nLeader = strlen(sLeader);

      if (strnicmp(sVideo, sLeader, nLeader) == 0) {
         if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                          sVideo + nLeader,
                          0,
                          KEY_READ,
                          &hkWin) == ERROR_SUCCESS) {

            dwSize = 256;
            RegQueryValueEx(hkWin,
                            "Device Description",
                            NULL,
                            NULL,
                            (LPBYTE) sDriver,
                            &dwSize);

            RegCloseKey(hkWin);
         }
      }
   }

   if (sDriver[0]) {
      Print("| %-66s |\n", sDriver);
      Print("|                                                                    |\n");
   }
}

// +--------------------------------------------------------------------+

static char sTranslation[16];

static void GetTranslation(const LPBYTE pBlock)
{
   LPBYTE  sData = NULL;
   UINT    lenData = 0;

   if (VerQueryValue(pBlock, "\\VarFileInfo\\Translation",
                     (LPVOID*) &sData, &lenData)) {

      if (lenData && sData) {
         sprintf(sTranslation, "%02X%02X%02X%02X", sData[1], sData[0], sData[3], sData[2]);
      }
   }
}

void DisplayVersionString(const LPBYTE pBlock, LPTSTR sSection)
{
   char     txt[256];
   char     sFullSection[256];

   sprintf(sFullSection, "\\StringFileInfo\\%s\\%s", sTranslation, sSection);

   LPBYTE  sData = NULL;
   UINT    lenData = 0;
   DWORD   dwErr = 0;

   if (VerQueryValue(pBlock, sFullSection, (LPVOID*) &sData, &lenData)) {
      if (lenData && sData) {
         sprintf(txt, "%-16s %s", sSection, sData);
         Print("|       %-60s |\n", txt);
      }
   }
}

static void DescribeDriverVersion(const char* file)
{
   DWORD dwHandle = 0;
   TCHAR szFile[512];

   strcpy(szFile, file);

   int nBytes = GetFileVersionInfoSize(szFile, &dwHandle);

   if (nBytes <= 0) {
      char  szWinDir[256];
      GetSystemDirectory(szWinDir, 256);
      sprintf(szFile, "%s\\%s", szWinDir, file);

      nBytes = GetFileVersionInfoSize(szFile, &dwHandle);

      if (nBytes <= 0)
         return;
   }

   LPBYTE pBlock = new(__FILE__,__LINE__) BYTE[nBytes];

   if (pBlock && GetFileVersionInfo(szFile, dwHandle, nBytes, (LPVOID) pBlock)) {
      GetTranslation(pBlock);
      DisplayVersionString(pBlock, "CompanyName");
//      DisplayVersionString(pBlock, "FileDescription");
      DisplayVersionString(pBlock, "FileVersion");
//      DisplayVersionString(pBlock, "InternalName");
//      DisplayVersionString(pBlock, "LegalCopyright");
//      DisplayVersionString(pBlock, "OriginalFilename");
//      DisplayVersionString(pBlock, "ProductName");
//      DisplayVersionString(pBlock, "ProductVersion");
//      DisplayVersionString(pBlock, "Comments");
//      DisplayVersionString(pBlock, "LegalTrademarks");
//      DisplayVersionString(pBlock, "PrivateBuild");
//      DisplayVersionString(pBlock, "SpecialBuild");
   }

   delete [] pBlock;
}

static void DescribeDXVersion(const char* component)
{
   DWORD dwHandle = 0;
   char  szFile[512];
   char  szWinDir[512];

   GetSystemDirectory(szWinDir, 512);

   sprintf(szFile, "%s\\%s.dll", szWinDir, component);

   int nBytes = GetFileVersionInfoSize(szFile, &dwHandle);

   if (nBytes <= 0) {
      return;
   }

   LPBYTE pBlock = new(__FILE__,__LINE__) BYTE[nBytes];

   if (pBlock && GetFileVersionInfo(szFile, dwHandle, nBytes, (LPVOID) pBlock)) {
      GetTranslation(pBlock);

      char     txt[256];
      char     sFullSection[256];
      LPBYTE   sData = NULL;
      UINT     lenData = 0;
      DWORD    dwErr = 0;

      sprintf(sFullSection, "\\StringFileInfo\\%s\\FileVersion", sTranslation);

      if (VerQueryValue(pBlock, sFullSection, (LPVOID*) &sData, &lenData)) {
         if (lenData && sData) {
            sprintf(txt, "%-8s%s", component, sData);
            Print("|   %-64s |\n", txt);
         }
      }
   }

   delete [] pBlock;
}
/*  Project nGen
    John DiCamillo Software Consulting
    Copyright © 1997. All Rights Reserved.

    SUBSYSTEM:    DataFile.exe
    FILE:         main.cpp
    AUTHOR:       John DiCamillo

*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <windows.h>
#include <windowsx.h>
#include "Archive.h"

//#define MOD_MAKER 1

// +------------------------------------------------------------------+

void insertFile(DataArchive& a, const char* sPath, WIN32_FIND_DATA* find)
{
   char     sFile[256];
   char     sFlat[256];
   DWORD    find_attrib_forbidden =
                        FILE_ATTRIBUTE_DIRECTORY |
                        FILE_ATTRIBUTE_HIDDEN    |
                        FILE_ATTRIBUTE_SYSTEM    |
                        FILE_ATTRIBUTE_OFFLINE;

   if (sPath && *sPath)
      sprintf(sFile, "%s/%s", sPath, find->cFileName);
   else
      strcpy(sFile, find->cFileName);

   if (find->dwFileAttributes & find_attrib_forbidden) {
      printf("   Skipping:  %-48s \n", sFile);
      return;
   }

   int n = strlen(sFile);

   if (n >= NAMELEN) {
      printf("   Skipping:  %-48s (NAME TOO LONG!)\n", sFile);
      return;
   }

   for (int i = 0; i < n; i++)
      sFlat[i] = tolower(sFile[i]);

   if (strstr(sFlat, ".exe")) {
      printf("   Skipping:  %-48s (executable file)\n", sFile);
   }
   else if (strstr(sFlat, ".cmd")) {
      printf("   Skipping:  %-48s (executable file)\n", sFile);
   }
   else if (strstr(sFlat, ".bat")) {
      printf("   Skipping:  %-48s (executable file)\n", sFile);
   }
   else if (strstr(sFlat, ".bin")) {
      printf("   Skipping:  %-48s (unknown file)\n", sFile);
   }
   else if (strstr(sFlat, ".db")) {
      printf("   Skipping:  %-48s (unknown file)\n", sFile);
   }
   else if (strstr(sFlat, ".dat")) {
      printf("   Skipping:  %-48s (data file)\n", sFile);
   }
   else if (strstr(sFlat, ".zip")) {
      printf("   Skipping:  %-48s (zip file)\n", sFile);
   }
   else if (strstr(sFlat, ".arc")) {
      printf("   Skipping:  %-48s (archive file)\n", sFile);
   }
   else if (strstr(sFlat, ".psd")) {
      printf("   Skipping:  %-48s (PSD file)\n", sFile);
   }
   else {
      a.Insert(sFile);
   }
}

// +------------------------------------------------------------------+

void ins(DataArchive& a, int argc, char* argv[])
{
   char  sPath[256];
   char* pDirSep = 0;

   for (int i = 0; i < argc; i++) {
      if (strchr(argv[i], '*')) {
         strcpy(sPath, argv[i]);

         if ((pDirSep = strrchr(sPath, '\\')) != 0)
            *pDirSep = 0;

         else if ((pDirSep = strrchr(sPath, '/')) != 0)
            *pDirSep = 0;

         else
            sPath[0] = 0;

         WIN32_FIND_DATA   find;
         HANDLE h = FindFirstFile(argv[i], &find);
         if (h != INVALID_HANDLE_VALUE) {
            insertFile(a, sPath, &find);

            while (FindNextFile(h,&find)) {
               insertFile(a, sPath, &find);
            }

            FindClose(h);
         }
      }
      else {
         a.Insert(argv[i]);
      }
   }
}

// +--------------------------------------------------------------------+

void build(DataArchive& a, const char* sBasePath);

void buildFile(DataArchive& a, const char* sPath, WIN32_FIND_DATA& find)
{
   if (find.cFileName[0] == '.') {
   }

   else if (find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
      char subdir[256];
      if (sPath && *sPath)
         sprintf(subdir, "%s/%s", sPath, find.cFileName);
      else
         sprintf(subdir, "%s", find.cFileName);

      build(a, subdir);
   }

   else {
      insertFile(a, sPath, &find);
   }
}

void build(DataArchive& a, const char* sBasePath)
{
   char  sPath[256];
   char  sFind[256];

   if (sBasePath && *sBasePath) {
      strcpy(sPath, sBasePath);
      sprintf(sFind, "%s\\*.*", sPath);
   }
   else {
      sPath[0] = 0;
      strcpy(sFind, "*.*");
   }

   WIN32_FIND_DATA   find;
   HANDLE h = FindFirstFile(sFind, &find);
   if (h != INVALID_HANDLE_VALUE) {
      do 
         buildFile(a, sPath, find);

      while (FindNextFile(h, &find));

      FindClose(h);
   }
}

void mak(DataArchive& a)
{
   build(a, 0);
}

// +--------------------------------------------------------------------+
// for now, pattern must be either "*" or "*.???"

int match(const char* sFile, const char* sPattern)
{
   int   nPatternType = 0;
   char* sExt = 0;

   const int PATTERN_NOWILD         = 0;
   const int PATTERN_STAR           = 1;
   const int PATTERN_STAR_DOT_STAR  = 2;
   const int PATTERN_STAR_DOT_EXT   = 3;

   // what kind of pattern matching?
   if (strchr(sPattern, '*')) {
      if (strchr(sPattern, '.')) {
         if (strcmp(sPattern, "*.*") == 0) {
            nPatternType = PATTERN_STAR_DOT_STAR;
         }
         else {
            nPatternType = PATTERN_STAR_DOT_EXT;
            sExt = strchr(sPattern, '.');
         }
      }

      else {
         nPatternType = PATTERN_STAR;
      }
   }

   int file_matches_pattern = 0;

   switch (nPatternType) {
   case PATTERN_NOWILD:
   default:
      file_matches_pattern = (stricmp(sFile, sPattern) == 0);
      break;


   case PATTERN_STAR:
   case PATTERN_STAR_DOT_STAR:
      file_matches_pattern = 1;
      break;

   case PATTERN_STAR_DOT_EXT:
      file_matches_pattern = (strstr(sFile, sExt) != 0);
      break;
   }

   return file_matches_pattern;
}

void ext(DataArchive& a, int argc, char* argv[])
{
   if (argc) {
      char  sPath[256];
      char  sPatt[256];
      char* pDirSep;
      int   nPath;

      for (int i = 0; i < argc; i++) {
         if (strchr(argv[i], '*')) {
            strcpy(sPath, argv[i]);

            if ((pDirSep = strrchr(sPath, '\\')) != 0) {
               strcpy(sPatt, pDirSep+1);
               *pDirSep = 0;
               nPath = strlen(sPath);
            }

            else if ((pDirSep = strrchr(sPath, '/')) != 0) {
               strcpy(sPatt, pDirSep+1);
               *pDirSep = 0;
               nPath = strlen(sPath);
            }

            else {
               strcpy(sPatt, sPath);
               sPath[0] = 0;
               nPath    = 0;
            }

            // for each file in the archive:
            for (unsigned j = 0; j < a.NumFiles(); j++) {
               DataEntry* pde = a.GetFile(j);

               if (pde) {
                  // if we are extracting from a sub-directory,
                  if (nPath) {
                     // and this file is in the sub-directory,
                     if (strnicmp(pde->name, sPath, nPath) == 0) {
                        // and this file matches the pattern:
                        if (match(pde->name+nPath+1, sPatt)) {
                           char sName[256];
                           strcpy(sName, pde->name);
                           a.Extract(sName);
                        }
                     }
                  }

                  // if we are extracting from the main directory,
                  else {
                     // and this file is in the main directory,
                     if (strchr(pde->name, '/') == 0) {
                        // and this file matches the pattern:
                        if (match(pde->name, sPatt)) {
                           char sName[256];
                           strcpy(sName, pde->name);
                           a.Extract(sName);
                        }
                     }
                  }
               }
            }
         }

         else {
            // for each file in the archive:
            for (unsigned j = 0; j < a.NumFiles(); j++) {
               DataEntry* pde = a.GetFile(j);

               if (pde) {
                  if (stricmp(pde->name, argv[i]) == 0) {
                     a.Extract(argv[i]);
                  }
               }
            }
         }
      }
   }

   // full archive extraction:
   else {
      for (int i = 0; i < (int)a.NumFiles(); i++)
         a.Extract(a.GetFile(i)->name);
   }
}

void del(DataArchive& a, int argc, char* argv[])
{
   char  sPath[256];
   char  sPatt[256];
   char* pDirSep;
   int   nPath;

   for (int i = 0; i < argc; i++) {
      if (strchr(argv[i], '*')) {
         strcpy(sPath, argv[i]);

         if ((pDirSep = strrchr(sPath, '\\')) != 0) {
            strcpy(sPatt, pDirSep+1);
            *pDirSep = 0;
            nPath = strlen(sPath);
         }

         else if ((pDirSep = strrchr(sPath, '/')) != 0) {
            strcpy(sPatt, pDirSep+1);
            *pDirSep = 0;
            nPath = strlen(sPath);
         }

         else {
            strcpy(sPatt, sPath);
            sPath[0] = 0;
            nPath    = 0;
         }

         // for each file in the archive:
         for (unsigned j = 0; j < a.NumFiles(); j++) {
            DataEntry* pde = a.GetFile(j);

            if (pde) {
               // if we are deleting from a sub-directory,
               if (nPath) {
                  // and this file is in the sub-directory,
                  if (strnicmp(pde->name, sPath, nPath) == 0) {
                     // and this file matches the pattern:
                     if (match(pde->name+nPath+1, sPatt)) {
                        char sName[256];
                        strcpy(sName, pde->name);
                        a.Remove(sName);
                     }
                  }
               }

               // if we are deleting from the main directory,
               else {
                  // and this file is in the main directory,
                  if (strchr(pde->name, '/') == 0) {
                     // and this file matches the pattern:
                     if (match(pde->name, sPatt)) {
                        char sName[256];
                        strcpy(sName, pde->name);
                        a.Remove(sName);
                     }
                  }
               }
            }
         }
      }

      else {
         a.Remove(argv[i]);
      }
   }
}


// +--------------------------------------------------------------------+

void Usage()
{
   printf("Usage: datafile <dat-file> -option <file list>\n");
   printf("options:  -ins  (insert files into datafile)\n");
   printf("          -ext  (extract files from datafile)\n");
   printf("          -del  (delete files from datafile)\n");
   printf("          -mak  (insert all files in current directory and all subdirectories)\n");
   printf("          -list (display list of entries in datafile)\n");
   
   exit(-1);
}

#define OPT_NONE 0
#define OPT_INS  1
#define OPT_EXT  2
#define OPT_DEL  3
#define OPT_MAK  4
#define OPT_LIST 5

int main(int argc, char* argv[])
{
#ifdef MOD_MAKER
   printf("MODFILE\n");

   if (argc < 2) {
      printf("Usage: modfile <dat-file>\n");
      return 0;
   }

   ::unlink(argv[1]);
   DataArchive a(argv[1]);
   mak(a);

   return 0;
#else
   printf("DATAFILE\n");

   if (argc < 3)
      Usage();

   DataArchive a(argv[1]);
   int option = OPT_NONE;

   if      (!stricmp(argv[2], "-ins"))    option = OPT_INS;
   else if (!stricmp(argv[2], "-ext"))    option = OPT_EXT;
   else if (!stricmp(argv[2], "-del"))    option = OPT_DEL;
   else if (!stricmp(argv[2], "-mak"))    option = OPT_MAK;
   else if (!stricmp(argv[2], "-list"))   option = OPT_LIST;

   argc -= 3;
   argv += 3;

   switch (option) {
   default:
   case OPT_NONE:    Usage();             break;
   case OPT_INS:     ins(a, argc, argv);  break;
   case OPT_EXT:     ext(a, argc, argv);  break;
   case OPT_DEL:     del(a, argc, argv);  break;
   case OPT_MAK:     mak(a);              break;
   case OPT_LIST:    a.List();            break;
   }
#endif

   return 0;
}


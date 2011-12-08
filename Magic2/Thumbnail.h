/*  Project Magic 2.0
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Magic.exe
    FILE:         Thumbnail.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Function declarations for thumbnail img previewer
*/

#ifndef Thumbnail_h
#define Thumbnail_h

#ifndef __AFXWIN_H__
   #error include 'stdafx.h' before including 'Thumbnail.h' file for PCH
#endif

// +--------------------------------------------------------------------+

class Bitmap;

// +--------------------------------------------------------------------+

void ThumbPreview(HWND hprev, Bitmap* bitmap);

#endif Thumbnail_h


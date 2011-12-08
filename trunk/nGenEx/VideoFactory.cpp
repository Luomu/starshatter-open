/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib
    FILE:         VideoFac.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Video and Polygon Renderer Factory class
*/

#include "MemDebug.h"
#include "VideoFactory.h"

#include "VideoDX9.h"
#include "SoundD3D.h"

// +--------------------------------------------------------------------+

VideoFactory::VideoFactory(HWND h)
   : hwnd(h), video(0), audio(0)
{ }

VideoFactory::~VideoFactory()
{ }

// +--------------------------------------------------------------------+

Video*
VideoFactory::CreateVideo(VideoSettings* vs)
{
   if (!video) {
      video = (Video*) new(__FILE__,__LINE__) VideoDX9(hwnd, vs);
      
      if (!video) {
         delete video;
         video = 0;
      }
   }
   
   return video;
}

// +--------------------------------------------------------------------+

void
VideoFactory::DestroyVideo(Video* v)
{
   if (v == video) {
      delete video;
      video = 0;
   }
}

// +--------------------------------------------------------------------+

SoundCard*
VideoFactory::CreateSoundCard()
{
   if (!audio) {
      audio = new(__FILE__,__LINE__) SoundCardD3D(hwnd);
      Sound::UseSoundCard(audio);
   }
   
   return audio;
}



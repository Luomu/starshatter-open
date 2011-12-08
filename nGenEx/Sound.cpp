/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib
    FILE:         Sound.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Abstract sound class
*/

#include "MemDebug.h"
#include "Sound.h"
#include "SoundCard.h"
#include "Wave.h"

#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

// +--------------------------------------------------------------------+

SoundCard* Sound::creator = 0;

Sound*
Sound::CreateStream(const char* filename)
{
   Sound* sound = 0;

   if (!filename || !filename[0] || !creator)
      return sound;

   int namelen = strlen(filename);

   if (namelen < 5)
      return sound;

   if ((filename[namelen-3] == 'o' || filename[namelen-3] == 'O') &&
       (filename[namelen-2] == 'g' || filename[namelen-2] == 'G') &&
       (filename[namelen-1] == 'g' || filename[namelen-1] == 'G')) {

      return CreateOggStream(filename);
   }

   WAVE_HEADER    head;
   WAVE_FMT       fmt;
   WAVE_FACT      fact;
   WAVE_DATA      data;
   WAVEFORMATEX   wfex;

   ZeroMemory(&head, sizeof(head));
   ZeroMemory(&fmt,  sizeof(fmt));
   ZeroMemory(&fact, sizeof(fact));
   ZeroMemory(&data, sizeof(data));

   LPBYTE         buf   = 0;
   LPBYTE         p     = 0;
   int            len   = 0;

   FILE* f = ::fopen(filename, "rb");

   if (f) {
      fseek(f, 0, SEEK_END);
      len = ftell(f);
      fseek(f, 0, SEEK_SET);

      if (len > 4096) {
         len = 4096;
      }

      buf = new(__FILE__,__LINE__) BYTE[len];

      if (buf && len)
         fread(buf, len, 1, f);

      fclose(f);
   }


   if (len > sizeof(head)) {
      CopyMemory(&head, buf, sizeof(head));

      if (head.RIFF == MAKEFOURCC('R', 'I', 'F', 'F') &&
          head.WAVE == MAKEFOURCC('W', 'A', 'V', 'E')) {

         p = buf + sizeof(WAVE_HEADER);

         do {
            DWORD chunk_id = *((LPDWORD) p);

            switch (chunk_id) {
            case MAKEFOURCC('f', 'm', 't', ' '):
               CopyMemory(&fmt, p, sizeof(fmt));
               p += fmt.chunk_size + 8;
               break;

            case MAKEFOURCC('f', 'a', 'c', 't'):
               CopyMemory(&fact, p, sizeof(fact));
               p += fact.chunk_size + 8;
               break;

            case MAKEFOURCC('s', 'm', 'p', 'l'):
               CopyMemory(&fact, p, sizeof(fact));
               p += fact.chunk_size + 8;
               break;

            case MAKEFOURCC('d', 'a', 't', 'a'):
               CopyMemory(&data, p, sizeof(data));
               p += 8;
               break;

            default:
               delete buf;
               return sound;
            }
         }
         while (data.chunk_size == 0);

         wfex.wFormatTag      = fmt.wFormatTag;
         wfex.nChannels       = fmt.nChannels;
         wfex.nSamplesPerSec  = fmt.nSamplesPerSec;
         wfex.nAvgBytesPerSec = fmt.nAvgBytesPerSec;
         wfex.nBlockAlign     = fmt.nBlockAlign;
         wfex.wBitsPerSample  = fmt.wBitsPerSample;
         wfex.cbSize          = 0;

         sound = Create(Sound::STREAMED, &wfex);

         if (sound) {
            sound->SetFilename(filename);
            sound->StreamFile(filename, p - buf);
         }
      }
   }

   delete buf;
   return sound;
}

// +--------------------------------------------------------------------+

Sound*
Sound::CreateOggStream(const char* filename)
{
   Sound* sound = 0;

   if (!filename || !filename[0] || !creator)
      return sound;

   int namelen = strlen(filename);

   if (namelen < 5)
      return sound;

   WAVEFORMATEX wfex;
   ZeroMemory(&wfex, sizeof(wfex));

   FILE* f = ::fopen(filename, "rb");

   if (f) {
      OggVorbis_File* povf = new(__FILE__,__LINE__) OggVorbis_File;

      if (!povf) {
         Print("Sound::CreateOggStream(%s) - out of memory!\n", filename);
         return sound;
      }

      ZeroMemory(povf, sizeof(OggVorbis_File));

      if (ov_open(f, povf, NULL, 0) < 0) {
         Print("Sound::CreateOggStream(%s) - not an Ogg bitstream\n", filename);
         delete povf;
         return sound;
      }

      Print("\nOpened Ogg Bitstream '%s'\n", filename);
      char **ptr=ov_comment(povf,-1)->user_comments;
      vorbis_info *vi=ov_info(povf,-1);
      while(*ptr){
         Print("%s\n", *ptr);
         ++ptr;
      }

      Print("Bitstream is %d channel, %ldHz\n", vi->channels, vi->rate);
      Print("Decoded length: %ld samples\n",
	       (long)ov_pcm_total(povf,-1));
      Print("Encoded by: %s\n\n", ov_comment(povf,-1)->vendor);

      wfex.wFormatTag      = WAVE_FORMAT_PCM;
      wfex.nChannels       = vi->channels;
      wfex.nSamplesPerSec  = vi->rate;
      wfex.nAvgBytesPerSec = vi->channels * vi->rate * 2;
      wfex.nBlockAlign     = vi->channels * 2;
      wfex.wBitsPerSample  = 16;
      wfex.cbSize          = 0;

      sound = Create(Sound::STREAMED | Sound::OGGVORBIS,
                     &wfex,
                     sizeof(OggVorbis_File),
                     (LPBYTE) povf);

      sound->SetFilename(filename);
   }

   return sound;
}

// +--------------------------------------------------------------------+

Sound*
Sound::Create(DWORD flags, LPWAVEFORMATEX format)
{
   if (creator) return creator->CreateSound(flags, format);
   else         return 0;
}

Sound*
Sound::Create(DWORD flags, LPWAVEFORMATEX format, DWORD len, LPBYTE data)
{
   if (creator) return creator->CreateSound(flags, format, len, data);
   else         return 0;
}

void
Sound::SetListener(const Camera& cam, const Vec3& vel)
{
   if (creator)
      creator->SetListener(cam, vel);
}

// +--------------------------------------------------------------------+

Sound::Sound()
   : status(UNINITIALIZED), volume(0), flags(0), looped(0),
     velocity(0,0,0), location(0,0,0), sound_check(0)
{
   strcpy(filename, "Sound()");
}

// +--------------------------------------------------------------------+

Sound::~Sound()
{ }

// +--------------------------------------------------------------------+

void
Sound::Release()
{
   flags &= ~LOCKED;
}

// +--------------------------------------------------------------------+

void
Sound::AddToSoundCard()
{
   if (creator)
      creator->AddSound(this);
}

// +--------------------------------------------------------------------+

void
Sound::SetFilename(const char* s)
{
   if (s) {
      int n = strlen(s);

      if (n >= 60) {
         ZeroMemory(filename, sizeof(filename));
         strcpy(filename, "...");
         strcat(filename, s + n - 59);
         filename[63] = 0;
      }

      else {
         strcpy(filename, s);
      }
   }
}


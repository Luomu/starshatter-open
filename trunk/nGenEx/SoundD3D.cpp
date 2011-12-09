/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib
    FILE:         SoundD3D.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    DirectSound and DirectSound3D (sound card) class
*/

//#define INITGUID
#include <objbase.h>
#include <cguid.h>
#include <mmsystem.h>
#include <dsound.h>


#include "MemDebug.h"
#include "SoundD3D.h"
#include "Game.h"

#ifdef DIRECT_SOUND_3D
#include "ia3d.h"
#endif

// +--------------------------------------------------------------------+

void  Print(const char* msg, ...);
char* DSErrStr(HRESULT dserr);
void  SoundD3DError(const char* msg, HRESULT dserr);

static int           DS3D_report_errors = 1;

#ifndef RELEASE
#define RELEASE(x) if (x) { x->Release(); x=NULL; }
#endif

// +====================================================================+
// | SOUND CARD D3D
// +====================================================================+

SoundCardD3D::SoundCardD3D(HWND hwnd)
   : soundcard(0), primary(0)
{
   HRESULT  err = 0;
   status = SC_ERROR;

   // 1) Get interface to DirectSound object:

#ifdef DIRECT_SOUND_3D
   CoInitialize(NULL);
   err = CoCreateInstance(CLSID_A3d, NULL, CLSCTX_INPROC_SERVER,  
			                 IID_IDirectSound, (VOID **)&soundcard);

   if (SUCCEEDED(err)) {
      soundcard->Initialize(NULL);
      SoundD3DError("Initialized Aureal3D Sound", 0);
   }
   else {
      SoundD3DError("Could not initialize Aureal3D Sound", err);
      SoundD3DError("Proceding with standard DirectSoundCreate", 0);
#endif      

      err = DirectSoundCreate(0, &soundcard, 0);
      if (FAILED(err)) {
         SoundD3DError("Could not create DirectSound object.", err);
         soundcard = 0;
         primary   = 0;
         return;
      }

#ifdef DIRECT_SOUND_3D
   }
#endif
   
   // 2) Set the cooperative level:
   err = soundcard->SetCooperativeLevel(hwnd, DSSCL_PRIORITY);
   if (FAILED(err)) {
      SoundD3DError("Could not set cooperative level.", err);
      RELEASE(soundcard);
      return;
   }

   // Prepare to initialize the primary buffer:
   DSCAPS caps;
   memset(&caps, 0, sizeof(caps));
   caps.dwSize    = sizeof(caps);
   
   err = soundcard->GetCaps(&caps);
   if (FAILED(err)) {
      SoundD3DError("Could not get soundcard caps.", err);
      RELEASE(soundcard);
      return;
   }

   if (caps.dwFlags & DSCAPS_EMULDRIVER)
      Print("   WARNING: using DirectSound emulated drivers\n");

   memset(&dsbd, 0, sizeof(dsbd));
   dsbd.dwSize    = sizeof(dsbd);

#ifdef DIRECT_SOUND_3D
   int use_ds3d = true;

   // 3) Set up the primary buffer (try to use DS3D):
   dsbd.dwFlags   = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRL3D;

   err = soundcard->CreateSoundBuffer(&dsbd, &primary, 0);
   if (err == DSERR_CONTROLUNAVAIL) {
      use_ds3d = false;

      // try again, without using DS3D
      dsbd.dwFlags   = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLDEFAULT;

      err = soundcard->CreateSoundBuffer(&dsbd, &primary, 0);
      if (FAILED(err)) {
         SoundD3DError("Could not initialize primary buffer", err);
         RELEASE(soundcard);
         return;
      }
      else {
         Print("   WARNING: DirectSound3D is not available, simulating instead\n");
      }
   }

   // 4) Set up the listener:
   if (primary && use_ds3d) {
      err = primary->QueryInterface(IID_IDirectSound3DListener, (void**)&listener);
      if (FAILED(err)) {
         SoundD3DError("Could not get listener interface", err);
      }
      else {
         listener->SetPosition(0.0f, 0.0f, 0.0f, DS3D_IMMEDIATE);
         listener->SetOrientation(0.0f, 0.0f, 1.0f,
                                  0.0f, 1.0f, 0.0f, DS3D_IMMEDIATE);
      }
   }

#else
   // 3) Set up the primary buffer:
   dsbd.dwFlags   = DSBCAPS_PRIMARYBUFFER;

   err = soundcard->CreateSoundBuffer(&dsbd, &primary, 0);
   if (FAILED(err)) {
      SoundD3DError("Could not initialize primary buffer", err);
      RELEASE(soundcard);
      return;
   }
#endif

   // 5) Set primary buffer parameters to 16 bit STEREO at 44kHz
   SetFormat(16, 2, 44100);

   // read back the result format and display to the log file:
   GetFormat(0);
   ShowFormat();

   status = SC_OK;
}

// +--------------------------------------------------------------------+

SoundCardD3D::~SoundCardD3D()
{
   ListIter<Sound> iter = sounds;
   while (++iter) {
      Sound* s = iter.value();
      s->Stop();
   }

   sounds.destroy();

#ifdef DIRECT_SOUND_3D
   RELEASE(listener);
#endif

   RELEASE(primary);
   RELEASE(soundcard);

   Print("   SoundCardD3D: shutdown\n");

   status = SC_UNINITIALIZED;
}

// +--------------------------------------------------------------------+

bool
SoundCardD3D::SetFormat(int bits, int channels, int hertz)
{
   if (!soundcard) return false;

   DSCAPS caps;
   memset(&caps, 0, sizeof(caps));
   caps.dwSize    = sizeof(caps);
   soundcard->GetCaps(&caps);

   if (!(caps.dwFlags & DSCAPS_PRIMARY16BIT))  bits     = 8;
   if (!(caps.dwFlags & DSCAPS_PRIMARYSTEREO)) channels = 1;

   memset(&wfex, 0, sizeof(wfex));

   wfex.wFormatTag      = WAVE_FORMAT_PCM;
   wfex.nChannels       = channels;
   wfex.nSamplesPerSec  = hertz;
   wfex.nBlockAlign     = (channels * bits) / 8;
   wfex.nAvgBytesPerSec = wfex.nSamplesPerSec * wfex.nBlockAlign;
   wfex.wBitsPerSample  = bits;
   
   return SetFormat(&wfex);
}

// +--------------------------------------------------------------------+

bool
SoundCardD3D::SetFormat(LPWAVEFORMATEX format)
{
   HRESULT err = E_FAIL;

   if (primary)
      err = primary->SetFormat(format);

   return SUCCEEDED(err);
}

// +--------------------------------------------------------------------+

bool
SoundCardD3D::GetFormat(LPWAVEFORMATEX format)
{
   if (!format) format = &wfex;

   HRESULT err = E_FAIL;

   if (primary)
      err = primary->GetFormat(format, sizeof(WAVEFORMATEX), 0);

   return SUCCEEDED(err);
}

// +--------------------------------------------------------------------+

void
SoundCardD3D::ShowFormat()
{
   Print("   SoundCardD3D Primary Buffer Format:\n");
   Print("     bits: %d\n",   wfex.wBitsPerSample);
   Print("     chls: %d\n",   wfex.nChannels);
   Print("     rate: %d\n\n", wfex.nSamplesPerSec);
}

// +--------------------------------------------------------------------+

Sound*
SoundCardD3D::CreateSound(DWORD flags, LPWAVEFORMATEX format)
{
   if (!soundcard) return 0;

   Sound* result = new(__FILE__,__LINE__) SoundD3D(soundcard, flags, format);
   if (result) AddSound(result);
   return result;
}

// +--------------------------------------------------------------------+

Sound*
SoundCardD3D::CreateSound(DWORD flags, LPWAVEFORMATEX format, DWORD len, LPBYTE data)
{
   if (!soundcard) return 0;
   Sound* result = new(__FILE__,__LINE__) SoundD3D(soundcard, flags, format, len, data);

   if (flags & (Sound::STREAMED | Sound::OGGVORBIS)) {
      if (result)
         AddSound(result);
   }

   return result;
}

// +--------------------------------------------------------------------+

void
SoundCardD3D::SetListener(const Camera& cam, const Vec3& vel)
{
   Point pos = cam.Pos();

#ifdef DIRECT_SOUND_3D
   listener->SetPosition((float) pos.x, (float) pos.z, (float) pos.y, DS3D_IMMEDIATE);
   listener->SetOrientation((float) cam.vpn().x, (float) cam.vpn().y, (float) cam.vpn().z,
                            (float) cam.vup().x, (float) cam.vup().y, (float) cam.vup().z,
                            DS3D_IMMEDIATE);
   listener->SetVelocity(vel.x, vel.y, vel.z, DS3D_IMMEDIATE);
#else
   listener.Clone(cam);
   listener.MoveTo(pos.x, pos.z, pos.y);
   velocity = vel;
#endif
}

// +--------------------------------------------------------------------+

bool
SoundCardD3D::Pause()
{
   AutoThreadSync a(sync);

   ListIter<Sound> iter = sounds;
   while (++iter) {
      Sound* s = iter.value();

      if ((s->GetFlags() & Sound::INTERFACE) == 0)
         s->Pause();
   }
   
   return true;
}

// +--------------------------------------------------------------------+

bool
SoundCardD3D::Resume()
{
   AutoThreadSync a(sync);

   ListIter<Sound> iter = sounds;
   while (++iter) {
      Sound* s = iter.value();

      if (s->IsReady())
         s->Play();
   }
   
   return true;
}

// +--------------------------------------------------------------------+

bool
SoundCardD3D::StopSoundEffects()
{
   AutoThreadSync a(sync);

   DWORD ok_sounds = (Sound::INTERFACE | Sound::OGGVORBIS | Sound::RESOURCE);

   ListIter<Sound> iter = sounds;
   while (++iter) {
      Sound* s = iter.value();

      if ((s->GetFlags() & ok_sounds) == 0)
         s->Stop();
   }
   
   return true;
}

// +====================================================================+
// | SOUND D3D
// +====================================================================+

SoundD3D::SoundD3D(LPDIRECTSOUND card, DWORD flag_req, LPWAVEFORMATEX format)
   : soundcard(card), buffer(0), min_dist(1.0f), max_dist(100000.0f),
     stream(0), stream_left(0), min_safety(0), read_size(0), transfer(0), w(0), r(0),
     stream_offset(0), data_len(0), data(0), moved(false), eos_written(false), eos_latch(0),
     ov_file(0), total_time(0)
{
   flags = flag_req;

   CopyMemory(&wfex, format, sizeof(wfex));
   ZeroMemory(&dsbd, sizeof(dsbd));

   dsbd.dwSize  = sizeof(dsbd);
   dsbd.dwFlags = DSBCAPS_CTRLVOLUME /* | DSBCAPS_GETCURRENTPOSITION2 */;
   dsbd.lpwfxFormat = &wfex;

#ifdef DIRECT_SOUND_3D
   sound3d = 0;
   if (flags & LOCALIZED)
      if (flags & LOC_3D)
         dsbd.dwFlags |= DSBCAPS_CTRL3D;
      else
         dsbd.dwFlags |= DSBCAPS_CTRLPAN | DSBCAPS_CTRLFREQUENCY;
#else
   dsbd.dwFlags |= DSBCAPS_LOCSOFTWARE;

   if (flags & LOCALIZED)
      dsbd.dwFlags |= DSBCAPS_CTRLPAN | DSBCAPS_CTRLFREQUENCY;
#endif

}

// +--------------------------------------------------------------------+
// SOUND RESOURCE CONSTRUCTOR:
// (Now also used to create Ogg Vorbis streaming sound objects)

SoundD3D::SoundD3D(LPDIRECTSOUND card, DWORD flag_req, LPWAVEFORMATEX format, DWORD len, LPBYTE pData)
   : soundcard(card), buffer(0), min_dist(1.0f), max_dist(100000.0f),
     stream(0), stream_left(0), min_safety(0), read_size(0), transfer(0), w(0), r(0),
     stream_offset(0), data_len(0), data(0), moved(false), eos_written(false), eos_latch(0),
     ov_file(0)
{
   flags = flag_req;

   if (!(flags & (STREAMED | OGGVORBIS)))
      flags = flag_req | RESOURCE;

   CopyMemory(&wfex, format, sizeof(wfex));
   ZeroMemory(&dsbd, sizeof(dsbd));

   dsbd.dwSize  = sizeof(dsbd);
   dsbd.dwFlags = DSBCAPS_CTRLVOLUME /* | DSBCAPS_GETCURRENTPOSITION2 */;
   dsbd.lpwfxFormat = &wfex;

#ifdef DIRECT_SOUND_3D
   sound3d = 0;
   if (flags & LOCALIZED)
      if (flags & LOC_3D)
         dsbd.dwFlags |= DSBCAPS_CTRL3D;
      else
         dsbd.dwFlags |= DSBCAPS_CTRLPAN | DSBCAPS_CTRLFREQUENCY;
#else
   if (flags & LOCALIZED)
      dsbd.dwFlags |= DSBCAPS_CTRLPAN | DSBCAPS_CTRLFREQUENCY;
#endif

   if (len) {
      // If this is an OGG VORBIS streaming sound,
      // the parameter that normally points to the actual data
      // is used to pass in an Ogg Vorbis file structure instead:

      if (flags & OGGVORBIS) {
         ov_file = (OggVorbis_File*) pData;
         StreamOggFile();
      }

      else {
         data_len = len;
         data     = new(__FILE__,__LINE__) BYTE[len];

         if (!data) {
            data_len = 0;
         }

         else {
            CopyMemory(data, pData, data_len);
            Load(data_len, data);
         }
      }
   }
}

// +--------------------------------------------------------------------+

SoundD3D::~SoundD3D()
{
   delete [] data;
   delete [] transfer;

   if (ov_file) {
      ov_clear(ov_file);
      delete ov_file;
   }

   else if (stream) {
      fclose(stream);
   }

   RELEASE(buffer);
   
#ifdef DIRECT_SOUND_3D
   RELEASE(sound3d);
#endif
}

// +--------------------------------------------------------------------+

void
SoundD3D::Update()
{
   if (!buffer || status != PLAYING) return; // nothing to do
   
   DWORD dstat;
   HRESULT hr = buffer->GetStatus(&dstat);
   if (FAILED(hr)) {
      SoundD3DError("Update: GetStatus failed", hr);
      return;
   }

   AutoThreadSync a(sync);

   if (sound_check) sound_check->Update(this);

   if (!Game::Paused() || flags & STREAMED) {
      // see if we are done:
      if (!(dstat & DSBSTATUS_PLAYING)) {
         status = DONE;
         buffer->Stop();
         return;
      }
   
      // if not done, see if we need to change location params:
      if (moved) {
         Localize();
      }
   }

   // if not done, see if we need to load more data
   // into the streaming buffer:
   if (flags & STREAMED) {
      buffer->GetCurrentPosition(&r, 0);
      
      DWORD data_left;
      if (w > r)
         data_left = w - r;
      else
         data_left = w + (read_size + min_safety) - r;

      // getting low, fill 'er up:
      if (eos_written || data_left <= min_safety) {
         StreamBlock();

         if (stream_left == 0) {
            // if this is the end of a looping stream,
            if (flags & LOOP) {
               RewindStream();
               looped++;
            }
            else {
               if (!eos_written) {
                  eos_written = true;
                  eos_latch   = 3;
               }

               else if (--eos_latch == 0) {
                  status = DONE;
                  buffer->Stop();
               }
            }
         }

         status = PLAYING;
      }
   }
}

void
SoundD3D::StreamBlock()
{
   if (flags & OGGVORBIS) {
      StreamOggBlock();
      return;
   }

   if (!stream || !stream_left)
      return;

   if (stream_left < read_size) {
      if (stream_left > 0) {
         fread(transfer, stream_left, 1, stream);
         Load(stream_left, transfer);
         stream_left = 0;
      }
   }

   else if (read_size > 0) {
         fread(transfer, read_size, 1, stream);
         Load(read_size, transfer);
         stream_left -= read_size;
   }
}

int ogg_read_error_count = 0;

void
SoundD3D::StreamOggBlock()
{
   int   current_bitstream;
   DWORD bytes_read = 0;
   long  retval     = 0;
   char* p          = (char*) transfer;

   while (stream_left && bytes_read < read_size) {
      retval = ov_read(ov_file, p, read_size-bytes_read, 0,2,1, &current_bitstream);
   
      if (retval == 0) {
         /* EOF */
         stream_left = 0;
      }
      else if (retval < 0) {
         /* error in the stream.  Not a problem, just reporting it in
            case the app cares.  In this case, we don't. */
         ogg_read_error_count++;
      }
      else {
         /* we don't bother dealing with sample rate changes, etc, but you'll have to ??? */
         bytes_read  += retval;
         stream_left -= retval;
         p           += retval;
      }
   }

   if (bytes_read)
      Load(bytes_read, transfer);
}

void
SoundD3D::RewindStream()
{
   if (flags & OGGVORBIS) {
      RewindOggStream();
      return;
   }

   if (!stream || !buffer)
      return;

   // rewind the stream and keep going...
   eos_written = false;
   eos_latch   = 0;
   read_size   = wfex.nAvgBytesPerSec / 2;

   // find the size of the file:
   fseek(stream, 0, SEEK_END);
   stream_left = ftell(stream) - stream_offset;
   fseek(stream, stream_offset, SEEK_SET);

   total_time = (double) stream_left /
                (double) wfex.nAvgBytesPerSec;

   if (stream_left < read_size) {
      status = DONE;
      buffer->Stop();
   }
}

void
SoundD3D::RewindOggStream()
{
   if (!ov_file || !buffer)
      return;

   // rewind the stream and keep going...
   eos_written = false;
   eos_latch   = 0;
   read_size   = wfex.nAvgBytesPerSec / 2;

   // set the stream pointer back to the beginning:
   ov_pcm_seek(ov_file, 0);

   // find the size of the file:
   stream_left    = (DWORD) ov_pcm_total(ov_file,-1);
   stream_offset  = 0;

   total_time     = (double) stream_left /
                    (double) wfex.nAvgBytesPerSec;

   if (stream_left < read_size) {
      status = DONE;
      buffer->Stop();
   }
}

void
SoundD3D::Localize()
{
#ifdef DIRECT_SOUND_3D
   if (sound3d) {
      sound3d->SetMinDistance(min_dist,                        DS3D_IMMEDIATE);
      sound3d->SetMaxDistance(max_dist,                        DS3D_IMMEDIATE);
      sound3d->SetPosition(location.x, location.y, location.z, DS3D_IMMEDIATE);
      sound3d->SetVelocity(velocity.x, velocity.y, velocity.z, DS3D_IMMEDIATE);
   }

#else

   // if no buffer, nothing to do:
   if (!buffer) {
      moved = false;
      return;
   }

   // Compute pan and volume from scratch:
      
   if ((flags & LOC_3D) && creator) {
      Vec3 loc = location;

      SoundCardD3D* ears = (SoundCardD3D*) creator;
      Camera& listener = ears->listener;
      Vec3  ear_loc = listener.Pos();  ear_loc.SwapYZ();
      Vec3  direction = loc - ear_loc;
      
      loc.x = direction * listener.vrt();
      loc.y = direction * listener.vup();
      loc.z = direction * listener.vpn();

      double pan = 10000;
      if (loc.z != 0.0f) pan = fabs(1000.0f * loc.x / loc.z);
      if (pan > 10000)   pan = 10000;
      if (loc.x < 0)     pan = -pan;

      if (volume > 0)
         volume = 0;

      double vol   = volume;
      double mind2 = min_dist * min_dist;
      double maxd2 = max_dist * max_dist;
      double d2    = (loc.x*loc.x) + (loc.y*loc.y) + (loc.z*loc.z);
   
      if (d2 > maxd2)
         vol = -10000;
      else if (d2 > mind2)
         vol -= (d2-mind2)/(maxd2-mind2) * (vol+10000);

      // clamp volume to legal range:
      if (vol < -10000) vol = -10000;
      else if (vol > volume) vol = volume;

      /***
      Print("Localize: ears  = (%f, %f, %f)\n", ear_loc.x, ear_loc.y, ear_loc.z);
      Print("          world = (%f, %f, %f)\n", location.x, location.y, location.z);
      Print("          view  = (%f, %f, %f)\n", loc.x, loc.y, loc.z);
      Print("          Pan=%f  Volume=%f\n", pan, vol);
      /***/

      HRESULT hr = buffer->SetPan((LONG) pan);
      if (!SUCCEEDED(hr)) {
         char warn[512];
         sprintf_s(warn, "Warning could not set pan on buffer to %d", pan);
         SoundD3DError(warn, hr);
      }

      hr = buffer->SetVolume((LONG) vol);
      if (!SUCCEEDED(hr)) {
         char warn[512];
         sprintf_s(warn, "Warning: could not set volume on buffer to %d", vol);
         SoundD3DError(warn, hr);
      }
      
      // if not too far to hear...
      if ((flags & DOPPLER) && (d2 < maxd2)) {
         // COMPUTE DOPPLER SHIFT:
         const float c = 10000.0f;
         
         direction.Normalize();
         float v_L = ears->velocity * direction;
         float v_S =       velocity * direction;
         
         DWORD f_shift = wfex.nSamplesPerSec;

         if (v_L != v_S) {
            // towards listener:
            if (v_S < 0)
               f_shift = wfex.nSamplesPerSec + 20;
            else
               f_shift = wfex.nSamplesPerSec - 20;
         }

         // distance rolloff of high frequencies:
         double dist     = sqrt(d2);
         DWORD  roll_off = (DWORD) (80 * dist/max_dist);

         f_shift -= roll_off;
            
         if (f_shift < 100)    f_shift = 100;
         if (f_shift > 100000) f_shift = 100000;
            
         hr = buffer->SetFrequency(f_shift);
         if (!SUCCEEDED(hr)) {
            char warn[512];
            sprintf_s(warn, "Warning: could not set Doppler frequency on buffer to %d", f_shift);
            SoundD3DError(warn, hr);
         }
      }
   }
   else {
      buffer->SetPan((LONG) location.x);
      buffer->SetVolume((LONG) volume);
   }
#endif

   moved = false;
}

// +--------------------------------------------------------------------+

Sound*
SoundD3D::Duplicate()
{
   Sound* sound = 0;

   if (flags & RESOURCE) {
      sound = Sound::Create(flags & ~RESOURCE, &wfex);

      if (sound && !(flags & STREAMED)) {
         sound->SetMinDistance(min_dist);
         sound->SetMaxDistance(max_dist);

         if (!buffer) {
            sound->Load(data_len, data);
         }

         else {
            SoundD3D* s3d = (SoundD3D*) sound;
            soundcard->DuplicateSoundBuffer(buffer, &s3d->buffer);
            sound->Rewind();
         }
      }
   }

   return sound;
}

// +--------------------------------------------------------------------+

HRESULT
SoundD3D::StreamFile(const char* name, DWORD offset)
{
   DWORD buf_size    = wfex.nAvgBytesPerSec / 2;
   DWORD safety_zone = buf_size * 2;

   if (stream) {
      delete[] transfer;
      transfer = 0;
      fclose(stream);
   }

   status         = UNINITIALIZED;
   stream_left    = 0;
   stream_offset  = offset;

   eos_written    = false;
   eos_latch      = 0;
   min_safety     = safety_zone;
   read_size      = buf_size;

   fopen_s(&stream, name, "rb");

   // open the stream:
   if (stream == 0) {
      SoundD3DError("StreamFile: could not open stream", E_FAIL);
      return E_FAIL;
   }

   // find the size of the file:
   fseek(stream, 0, SEEK_END);
   stream_left = ftell(stream) - offset;
   fseek(stream, stream_offset, SEEK_SET);

   total_time = (double) stream_left /
                (double) wfex.nAvgBytesPerSec;

   if (stream_left < read_size) {
      read_size = stream_left;
   }

   HRESULT hr = AllocateBuffer(read_size + min_safety);
   
   if (FAILED(hr))
      return hr;

   flags |= STREAMED;

   // preload the buffer:
   w = r = 0;
   transfer = new(__FILE__,__LINE__) BYTE[read_size + 1024];

   if (!transfer) {
      hr = E_FAIL;
   }

   else {
      ZeroMemory(transfer, read_size+1024);
      StreamBlock();
   }

   return hr;
}

// +--------------------------------------------------------------------+

HRESULT
SoundD3D::StreamOggFile()
{
   DWORD buf_size    = wfex.nAvgBytesPerSec / 2;
   DWORD safety_zone = buf_size * 2;

   if (stream) {
      delete[] transfer;
      fclose(stream);

      transfer = 0;
      stream   = 0;
   }

   status         = UNINITIALIZED;
   stream_left    = (DWORD) ov_pcm_total(ov_file,-1);
   stream_offset  = 0;

   eos_written    = false;
   eos_latch      = 0;
   min_safety     = safety_zone;
   read_size      = buf_size;

   total_time     = (double) stream_left /
                    (double) wfex.nAvgBytesPerSec;

   if (stream_left < read_size) {
      read_size = stream_left;
   }

   HRESULT hr = AllocateBuffer(read_size + min_safety);
   
   if (FAILED(hr))
      return hr;

   flags |= STREAMED | OGGVORBIS;

   // preload the buffer:
   w = r = 0;
   transfer = new(__FILE__,__LINE__) BYTE[read_size + 1024];

   if (!transfer) {
      hr = E_FAIL;
   }

   else {
      ZeroMemory(transfer, read_size+1024);
      StreamOggBlock();
   }

   return hr;
}

// +--------------------------------------------------------------------+

HRESULT
SoundD3D::Load(DWORD bytes, BYTE* data)
{
   status = UNINITIALIZED;

   HRESULT hr;

   if (!buffer) {
      hr = AllocateBuffer(bytes);
      if (FAILED(hr)) {
         return hr;
      }
   }

   LPVOID   dest1, dest2;
   DWORD    size1, size2;

   hr = buffer->Lock(w, bytes, &dest1, &size1, &dest2, &size2, 0);

   if (hr == DSERR_BUFFERLOST) {
      buffer->Restore();
      hr = buffer->Lock(w, bytes, &dest1, &size1, &dest2, &size2, 0);
   }

   if (SUCCEEDED(hr)) {
      CopyMemory(dest1, data, size1);
      if (dest2) {
         CopyMemory(dest2, data + size1, size2);
      }

      if (flags & STREAMED)
         w = (w + size1 + size2) % (read_size + min_safety);
      else
         w += size1 + size2;

      hr = buffer->Unlock(dest1, size1, dest2, size2);
      if (FAILED(hr)) {
         SoundD3DError("Load: could not unlock buffer", hr);
      }
   }
   else {
      SoundD3DError("Load: could not lock buffer", hr);
   }

   if (SUCCEEDED(hr)) {
      status = READY;
   }

   return hr;
}

// +--------------------------------------------------------------------+

HRESULT
SoundD3D::AllocateBuffer(DWORD bytes)
{
   HRESULT hr = S_OK;

   if (!buffer) {
      dsbd.dwBufferBytes = bytes;

      if (soundcard) {
         hr = soundcard->CreateSoundBuffer(&dsbd, &buffer, NULL);
      
         if (FAILED(hr)) {
            SoundD3DError("AllocateBuffer: could not create buffer", hr);

            Print("   dsbd.dwSize         = %8d\n",  dsbd.dwSize);
            Print("   dsbd.dwFlags        = %08x\n", dsbd.dwFlags);
            Print("   dsbd.dwBufferBytes  = %8d\n",  dsbd.dwBufferBytes);
            Print("   dsbd.lpwfxFormat    = %08x\n", dsbd.lpwfxFormat);

            if (dsbd.lpwfxFormat) {
            Print("   wfex.wBitsPerSample = %8d\n",  dsbd.lpwfxFormat->wBitsPerSample);
            Print("   wfex.nChannels      = %8d\n",  dsbd.lpwfxFormat->nChannels);
            Print("   wfex.nSamplesPerSec = %8d\n",  dsbd.lpwfxFormat->nSamplesPerSec);
            }
         }
      }
      else {
         SoundD3DError("AllocateBuffer: soundcard is null", E_FAIL);
      }
   }
   
   return hr;
}

// +--------------------------------------------------------------------+

HRESULT
SoundD3D::Play()
{
   if (IsPlaying())  return S_OK;
   if (!buffer)      return E_FAIL;

   HRESULT hr = E_FAIL;

   if (IsDone())
      hr = Rewind();

   if (IsReady()) {
      if (moved)
         Localize();

      if (flags & LOOP || flags & STREAMED)
         hr = buffer->Play(0, 0, DSBPLAY_LOOPING);
      else
         hr = buffer->Play(0, 0, 0);

      if (SUCCEEDED(hr))
         status = PLAYING;
   }

   return hr;
}

// +--------------------------------------------------------------------+

HRESULT
SoundD3D::Rewind()
{
   if (!buffer) return E_FAIL;

   HRESULT hr = S_OK;
   
   if (IsPlaying())
      hr = Stop();

   if (flags & STREAMED) {
      RewindStream();
      StreamBlock();
   }

   else {
      hr = buffer->SetCurrentPosition(0);
   }

   if (SUCCEEDED(hr)) {
      status = READY;
      looped = 0;
   }

   return hr;
}

// +--------------------------------------------------------------------+

HRESULT
SoundD3D::Pause()
{
   if (status == DONE)
      return S_OK;

   HRESULT hr = Stop();

   if (SUCCEEDED(hr))
      status = READY;

   return hr;
}

// +--------------------------------------------------------------------+

HRESULT
SoundD3D::Stop()
{
   if (!buffer)
      return E_FAIL;

   if (!IsPlaying()) {
      status = DONE;
      return S_OK;
   }

   status = DONE;
   return buffer->Stop();
}

// +--------------------------------------------------------------------+

double
SoundD3D::GetTimeRemaining() const
{
   double time_left = -1;

   if (IsPlaying() || IsReady()) {
      time_left = (double) stream_left /
                  (double) wfex.nAvgBytesPerSec;
   }

   return time_left;
}

double
SoundD3D::GetTimeElapsed() const
{
   double time_elapsed = 0;

   if (IsPlaying()) {
      time_elapsed = total_time - GetTimeRemaining();
   }

   return time_elapsed;
}

// +--------------------------------------------------------------------+

void
SoundD3D::SetVolume(long v)
{
   if (v > 0)           v = 0;
   else if (v < -10000) v = -10000;

   volume = v;
   moved  = true;
}

// +--------------------------------------------------------------------+

long
SoundD3D::GetPan() const
{
   long p = 10000;
   
   if (location.z)     p = (long) fabs(location.x/location.z);
   if (p > 10000)      p = 10000;
   if (location.x < 0) p = -p;

   return p;
}

void
SoundD3D::SetPan(long p)
{
   if (p >  10000) p =  10000;
   if (p < -10000) p = -10000;

   location.x = (float) p;
   location.y = 0.0f;
   location.z = 1.0f;
   moved      = true;
}

// +--------------------------------------------------------------------+

void
SoundD3D::SetLocation(const Vec3& l)
{
   location = l;
   moved    = true;
}

void
SoundD3D::SetVelocity(const Vec3& v)
{
   velocity = v;
   moved    = true;
}

// +--------------------------------------------------------------------+

float
SoundD3D::GetMinDistance() const
{
   return min_dist;
}

void
SoundD3D::SetMinDistance(float f)
{
   min_dist = f;
   moved    = true;
}

// +--------------------------------------------------------------------+

float
SoundD3D::GetMaxDistance() const
{
   return max_dist;
}
void
SoundD3D::SetMaxDistance(float f)
{
   max_dist = f;
   moved    = true;
}


// +--------------------------------------------------------------------+

void
SoundD3DError(const char* msg, HRESULT err)
{
   Print("   SoundD3D: %s. [%s]\n", msg, DSErrStr(err));
}

char*
DSErrStr(HRESULT err)
{
   switch (err) {
   case DS_OK:                           return "DS_OK";

   case DSERR_ALLOCATED:                 return
   "The call failed because resources (such as a priority level) "
   "were already being used by another caller.";

   case DSERR_CONTROLUNAVAIL:            return
   "The control (vol,pan,etc.) requested by the caller is not available.";

   case DSERR_INVALIDPARAM:              return
   "An invalid parameter was passed to the returning function.";

   case DSERR_INVALIDCALL:               return
   "This call is not valid for the current state of this object";

   case DSERR_GENERIC:                   return
   "An undetermined error occured inside the DirectSound subsystem";

   case DSERR_PRIOLEVELNEEDED:           return
   "The caller does not have the priority level required for the function to succeed.";

   case DSERR_OUTOFMEMORY:               return
   "Not enough free memory is available to complete the operation";

   case DSERR_BADFORMAT:                 return
   "The specified WAVE format is not supported";

   case DSERR_UNSUPPORTED:               return
   "The function called is not supported at this time";

   case DSERR_NODRIVER:                  return
   "No sound driver is available for use";

   case DSERR_ALREADYINITIALIZED:        return
   "This object is already initialized";

   case DSERR_NOAGGREGATION:             return
   "This object does not support aggregation";

   case DSERR_BUFFERLOST:                return
   "The buffer memory has been lost, and must be restored.";

   case DSERR_OTHERAPPHASPRIO:           return
   "Another app has a higher priority level, preventing this call from succeeding.";

   case DSERR_UNINITIALIZED:             return
   "This object has not been initialized.";

#ifdef DIRECT_SOUND_3D
   case DSERR_NOINTERFACE:               return
   "The requested COM interface is not available.";
#endif

   default: return "Unknown Error Code";
   }

   return "Internal Error";
}


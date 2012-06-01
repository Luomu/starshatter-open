/*  Project nGenEx
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         Sound.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Abstract Sound Object
*/

#ifndef Sound_h
#define Sound_h

#include "Types.h"
#include "Geometry.h"

// +--------------------------------------------------------------------+

class SoundCard;
class SoundCheck;
class Camera;

// +--------------------------------------------------------------------+

class Sound
{
public:
	static const char* TYPENAME() { return "Sound"; }

	static Sound*     CreateStream(const char* filename);
	static Sound*     CreateOggStream(const char* filename);
	static Sound*     Create(DWORD flags, LPWAVEFORMATEX format);
	static Sound*     Create(DWORD flags, LPWAVEFORMATEX format, DWORD len, LPBYTE data);
	static void       SetListener(const Camera& cam, const Vec3& vel);
	static void       UseSoundCard(SoundCard* s) { creator = s; }

public:
	Sound();
	virtual ~Sound();

	int operator==(const Sound& rhs) const { return this == &rhs; }

	enum FlagEnum {   AMBIENT   = 0x0000,
		LOCALIZED = 0x0001,
		LOC_3D    = 0x0002,
		MEMORY    = 0x0000,
		STREAMED  = 0x0004,
		ONCE      = 0x0000,
		LOOP      = 0x0008,
		FREE      = 0x0000,
		LOCKED    = 0x0010,
		DOPPLER   = 0x0020,
		INTERFACE = 0x0040,
		OGGVORBIS = 0x4000,
		RESOURCE  = 0x8000   // not playable, only used to store data
	};

	enum StatusEnum { UNINITIALIZED,
		INITIALIZING,
		READY,
		PLAYING,
		DONE };

	// once per frame:
	virtual void      Update()                      { }

	// mark for collection:
	virtual void      Release();

	// data loading:
	// this method is for streamed sounds:
	virtual HRESULT   StreamFile(const char* name, DWORD offset) { return E_NOINTERFACE; }

	// this method is for memory sounds:
	virtual HRESULT   Load(DWORD bytes, BYTE* data) { return E_NOINTERFACE; }  // => Ready

	// this method is for sound resources:
	virtual Sound*    Duplicate()                   { return 0;             }  // => Ready

	// transport operations:
	virtual HRESULT   Play()                        { return E_NOINTERFACE; }  // => Playing
	virtual HRESULT   Rewind()                      { return E_NOINTERFACE; }  // => Ready
	virtual HRESULT   Pause()                       { return E_NOINTERFACE; }  // => Ready
	virtual HRESULT   Stop()                        { return E_NOINTERFACE; }  // => Done

	// accessors / mutators
	int               IsReady()               const { return status == READY;   }
	int               IsPlaying()             const { return status == PLAYING; }
	int               IsDone()                const { return status == DONE;    }
	int               LoopCount()             const { return looped;   }

	virtual DWORD     GetFlags()              const { return flags;    }
	virtual void      SetFlags(DWORD f)             { flags = f;       }
	virtual DWORD     GetStatus()             const { return status;   }

	virtual long      GetVolume()             const { return volume;   }
	virtual void      SetVolume(long v)             { volume = v;      }
	virtual long      GetPan()                const { return 0;        }
	virtual void      SetPan(long p)                {                  }

	// (only for streamed sounds)
	virtual double    GetTotalTime()          const { return 0; }
	virtual double    GetTimeRemaining()      const { return 0; }
	virtual double    GetTimeElapsed()        const { return 0; }

	// These should be relative to the listener:
	// (only used for localized sounds)
	virtual const Vec3& GetLocation()         const { return location; }
	virtual void        SetLocation(const Vec3& l)  { location = l;    }
	virtual const Vec3& GetVelocity()         const { return velocity; }
	virtual void        SetVelocity(const Vec3& v)  { velocity = v;    }

	virtual float     GetMinDistance()        const { return 0; }
	virtual void      SetMinDistance(float f)       {           }
	virtual float     GetMaxDistance()        const { return 0; }
	virtual void      SetMaxDistance(float f)       {           }

	virtual void      SetSoundCheck(SoundCheck* s)  { sound_check = s; }
	virtual void      AddToSoundCard();

	const char*       GetFilename()           const { return filename; }
	void              SetFilename(const char* s);

protected:
	DWORD             flags;
	DWORD             status;
	long              volume;  // centibels, (0 .. -10000)
	int               looped;
	Vec3              location;
	Vec3              velocity;
	SoundCheck*       sound_check;
	char              filename[64];

	static SoundCard* creator;
};

// +--------------------------------------------------------------------+

class SoundCheck
{
public:
	virtual void Update(Sound* s) { }
};

#endif Sound_h


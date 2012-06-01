/*  Project Starshatter 4.6
	Destroyer Studios LLC
	Copyright © 1997-2006. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         TrackIR.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	TrackIR head tracker interface class
*/

#ifndef TrackIR_h
#define TrackIR_h

#include "Types.h"

// +--------------------------------------------------------------------+

class TrackIR
{
public:
	TrackIR();
	~TrackIR();

	DWORD    ExecFrame();

	bool     IsRunning()    const { return running; }
	double   GetAzimuth()   const { return az; }
	double   GetElevation() const { return el; }

	double   GetX()         const { return x;  }
	double   GetY()         const { return y;  }
	double   GetZ()         const { return z;  }

protected:

	bool     running;
	DWORD    stale_frames;
	DWORD    frame_signature;

	double   az;
	double   el;

	double   x; // vrt
	double   y; // vup
	double   z; // vpn (i think)
};

#endif TrackIR_h


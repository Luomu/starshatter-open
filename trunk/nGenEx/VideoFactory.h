/*  Project nGenEx
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         VideoFactory.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Video Factory class
*/

#ifndef VideoFactory_h
#define VideoFactory_h

#include "Types.h"
#include "Video.h"
#include "SoundCard.h"

// +--------------------------------------------------------------------+

class VideoFactory
{
public:
	VideoFactory(HWND h);
	virtual ~VideoFactory();

	virtual Video*       CreateVideo(VideoSettings* vs);
	virtual void         DestroyVideo(Video* video);
	virtual SoundCard*   CreateSoundCard();

private:
	HWND        hwnd;

	Video*      video;
	SoundCard*  audio;
};

#endif VideoFactory_h


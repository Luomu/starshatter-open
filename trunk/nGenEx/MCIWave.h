/*  Project nGen
	John DiCamillo Software Consulting
	Copyright © 1997-2000. All Rights Reserved.

	SUBSYSTEM:    nGen.lib
	FILE:         MCIWave.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	MCI Wave Output stuff
*/

#ifndef MCI_WAVE_H
#define MCI_WAVE_H

// +--------------------------------------------------------------------+

int  load_wave_file(const char* fname, LPWAVEHDR hdr, LPWAVEFORMATEX format);
void delete_wave_file(LPWAVEHDR hdr, LPWAVEFORMATEX format);

// +--------------------------------------------------------------------+

#endif

/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         MusicDirector.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Music Director class to manage selection, setup, and playback
	of background music tracks for both menu and game modes
*/


#include "MemDebug.h"
#include "MusicDirector.h"
#include "MusicTrack.h"

#include "Random.h"
#include "DataLoader.h"
#include "FormatUtil.h"
#include "Sound.h"

static MusicDirector* music_director = 0;

// +-------------------------------------------------------------------+

MusicDirector::MusicDirector()
: mode(0), transition(0), track(0), next_track(0), no_music(true),
hproc(0)
{
	music_director = this;

	ScanTracks();

	if (!no_music)
	StartThread();
}

MusicDirector::~MusicDirector()
{
	StopThread();

	delete track;
	delete next_track;

	menu_tracks.destroy();
	intro_tracks.destroy();
	brief_tracks.destroy();
	debrief_tracks.destroy();
	promote_tracks.destroy();
	flight_tracks.destroy();
	combat_tracks.destroy();
	launch_tracks.destroy();
	recovery_tracks.destroy();
	victory_tracks.destroy();
	defeat_tracks.destroy();
	credit_tracks.destroy();

	if (this == music_director)
	music_director = 0;
}

// +--------------------------------------------------------------------+

void
MusicDirector::Initialize()
{
	if (music_director) delete music_director;
	music_director = new(__FILE__,__LINE__) MusicDirector();
}

void
MusicDirector::Close()
{
	delete music_director;
	music_director = 0;
}

MusicDirector*
MusicDirector::GetInstance()
{
	return music_director;
}

// +-------------------------------------------------------------------+

void
MusicDirector::ExecFrame()
{
	if (no_music) return;

	AutoThreadSync a(sync);

	if (next_track && !track) {
		track = next_track;
		next_track = 0;
	}

	if (track) {
		if (track->IsDone()) {
			if (mode != NONE && mode != SHUTDOWN && next_track == 0) {
				GetNextTrack(track->GetIndex()+1);
			}

			delete track;
			track = next_track;
			next_track = 0;
		}

		else if (track->IsLooped()) {
			if (mode != NONE && mode != SHUTDOWN && next_track == 0) {
				GetNextTrack(track->GetIndex()+1);
			}

			track->FadeOut();
			track->ExecFrame();
		}

		else {
			track->ExecFrame();
		}
	}

	if (next_track) {
		if (next_track->IsDone()) {
			delete next_track;
			next_track = 0;
		}

		else if (next_track->IsLooped()) {
			next_track->FadeOut();
			next_track->ExecFrame();
		}

		else {
			next_track->ExecFrame();
		}
	}
}

// +-------------------------------------------------------------------+

void
MusicDirector::ScanTracks()
{
	DataLoader* loader = DataLoader::GetLoader();

	bool old_file_system = loader->IsFileSystemEnabled();
	loader->UseFileSystem(true);
	loader->SetDataPath("Music/");

	List<Text> files;
	loader->ListFiles("*.ogg", files, true);

	if (files.size() == 0) {
		loader->UseFileSystem(old_file_system);
		no_music = true;
		return;
	}

	no_music = false;

	ListIter<Text> iter = files;
	while (++iter) {
		Text* name = iter.value();
		Text* file = new(__FILE__,__LINE__) Text("Music/");

		name->setSensitive(false);
		file->append(*name);

		if (name->indexOf("Menu") == 0) {
			menu_tracks.append(file);
		}

		else if (name->indexOf("Intro") == 0) {
			intro_tracks.append(file);
		}

		else if (name->indexOf("Brief") == 0) {
			brief_tracks.append(file);
		}

		else if (name->indexOf("Debrief") == 0) {
			debrief_tracks.append(file);
		}

		else if (name->indexOf("Promot") == 0) {
			promote_tracks.append(file);
		}

		else if (name->indexOf("Flight") == 0) {
			flight_tracks.append(file);
		}

		else if (name->indexOf("Combat") == 0) {
			combat_tracks.append(file);
		}

		else if (name->indexOf("Launch") == 0) {
			launch_tracks.append(file);
		}

		else if (name->indexOf("Recovery") == 0) {
			recovery_tracks.append(file);
		}

		else if (name->indexOf("Victory") == 0) {
			victory_tracks.append(file);
		}

		else if (name->indexOf("Defeat") == 0) {
			defeat_tracks.append(file);
		}

		else if (name->indexOf("Credit") == 0) {
			credit_tracks.append(file);
		}

		else {
			menu_tracks.append(file);
		}

		delete iter.removeItem();
	}

	loader->UseFileSystem(old_file_system);

	menu_tracks.sort();
	intro_tracks.sort();
	brief_tracks.sort();
	debrief_tracks.sort();
	promote_tracks.sort();
	flight_tracks.sort();
	combat_tracks.sort();
	launch_tracks.sort();
	recovery_tracks.sort();
	victory_tracks.sort();
	defeat_tracks.sort();
	credit_tracks.sort();
}

// +-------------------------------------------------------------------+

const char*
MusicDirector::GetModeName(int mode)
{
	switch (mode) {
	case NONE:        return "NONE";
	case MENU:        return "MENU";
	case INTRO:       return "INTRO";
	case BRIEFING:    return "BRIEFING";
	case DEBRIEFING:  return "DEBRIEFING";
	case PROMOTION:   return "PROMOTION";
	case FLIGHT:      return "FLIGHT";
	case COMBAT:      return "COMBAT";
	case LAUNCH:      return "LAUNCH";
	case RECOVERY:    return "RECOVERY";
	case VICTORY:     return "VICTORY";
	case DEFEAT:      return "DEFEAT";
	case CREDITS:     return "CREDITS";
	case SHUTDOWN:    return "SHUTDOWN";
	}

	return "UNKNOWN?";
}

// +-------------------------------------------------------------------+

void
MusicDirector::SetMode(int mode)
{
	if (!music_director || music_director->no_music) return;

	AutoThreadSync a(music_director->sync);

	// stay in intro mode until it is complete:
	if (mode == MENU && (music_director->GetMode() == NONE || 
				music_director->GetMode() == INTRO))
	mode = INTRO;

	mode = music_director->CheckMode(mode);

	if (mode != music_director->mode) {
		::Print("MusicDirector::SetMode() old: %s  new: %s\n",
		GetModeName(music_director->mode), 
		GetModeName(mode));

		music_director->mode = mode;

		MusicTrack* t = music_director->track;
		if (t && t->GetState() && !t->IsDone()) {
			if (mode == NONE || mode == SHUTDOWN)
			t->SetFadeTime(0.5);

			t->FadeOut();
		}

		t = music_director->next_track;
		if (t && t->GetState() && !t->IsDone()) {
			if (mode == NONE || mode == SHUTDOWN)
			t->SetFadeTime(0.5);
			t->FadeOut();

			delete music_director->track;
			music_director->track      = t;
			music_director->next_track = 0;
		}

		music_director->ShuffleTracks();
		music_director->GetNextTrack(0);

		if (music_director->next_track)
		music_director->next_track->FadeIn();
	}
}

int
MusicDirector::CheckMode(int req_mode)
{
	if (req_mode == RECOVERY && recovery_tracks.size() == 0)
	req_mode = LAUNCH;

	if (req_mode == LAUNCH && launch_tracks.size() == 0)
	req_mode = FLIGHT;

	if (req_mode == COMBAT && combat_tracks.size() == 0)
	req_mode = FLIGHT;

	if (req_mode == FLIGHT && flight_tracks.size() == 0)
	req_mode = NONE;

	if (req_mode == PROMOTION && promote_tracks.size() == 0)
	req_mode = VICTORY;

	if (req_mode == DEBRIEFING && debrief_tracks.size() == 0)
	req_mode = BRIEFING;

	if (req_mode == BRIEFING && brief_tracks.size() == 0)
	req_mode = MENU;

	if (req_mode == INTRO && intro_tracks.size() == 0)
	req_mode = MENU;

	if (req_mode == VICTORY && victory_tracks.size() == 0)
	req_mode = MENU;

	if (req_mode == DEFEAT && defeat_tracks.size() == 0)
	req_mode = MENU;

	if (req_mode == CREDITS && credit_tracks.size() == 0)
	req_mode = MENU;

	if (req_mode == MENU && menu_tracks.size() == 0)
	req_mode = NONE;

	return req_mode;
}

// +-------------------------------------------------------------------+

bool
MusicDirector::IsNoMusic()
{
	if (music_director)
	return music_director->no_music;

	return true;
}

// +-------------------------------------------------------------------+

void
MusicDirector::GetNextTrack(int index)
{
	List<Text>* tracks = 0;

	switch (mode) {
	case MENU:        tracks = &menu_tracks;     break;
	case INTRO:       tracks = &intro_tracks;    break;
	case BRIEFING:    tracks = &brief_tracks;    break;
	case DEBRIEFING:  tracks = &debrief_tracks;  break;
	case PROMOTION:   tracks = &promote_tracks;  break;
	case FLIGHT:      tracks = &flight_tracks;   break;
	case COMBAT:      tracks = &combat_tracks;   break;
	case LAUNCH:      tracks = &launch_tracks;   break;
	case RECOVERY:    tracks = &recovery_tracks; break;
	case VICTORY:     tracks = &victory_tracks;  break;
	case DEFEAT:      tracks = &defeat_tracks;   break;
	case CREDITS:     tracks = &credit_tracks;   break;
	default:          tracks = 0;                break;
	}

	if (tracks && tracks->size()) {
		if (next_track)
		delete next_track;

		if (index < 0 || index >= tracks->size()) {
			index = 0;

			if (mode == INTRO) {
				mode = MENU;
				ShuffleTracks();
				tracks = &menu_tracks;

				::Print("MusicDirector: INTRO mode complete, switching to MENU\n");

				if (!tracks->size())
				return;
			}
		}

		next_track = new(__FILE__,__LINE__) MusicTrack(*tracks->at(index), mode, index);
		next_track->FadeIn();
	}

	else if (next_track) {
		next_track->FadeOut();
	}
}

// +-------------------------------------------------------------------+

void
MusicDirector::ShuffleTracks()
{
	List<Text>* tracks = 0;

	switch (mode) {
	case MENU:        tracks = &menu_tracks;     break;
	case INTRO:       tracks = &intro_tracks;    break;
	case BRIEFING:    tracks = &brief_tracks;    break;
	case DEBRIEFING:  tracks = &debrief_tracks;  break;
	case PROMOTION:   tracks = &promote_tracks;  break;
	case FLIGHT:      tracks = &flight_tracks;   break;
	case COMBAT:      tracks = &combat_tracks;   break;
	case LAUNCH:      tracks = &launch_tracks;   break;
	case RECOVERY:    tracks = &recovery_tracks; break;
	case VICTORY:     tracks = &victory_tracks;  break;
	case DEFEAT:      tracks = &defeat_tracks;   break;
	case CREDITS:     tracks = &credit_tracks;   break;
	default:          tracks = 0;                break;
	}

	if (tracks && tracks->size() > 1) {
		tracks->sort();

		Text* t = tracks->at(0);

		if (!isdigit(*t[0]))
		tracks->shuffle();
	}
}

// +--------------------------------------------------------------------+

DWORD WINAPI MusicDirectorThreadProc(LPVOID link);

void
MusicDirector::StartThread()
{
	if (hproc != 0) {
		DWORD result = 0;
		GetExitCodeThread(hproc, &result);

		if (result != STILL_ACTIVE) {
			CloseHandle(hproc);
			hproc = 0;
		}
		else {
			return;
		}
	}

	if (hproc == 0) {
		DWORD thread_id = 0;
		hproc = CreateThread(0, 4096, MusicDirectorThreadProc, (LPVOID) this, 0, &thread_id);

		if (hproc == 0) {
			static int report = 10;
			if (report > 0) {
				::Print("WARNING: MusicDirector failed to create thread (err=%08x)\n", GetLastError());
				report--;

				if (report == 0)
				::Print("         Further warnings of this type will be supressed.\n");
			}
		}
	}
}

void
MusicDirector::StopThread()
{
	if (hproc != 0) {
		SetMode(SHUTDOWN);
		WaitForSingleObject(hproc, 1500);
		CloseHandle(hproc);
		hproc = 0;
	}
}

DWORD WINAPI MusicDirectorThreadProc(LPVOID link)
{
	MusicDirector* dir = (MusicDirector*) link;

	if (dir) {
		while (dir->GetMode() != MusicDirector::SHUTDOWN) {
			dir->ExecFrame();
			Sleep(100);
		}

		return 0;
	}

	return (DWORD) E_POINTER;
}


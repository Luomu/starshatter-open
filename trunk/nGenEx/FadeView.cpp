/*  Project nGenEx
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         FadeView.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Fading Bitmap "billboard" Image View class
*/

#include "MemDebug.h"
#include "FadeView.h"
#include "Color.h"
#include "Window.h"
#include "Video.h"
#include "Screen.h"
#include "Game.h"

// +--------------------------------------------------------------------+

FadeView::FadeView(Window* c, double in, double out, double hold)
: View(c),
fade_in(in * 1000),
fade_out(out * 1000),
hold_time(hold * 1000),
step_time(0),
fast(1),
time(0)
{
	state = StateStart;
}

FadeView::~FadeView()
{ }

// +--------------------------------------------------------------------+

void FadeView::FadeIn(double in)       { fade_in   = in  * 1000; }
void FadeView::FadeOut(double out)     { fade_out  = out * 1000; }
void FadeView::FastFade(int fade_fast) { fast      = fade_fast;  }
void FadeView::StopHold()
{
	//Print("  FadeView::StopHold()\n");
	hold_time = 0;
}

// +--------------------------------------------------------------------+

void
FadeView::Refresh()
{
	double msec = 0;

	if (state == StateStart) {
		time = Game::RealTime();
	}
	else if (state != StateDone) {
		double new_time = Game::RealTime();
		msec = new_time - time;
		time = new_time;
	}

	switch (state) {
	case StateStart:
		if (fade_in) {
			//Print("  *  FadeView: %f, %f, %f\n", fade_in, fade_out, hold_time);
			Color::SetFade(0);
			//Print("  1. FadeView SetFade to   0  (%6.1f)\n", time);
		}

		step_time = 0;
		state = State2;
		break;

	case State2:
		if (fade_in) {
			Color::SetFade(0);
			//Print("  1. FadeView SetFade to   0  (%6.1f)\n", time);
		}

		step_time = 0;
		state = StateIn;
		break;

	case StateIn:
		if (step_time < fade_in) {
			double fade = step_time / fade_in;
			Color::SetFade(fade);
			//Print("  2. FadeView SetFade to %3d  (%6.1f) %6.1f\n", (int) (fade * 100), time, step_time);
			step_time += msec;
		}
		else {
			Color::SetFade(1);
			//Print("  2. FadeView SetFade to %3d  (%6.1f) %6.1f => HOLDING\n", 100, time, step_time);
			step_time = 0;
			state = StateHold;
		}
		break;

	case StateHold:
		if (step_time < hold_time) {
			step_time += msec;
			//Print("  3. FadeView holding at %3d  (%6.1f) %6.1f\n", 100, time, step_time);
		}
		else {
			//Print("  3. FadeView HOLD COMPLETE   (%6.1f) %6.1f\n", time, step_time);
			step_time = 0;
			state = StateOut;
		}
		break;

	case StateOut:
		if (fade_out > 0) {
			if (step_time < fade_out) {
				double fade = 1 - step_time / fade_out;
				Color::SetFade(fade);
				//Print("  4. FadeView SetFade to %3d  (%6.1f) %6.1f\n", (int) (fade*100), time, step_time);
				step_time += msec;
			}
			else {
				Color::SetFade(0);
				//Print("  4. FadeView SetFade to %3d  (%6.1f) %6.1f\n", 0, time, step_time);
				step_time = 0;
				state = StateDone;
			}
		}
		else {
			Color::SetFade(1);
			//Print("  4. FadeView SetFade to %3d  (%6.1f) %6.1f\n", 0, time, step_time);
			step_time = 0;
			state = StateDone;
		}
		break;

	default:
	case StateDone:
		//Print("  5. FadeView done  (%6.1f) %6.1f\n", time, step_time);
		break;
	}
}


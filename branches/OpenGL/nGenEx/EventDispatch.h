/*  Project nGenEx
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         EventDispatch.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Event Dispatch class
*/

#ifndef EventDispatch_h
#define EventDispatch_h

#include "Types.h"
#include "EventTarget.h"
#include "List.h"

// +--------------------------------------------------------------------+

class EventDispatch
{
public:
	static const char* TYPENAME() { return "EventDispatch"; }

	EventDispatch();
	virtual ~EventDispatch();

	static void    Create();
	static void    Close();
	static EventDispatch* GetInstance()  { return dispatcher; }

	virtual void   Dispatch();
	virtual void   Register(EventTarget* tgt);
	virtual void   Unregister(EventTarget* tgt);

	virtual EventTarget* GetCapture();
	virtual int    CaptureMouse(EventTarget* tgt);
	virtual int    ReleaseMouse(EventTarget* tgt);

	virtual EventTarget* GetFocus();
	virtual void   SetFocus(EventTarget* tgt);
	virtual void   KillFocus(EventTarget* tgt);

	virtual void   MouseEnter(EventTarget* tgt);

protected:
	int                     mouse_x, mouse_y, mouse_l, mouse_r;
	List<EventTarget>       clients;
	EventTarget*            capture;
	EventTarget*            current;
	EventTarget*            focus;
	EventTarget*            click_tgt;

	static EventDispatch*   dispatcher;
};

#endif EventDispatch_h


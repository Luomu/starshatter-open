/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         MissionEvent.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Events for mission scripting
*/

#ifndef MissionEvent_h
#define MissionEvent_h

#include "Types.h"
#include "List.h"
#include "Text.h"
#include "Geometry.h"
#include "Bitmap.h"

// +--------------------------------------------------------------------+

class Mission;
class MissionElement;
class MissionLoad;
class MissionEvent;

class Ship;
class System;
class Element;
class ShipDesign;
class WeaponDesign;
class StarSystem;
class Instruction;
class Sound;

// +--------------------------------------------------------------------+

class MissionEvent
{
	friend class Mission;
	friend class MissionTemplate;
	friend class MsnEditDlg;
	friend class MsnEventDlg;

public:
	static const char* TYPENAME() { return "MissionEvent"; }

	enum EVENT_TYPE {
		MESSAGE, 
		OBJECTIVE,
		INSTRUCTION,
		IFF,
		DAMAGE,
		JUMP,
		HOLD,
		SKIP,
		END_MISSION,

		BEGIN_SCENE,
		CAMERA,
		VOLUME,
		DISPLAY,
		FIRE_WEAPON,
		END_SCENE,

		NUM_EVENTS
	};

	enum EVENT_STATUS {
		PENDING, ACTIVE, COMPLETE, SKIPPED
	};

	enum EVENT_TRIGGER {
		TRIGGER_TIME,        TRIGGER_DAMAGE,      TRIGGER_DESTROYED, 
		TRIGGER_JUMP,        TRIGGER_LAUNCH,      TRIGGER_DOCK,
		TRIGGER_NAVPT,       TRIGGER_EVENT,       TRIGGER_SKIPPED,
		TRIGGER_TARGET,      TRIGGER_SHIPS_LEFT,  TRIGGER_DETECT,
		TRIGGER_RANGE,       TRIGGER_EVENT_ALL,   TRIGGER_EVENT_ANY,
		NUM_TRIGGERS
	};

	MissionEvent();
	~MissionEvent();

	// operations:
	void                 ExecFrame(double seconds);
	void                 Activate();

	virtual bool         CheckTrigger();
	virtual void         Execute(bool silent=false);
	virtual void         Skip();

	// accessors:
	int                  EventID()         const { return id;      }
	int                  Status()          const { return status;  }
	bool                 IsPending()       const { return status == PENDING;  }
	bool                 IsActive()        const { return status == ACTIVE;   }
	bool                 IsComplete()      const { return status == COMPLETE; }
	bool                 IsSkipped()       const { return status == SKIPPED;  }

	double               Time()            const { return time;    }
	double               Delay()           const { return delay;   }

	int                  Event()           const { return event;         }
	const char*          EventName()       const;
	Text                 EventShip()       const { return event_ship;    }
	Text                 EventSource()     const { return event_source;  }
	Text                 EventTarget()     const { return event_target;  }
	Text                 EventMessage()    const { return event_message; }
	Text                 EventSound()      const { return event_sound;   }

	int                  EventParam(int index=0)    const;
	int                  NumEventParams()           const;

	int                  EventChance()     const { return event_chance;  }
	Point                EventPoint()      const { return event_point;   }
	Rect                 EventRect()       const { return event_rect;    }

	int                  Trigger()         const { return trigger;          }
	const char*          TriggerName()     const;
	Text                 TriggerShip()     const { return trigger_ship;     }
	Text                 TriggerTarget()   const { return trigger_target;   }

	Text                 TriggerParamStr()          const;
	int                  TriggerParam(int index=0)  const;
	int                  NumTriggerParams()         const;

	static const char*   EventName(int n);
	static int           EventForName(const char* n);
	static const char*   TriggerName(int n);
	static int           TriggerForName(const char* n);

protected:
	int                  id;
	int                  status;
	double               time;
	double               delay;

	int                  event;
	Text                 event_ship;
	Text                 event_source;
	Text                 event_target;
	Text                 event_message;
	Text                 event_sound;
	int                  event_param[10];
	int                  event_nparams;
	int                  event_chance;
	Vec3                 event_point;
	Rect                 event_rect;

	int                  trigger;
	Text                 trigger_ship;
	Text                 trigger_target;
	int                  trigger_param[10];
	int                  trigger_nparams;

	Bitmap               image;
	Sound*               sound;
};


#endif MissionEvent_h


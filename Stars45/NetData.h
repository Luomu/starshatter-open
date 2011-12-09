/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         NetData.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Payload structures for multiplayer network packets
*/

#ifndef NetData_h
#define NetData_h

#include "Types.h"
#include "Geometry.h"
#include "Text.h"

// +--------------------------------------------------------------------+

// UNRELIABLE: 0x01 - 0x0F

const BYTE NET_PING           = 0x01;
const BYTE NET_PONG           = 0x02;
const BYTE NET_OBJ_LOC        = 0x03;

// RELIABLE:   0x10 - 0x7F

const BYTE NET_JOIN_REQUEST   = 0x10;
const BYTE NET_JOIN_ANNOUNCE  = 0x11;
const BYTE NET_QUIT_REQUEST   = 0x12;
const BYTE NET_QUIT_ANNOUNCE  = 0x13;
const BYTE NET_KICK_REQUEST   = 0x14;
const BYTE NET_KICK_ANNOUNCE  = 0x15;
const BYTE NET_GAME_OVER      = 0x16;
const BYTE NET_COMM_MESSAGE   = 0x17;
const BYTE NET_CHAT_MESSAGE   = 0x18;
const BYTE NET_DISCONNECT     = 0x19;

const BYTE NET_OBJ_DAMAGE     = 0x20;
const BYTE NET_OBJ_KILL       = 0x21;
const BYTE NET_OBJ_SPAWN      = 0x22;
const BYTE NET_OBJ_HYPER      = 0x23;
const BYTE NET_OBJ_TARGET     = 0x24;
const BYTE NET_OBJ_EMCON      = 0x25;
const BYTE NET_SYS_DAMAGE     = 0x26;
const BYTE NET_SYS_STATUS     = 0x27;

const BYTE NET_ELEM_CREATE    = 0x28;
const BYTE NET_SHIP_LAUNCH    = 0x29;
const BYTE NET_NAV_DATA       = 0x2A;
const BYTE NET_NAV_DELETE     = 0x2B;
const BYTE NET_ELEM_REQUEST   = 0x2C;

const BYTE NET_WEP_TRIGGER    = 0x30;
const BYTE NET_WEP_RELEASE    = 0x31;
const BYTE NET_WEP_DESTROY    = 0x32;

const BYTE NET_SELF_DESTRUCT  = 0x3F;

// +--------------------------------------------------------------------+

class Ship;

// +--------------------------------------------------------------------+

class NetData
{
public:
	static const char* TYPENAME() { return "NetData"; }

	NetData()          { }
	virtual ~NetData() { }

	virtual int    Type()               const { return 0; }
	virtual int    Length()             const { return 0; }
	virtual BYTE*  Pack()                     { return 0; }
	virtual bool   Unpack(const BYTE* data)   { return 0; }

	virtual DWORD  GetObjID()           const { return 0; }
	virtual void   SetObjID(DWORD o)          {           }
};

// +--------------------------------------------------------------------+

class NetObjLoc : public NetData
{
public:
	static const char* TYPENAME() { return "NetObjLoc"; }

	NetObjLoc() : objid(0), throttle(false), augmenter(false), shield(0) { }
	NetObjLoc(DWORD oid, const Point& pos, const Point& orient, const Point& vel) :
	objid(oid), location(pos), euler(orient), velocity(vel),
	throttle(false), augmenter(false), gear(false), shield(0) { }

	enum { TYPE=NET_OBJ_LOC, SIZE=24 };

	virtual BYTE*  Pack();
	virtual bool   Unpack(const BYTE* data);
	virtual int    Type()                     const { return TYPE;       }
	virtual int    Length()                   const { return SIZE;       }

	virtual DWORD  GetObjID()                 const { return objid;      }
	virtual void   SetObjID(DWORD id)               { objid = id;        }

	Point          GetLocation()              const { return location;   }
	Point          GetVelocity()              const { return velocity;   }
	Point          GetOrientation()           const { return euler;      }
	bool           GetThrottle()              const { return throttle;   }
	bool           GetAugmenter()             const { return augmenter;  }
	bool           GetGearDown()              const { return gear;       }
	int            GetShield()                const { return shield;     }

	void           SetLocation(const Point& loc)    { location = loc; }
	void           SetVelocity(const Point& v)      { velocity = v;   }
	void           SetOrientation(const Point& o)   { euler = o;      }
	void           SetThrottle(bool t)              { throttle = t;   }
	void           SetAugmenter(bool a)             { augmenter = a;  }
	void           SetGearDown(bool g)              { gear = g;       }
	void           SetShield(int s)                 { shield = s;     }

private:
	DWORD    objid;
	Point    location;
	Point    velocity;
	Point    euler;
	bool     throttle;
	bool     augmenter;
	bool     gear;
	int      shield;

	BYTE     data[SIZE];
};

// +--------------------------------------------------------------------+

class NetJoinRequest : public NetData
{
public:
	static const char* TYPENAME() { return "NetJoinRequest"; }

	NetJoinRequest() : index(0) { }

	enum { TYPE=NET_JOIN_REQUEST, SIZE=128 };

	virtual BYTE*     Pack();
	virtual bool      Unpack(const BYTE* data);
	virtual int       Type()                  const { return TYPE;    }
	virtual int       Length()                const { return SIZE;    }

	const char*       GetName()               const { return name;    }
	const char*       GetPassword()           const { return pass;    }
	const char*       GetSerialNumber()       const { return serno;   }
	const char*       GetElement()            const { return elem;    }
	int               GetIndex()              const { return index;   }

	void              SetName(const char* n)        { name = n;       }
	void              SetPassword(const char* p)    { pass = p;       }
	void              SetSerialNumber(const char* s){ serno = s;      }
	void              SetElement(const char* n)     { elem = n;       }
	void              SetIndex(int n)               { index = n;      }

private:
	Text     name;    // callsign
	Text     pass;    // password
	Text     serno;   // box cdkey
	Text     elem;    // element to join
	int      index;   // one-based index

	BYTE     data[SIZE];
};

// +--------------------------------------------------------------------+

class NetJoinAnnounce : public NetData
{
public:
	static const char* TYPENAME() { return "NetJoinAnnounce"; }

	NetJoinAnnounce();

	enum { TYPE=NET_JOIN_ANNOUNCE, SIZE=200 };

	virtual BYTE*     Pack();
	virtual bool      Unpack(const BYTE* data);
	virtual int       Type()                  const { return TYPE;       }
	virtual int       Length()                const { return SIZE;       }

	virtual DWORD     GetObjID()              const { return objid;      }
	virtual void      SetObjID(DWORD o)             { objid = o;         }

	const char*       GetName()               const { return name;       }
	const char*       GetElement()            const { return elem;       }
	const char*       GetRegion()             const { return region;     }
	const Point&      GetLocation()           const { return loc;        }
	const Point&      GetVelocity()           const { return velocity;   }
	int               GetIndex()              const { return index;      }
	double            GetIntegrity()          const { return integrity;  }
	int               GetRespawns()           const { return respawns;   }
	int               GetDecoys()             const { return decoys;     }
	int               GetProbes()             const { return probes;     }
	int               GetFuel()               const { return fuel;       }
	int               GetShield()             const { return shield;     }
	const int*        GetAmmo()               const { return ammo;       }

	void              SetShip(Ship* s);

	void              SetName(const char* n)        { name = n;          }
	void              SetElement(const char* n)     { elem = n;          }
	void              SetRegion(const char* r)      { region = r;        }
	void              SetLocation(const Point& l)   { loc = l;           }
	void              SetVelocity(const Point& v)   { velocity = v;      }
	void              SetIndex(int n)               { index = n;         }
	void              SetIntegrity(double n)        { integrity = (float) n; }
	void              SetRespawns(int n)            { respawns = n;      }
	void              SetDecoys(int n)              { decoys = n;        }
	void              SetProbes(int n)              { probes = n;        }
	void              SetFuel(int n)                { fuel = n;          }
	void              SetShield(int n)              { shield = n;        }
	void              SetAmmo(const int* a);

	virtual DWORD     GetNetID()              const { return nid;        }
	virtual void      SetNetID(DWORD n)             { nid = n;           }

private:
	Text     name;       // callsign
	Text     elem;       // element to join
	Text     region;     // region ship is in
	Point    loc;        // location of ship
	Point    velocity;   // velocity of ship
	int      index;      // one-based index
	float    integrity;  // hull integrity
	int      respawns;
	int      decoys;
	int      probes;
	int      fuel;
	int      shield;
	int      ammo[16];
	DWORD    objid;
	DWORD    nid;        // not sent over network

	BYTE     data[SIZE];
};

// +--------------------------------------------------------------------+

class NetQuitAnnounce : public NetData
{
public:
	static const char* TYPENAME() { return "NetQuitAnnounce"; }

	NetQuitAnnounce() : objid(0), disconnected(false) { }

	enum { TYPE=NET_QUIT_ANNOUNCE, SIZE=5 };

	virtual BYTE*     Pack();
	virtual bool      Unpack(const BYTE* data);
	virtual int       Type()                  const { return TYPE;    }
	virtual int       Length()                const { return SIZE;    }

	virtual DWORD     GetObjID()              const { return objid;   }
	virtual void      SetObjID(DWORD o)             { objid = o;      }
	virtual bool      GetDisconnected()       const { return disconnected; }
	virtual void      SetDisconnected(bool d)       { disconnected = d;    }

private:
	DWORD    objid;
	bool     disconnected;

	BYTE     data[SIZE];
};

// +--------------------------------------------------------------------+

class NetDisconnect : public NetData
{
public:
	static const char* TYPENAME() { return "NetDisconnect"; }

	NetDisconnect() { }

	enum { TYPE=NET_DISCONNECT, SIZE=2 };

	virtual BYTE*     Pack();
	virtual bool      Unpack(const BYTE* data);
	virtual int       Type()                  const { return TYPE;    }
	virtual int       Length()                const { return SIZE;    }

private:
	BYTE     data[SIZE];
};

// +--------------------------------------------------------------------+

class NetObjDamage : public NetData
{
public:
	static const char* TYPENAME() { return "NetObjDamage"; }

	NetObjDamage() : objid(0), damage(0), shotid(0) { }

	enum { TYPE=NET_OBJ_DAMAGE, SIZE=12 };

	virtual BYTE*     Pack();
	virtual bool      Unpack(const BYTE* data);
	virtual int       Type()                  const { return TYPE;    }
	virtual int       Length()                const { return SIZE;    }

	virtual DWORD     GetObjID()              const { return objid;   }
	virtual void      SetObjID(DWORD o)             { objid = o;      }

	virtual DWORD     GetShotID()             const { return shotid;  }
	virtual void      SetShotID(DWORD o)            { shotid = o;     }

	float             GetDamage()             const { return damage;  }
	void              SetDamage(float d)            { damage = d;     }

private:
	DWORD    objid;
	float    damage;
	DWORD    shotid;

	BYTE     data[SIZE];
};

// +--------------------------------------------------------------------+

class NetObjKill : public NetData
{
public:
	static const char* TYPENAME() { return "NetObjKill"; }

	NetObjKill() : objid(0), kill_id(0), killtype(0), respawn(false), deck(0) { }

	enum { TYPE=NET_OBJ_KILL, SIZE=24,
		KILL_MISC = 0, 
		KILL_PRIMARY, 
		KILL_SECONDARY, 
		KILL_COLLISION, 
		KILL_CRASH,
		KILL_DOCK
	};

	virtual BYTE*     Pack();
	virtual bool      Unpack(const BYTE* data);
	virtual int       Type()                  const { return TYPE;    }
	virtual int       Length()                const { return SIZE;    }

	virtual DWORD     GetObjID()              const { return objid;   }
	virtual void      SetObjID(DWORD o)             { objid = o;      }
	virtual DWORD     GetKillerID()           const { return kill_id; }
	virtual void      SetKillerID(DWORD o)          { kill_id = o;    }
	virtual int       GetKillType()           const { return killtype;}
	virtual void      SetKillType(int t)            { killtype = t;   }
	virtual bool      GetRespawn()            const { return respawn; }
	virtual void      SetRespawn(bool r)            { respawn = r;    }
	virtual Point     GetRespawnLoc()         const { return loc;     }
	virtual void      SetRespawnLoc(const Point& p) { loc = p;        }
	virtual int       GetFlightDeck()         const { return deck;    }
	virtual void      SetFlightDeck(int n)          { deck = n;       }

private:
	DWORD    objid;
	DWORD    kill_id;
	int      killtype;
	bool     respawn;
	Point    loc;
	int      deck;

	BYTE     data[SIZE];
};

// +--------------------------------------------------------------------+

class NetObjHyper : public NetData
{
public:
	static const char* TYPENAME() { return "NetObjHyper"; }

	NetObjHyper() : objid(0), fc_src(0), fc_dst(0), transtype(0) { }

	enum { TYPE=NET_OBJ_HYPER, SIZE=56 };

	virtual BYTE*  Pack();
	virtual bool   Unpack(const BYTE* data);
	virtual int    Type()                     const { return TYPE;       }
	virtual int    Length()                   const { return SIZE;       }

	virtual DWORD  GetObjID()                 const { return objid;      }
	virtual void   SetObjID(DWORD id)               { objid = id;        }

	const Point&   GetLocation()              const { return location;   }
	const Text&    GetRegion()                const { return region;     }
	DWORD          GetFarcaster1()            const { return fc_src;     }
	DWORD          GetFarcaster2()            const { return fc_dst;     }
	int            GetTransitionType()        const { return transtype;  }

	void           SetLocation(const Point& loc)    { location  = loc;   }
	void           SetRegion(const char* rgn)       { region    = rgn;   }
	void           SetFarcaster1(DWORD f)           { fc_src    = f;     }
	void           SetFarcaster2(DWORD f)           { fc_dst    = f;     }
	void           SetTransitionType(int t)         { transtype = t;     }

private:
	DWORD    objid;
	Point    location;
	Text     region;
	DWORD    fc_src;
	DWORD    fc_dst;
	int      transtype;

	BYTE     data[SIZE];
};

// +--------------------------------------------------------------------+

class NetObjTarget : public NetData
{
public:
	static const char* TYPENAME() { return "NetObjTarget"; }

	NetObjTarget() : objid(0), tgtid(0), sysix(0) { }

	enum { TYPE=NET_OBJ_TARGET, SIZE=7 };

	virtual BYTE*     Pack();
	virtual bool      Unpack(const BYTE* data);
	virtual int       Type()                  const { return TYPE;    }
	virtual int       Length()                const { return SIZE;    }

	virtual DWORD     GetObjID()              const { return objid;   }
	virtual void      SetObjID(DWORD o)             { objid = o;      }

	DWORD             GetTgtID()              const { return tgtid;   }
	void              SetTgtID(DWORD o)             { tgtid = o;      }
	int               GetSubtarget()          const { return sysix;   }
	void              SetSubtarget(int n)           { sysix = n;      }

private:
	DWORD    objid;
	DWORD    tgtid;
	int      sysix;

	BYTE     data[SIZE];
};

// +--------------------------------------------------------------------+

class NetObjEmcon : public NetData
{
public:
	static const char* TYPENAME() { return "NetObjEmcon"; }

	NetObjEmcon() : objid(0), emcon(0) { }

	enum { TYPE=NET_OBJ_EMCON, SIZE=5 };

	virtual BYTE*     Pack();
	virtual bool      Unpack(const BYTE* data);
	virtual int       Type()                  const { return TYPE;    }
	virtual int       Length()                const { return SIZE;    }

	virtual DWORD     GetObjID()              const { return objid;   }
	virtual void      SetObjID(DWORD o)             { objid = o;      }

	int               GetEMCON()              const { return emcon;   }
	void              SetEMCON(int n)               { emcon = n;      }

private:
	DWORD    objid;
	int      emcon;

	BYTE     data[SIZE];
};

// +--------------------------------------------------------------------+

class NetSysDamage : public NetData
{
public:
	static const char* TYPENAME() { return "NetSysDamage"; }

	NetSysDamage() : objid(0), sysix(-1), dmgtype(0), damage(0) { }

	enum { TYPE=NET_SYS_DAMAGE, SIZE=12 };

	virtual BYTE*     Pack();
	virtual bool      Unpack(const BYTE* data);
	virtual int       Type()                  const { return TYPE;    }
	virtual int       Length()                const { return SIZE;    }

	virtual DWORD     GetObjID()              const { return objid;   }
	virtual void      SetObjID(DWORD o)             { objid = o;      }

	int               GetSystem()             const { return sysix;   }
	void              SetSystem(int n)              { sysix = n;      }
	BYTE              GetDamageType()         const { return dmgtype; }
	void              SetDamageType(BYTE t)         { dmgtype = t;    }
	double            GetDamage()             const { return damage;  }
	void              SetDamage(double d)           { damage = d;     }

private:
	DWORD    objid;
	int      sysix;
	BYTE     dmgtype;
	double   damage;

	BYTE     data[SIZE];
};

// +--------------------------------------------------------------------+

class NetSysStatus : public NetData
{
public:
	static const char* TYPENAME() { return "NetSysStatus"; }

	NetSysStatus() : objid(0), sysix(-1), status(0), power(0), reactor(0),
	avail(1) { }

	enum { TYPE=NET_SYS_STATUS, SIZE=12 };

	virtual BYTE*     Pack();
	virtual bool      Unpack(const BYTE* data);
	virtual int       Type()                  const { return TYPE;    }
	virtual int       Length()                const { return SIZE;    }

	virtual DWORD     GetObjID()              const { return objid;   }
	virtual void      SetObjID(DWORD o)             { objid = o;      }

	int               GetSystem()             const { return sysix;   }
	void              SetSystem(int n)              { sysix = n;      }
	BYTE              GetStatus()             const { return status;  }
	void              SetStatus(BYTE s)             { status = s;     }
	int               GetPower()              const { return power;   }
	void              SetPower(int n)               { power = n;      }
	int               GetReactor()            const { return reactor; }
	void              SetReactor(int n)             { reactor = n;    }
	double            GetAvailability()       const { return avail;   }
	void              SetAvailablility(double a)    { avail = a;      }

private:
	DWORD    objid;
	int      sysix;
	int      status;
	int      power;
	int      reactor;
	double   avail;

	BYTE     data[SIZE];
};

// +--------------------------------------------------------------------+

class NetWepTrigger : public NetData
{
public:
	static const char* TYPENAME() { return "NetWepTrigger"; }

	NetWepTrigger() : objid(0), tgtid(0), sysix(-1), index(0), count(0),
	decoy(false), probe(false) { }

	enum { TYPE=NET_WEP_TRIGGER, SIZE=10 };

	virtual BYTE*     Pack();
	virtual bool      Unpack(const BYTE* data);
	virtual int       Type()                  const { return TYPE;    }
	virtual int       Length()                const { return SIZE;    }

	virtual DWORD     GetObjID()              const { return objid;   }
	virtual void      SetObjID(DWORD o)             { objid = o;      }

	DWORD             GetTgtID()              const { return tgtid;   }
	void              SetTgtID(DWORD o)             { tgtid = o;      }
	int               GetSubtarget()          const { return sysix;   }
	void              SetSubtarget(int n)           { sysix = n;      }
	int               GetIndex()              const { return index;   }
	void              SetIndex(int n)               { index = n;      }
	int               GetCount()              const { return count;   }
	void              SetCount(int n)               { count = n;      }
	bool              GetDecoy()              const { return decoy;   }
	void              SetDecoy(bool d)              { decoy = d;      }
	bool              GetProbe()              const { return probe;   }
	void              SetProbe(bool p)              { probe = p;      }

private:
	DWORD    objid;
	DWORD    tgtid;
	int      sysix;
	int      index;
	int      count;
	bool     decoy;
	bool     probe;

	BYTE     data[SIZE];
};

// +--------------------------------------------------------------------+

class NetWepRelease : public NetData
{
public:
	static const char* TYPENAME() { return "NetWepRelease"; }

	NetWepRelease() : objid(0), tgtid(0), wepid(0), sysix(-1), index(0),
	decoy(false), probe(false) { }

	enum { TYPE=NET_WEP_RELEASE, SIZE=11 };

	virtual BYTE*     Pack();
	virtual bool      Unpack(const BYTE* data);
	virtual int       Type()                  const { return TYPE;    }
	virtual int       Length()                const { return SIZE;    }

	virtual DWORD     GetObjID()              const { return objid;   }
	virtual void      SetObjID(DWORD o)             { objid = o;      }

	DWORD             GetTgtID()              const { return tgtid;   }
	void              SetTgtID(DWORD o)             { tgtid = o;      }
	int               GetSubtarget()          const { return sysix;   }
	void              SetSubtarget(int n)           { sysix = n;      }
	DWORD             GetWepID()              const { return wepid;   }
	void              SetWepID(DWORD o)             { wepid = o;      }
	int               GetIndex()              const { return index;   }
	void              SetIndex(int n)               { index = n;      }
	bool              GetDecoy()              const { return decoy;   }
	void              SetDecoy(bool d)              { decoy = d;      }
	bool              GetProbe()              const { return probe;   }
	void              SetProbe(bool p)              { probe = p;      }

private:
	DWORD    objid;
	DWORD    tgtid;
	DWORD    wepid;
	int      sysix;
	int      index;
	bool     decoy;
	bool     probe;

	BYTE     data[SIZE];
};

// +--------------------------------------------------------------------+

class NetWepDestroy : public NetData
{
public:
	static const char* TYPENAME() { return "NetWepDestroy"; }

	NetWepDestroy() : objid(0) { }

	enum { TYPE=NET_WEP_DESTROY, SIZE=4 };

	virtual BYTE*     Pack();
	virtual bool      Unpack(const BYTE* data);
	virtual int       Type()                  const { return TYPE;    }
	virtual int       Length()                const { return SIZE;    }

	virtual DWORD     GetObjID()              const { return objid;   }
	virtual void      SetObjID(DWORD o)             { objid = o;      }

private:
	DWORD    objid;

	BYTE     data[SIZE];
};

// +--------------------------------------------------------------------+

class RadioMessage;
class NetCommMsg : public NetData
{
public:
	static const char* TYPENAME() { return "NetCommMsg"; }

	NetCommMsg() : objid(0), radio_message(0), length(0) { }
	virtual ~NetCommMsg();

	enum { TYPE=NET_COMM_MESSAGE, SIZE=200 };

	virtual BYTE*     Pack();
	virtual bool      Unpack(const BYTE* data);
	virtual int       Type()                  const { return TYPE;    }
	virtual int       Length()                const { return length;  }

	virtual DWORD     GetObjID()              const { return objid;   }
	virtual void      SetObjID(DWORD o)             { objid = o;      }

	RadioMessage*     GetRadioMessage()             { return radio_message; }
	void              SetRadioMessage(RadioMessage* m);

private:
	DWORD          objid;
	RadioMessage*  radio_message;

	int            length;
	BYTE           data[SIZE];
};

// +--------------------------------------------------------------------+

class NetChatMsg : public NetData
{
public:
	static const char* TYPENAME() { return "NetChatMsg"; }

	NetChatMsg() : dstid(0), length(0) { }

	enum { TYPE=NET_CHAT_MESSAGE, SIZE=210, MAX_CHAT=160, HDR_LEN=4, NAME_LEN=32 };

	virtual BYTE*     Pack();
	virtual bool      Unpack(const BYTE* data);
	virtual int       Type()                  const { return TYPE;    }
	virtual int       Length()                const { return length;  }

	virtual DWORD     GetDstID()              const { return dstid;   }
	virtual void      SetDstID(DWORD d)             { dstid = d;      }
	const Text&       GetName()               const { return name;    }
	void              SetName(const char* m)        { name = m;       }
	const Text&       GetText()               const { return text;    }
	void              SetText(const char* m)        { text = m;       }

private:
	DWORD          dstid;
	Text           name;
	Text           text;

	int            length;
	BYTE           data[SIZE];
};

// +--------------------------------------------------------------------+

class NetElemRequest : public NetData
{
public:
	static const char* TYPENAME() { return "NetElemRequest"; }

	NetElemRequest();

	enum { TYPE=NET_ELEM_REQUEST, SIZE=64, NAME_LEN=32 };

	virtual BYTE*     Pack();
	virtual bool      Unpack(const BYTE* data);
	virtual int       Type()                  const { return TYPE;       }
	virtual int       Length()                const { return SIZE;       }

	const Text&       GetName()               const { return name;       }
	void              SetName(const char* m)        { name = m;          }

private:
	Text           name;

	BYTE           data[SIZE];
};

// +--------------------------------------------------------------------+

class NetElemCreate : public NetData
{
public:
	static const char* TYPENAME() { return "NetElemCreate"; }

	NetElemCreate();

	enum { TYPE=NET_ELEM_CREATE, SIZE=192, NAME_LEN=32 };

	virtual BYTE*     Pack();
	virtual bool      Unpack(const BYTE* data);
	virtual int       Type()                  const { return TYPE;       }
	virtual int       Length()                const { return SIZE;       }

	const Text&       GetName()               const { return name;       }
	void              SetName(const char* m)        { name = m;          }
	const Text&       GetCommander()          const { return commander;  }
	void              SetCommander(const char* m)   { commander = m;     }
	const Text&       GetObjective()         const  { return objective;  }
	void              SetObjective(const char* m)   { objective = m;     }
	const Text&       GetCarrier()            const { return carrier;    }
	void              SetCarrier(const char* m)     { carrier = m;       }

	int               GetIFF()                const { return iff;        }
	void              SetIFF(int n)                 { iff = n;           }
	int               GetType()               const { return type;       }
	void              SetType(int n)                { type = n;          }
	int               GetIntel()              const { return intel;      }
	void              SetIntel(int n)               { intel = n;         }
	int               GetObjCode()            const { return obj_code;   }
	void              SetObjCode(int n)             { obj_code = n;      }
	int               GetSquadron()           const { return squadron;   }
	void              SetSquadron(int n)            { squadron = n;      }

	int*              GetLoadout()                  { return load;       }
	void              SetLoadout(int* n);
	int*              GetSlots()                    { return slots;      }
	void              SetSlots(int* n);

	bool              GetAlert()              const { return alert;      }
	void              SetAlert(bool a)              { alert = a;         }

	bool              GetInFlight()           const { return in_flight;  }
	void              SetInFlight(bool f)           { in_flight = f;     }

private:
	Text           name;
	int            iff;
	int            type;
	int            intel;
	int            obj_code;
	int            squadron;

	Text           commander;
	Text           objective;
	Text           carrier;

	int            load[16];
	int            slots[4];
	bool           alert;
	bool           in_flight;

	BYTE           data[SIZE];
};

// +--------------------------------------------------------------------+

class NetShipLaunch : public NetData
{
public:
	static const char* TYPENAME() { return "NetShipLaunch"; }

	NetShipLaunch() : objid(0), squadron(0), slot(0) { }

	enum { TYPE=NET_SHIP_LAUNCH, SIZE=16 };

	virtual BYTE*     Pack();
	virtual bool      Unpack(const BYTE* data);
	virtual int       Type()                  const { return TYPE;       }
	virtual int       Length()                const { return SIZE;       }

	virtual DWORD     GetObjID()              const { return objid;      }
	virtual int       GetSquadron()           const { return squadron;   }
	virtual int       GetSlot()               const { return slot;       }

	virtual void      SetObjID(DWORD o)             { objid = o;         }
	virtual void      SetSquadron(int s)            { squadron = s;      }
	virtual void      SetSlot(int s)                { slot = s;          }

private:
	DWORD          objid;
	int            squadron;
	int            slot;

	BYTE           data[SIZE];
};

// +--------------------------------------------------------------------+

class Instruction;

class NetNavData : public NetData
{
public:
	static const char* TYPENAME() { return "NetNavData"; }

	NetNavData();
	virtual ~NetNavData();

	enum { TYPE=NET_NAV_DATA, SIZE=144, NAME_LEN=32 };

	virtual BYTE*     Pack();
	virtual bool      Unpack(const BYTE* data);
	virtual int       Type()                  const { return TYPE;       }
	virtual int       Length()                const { return SIZE;       }

	virtual DWORD     GetObjID()              const { return objid;      }
	bool              IsAdd()                 const { return create;     }
	bool              IsEdit()                const { return !create;    }
	const Text&       GetElem()               const { return elem;       }
	int               GetIndex()              const { return index;      }
	Instruction*      GetNavPoint()           const { return navpoint;   }

	virtual void      SetObjID(DWORD o)             { objid = o;         }
	void              SetAdd(bool b)                { create = b;        }
	void              SetElem(const char* e)        { elem = e;          }
	void              SetIndex(int n)               { index = n;         }
	void              SetNavPoint(Instruction* n);

private:
	DWORD             objid;
	bool              create;
	Text              elem;
	int               index;
	Instruction*      navpoint;

	BYTE              data[SIZE];
};

// +--------------------------------------------------------------------+

class NetNavDelete : public NetData
{
public:
	static const char* TYPENAME() { return "NetNavDelete"; }

	NetNavDelete() : objid(0), index(0) { }

	enum { TYPE=NET_NAV_DELETE, SIZE=40 };

	virtual BYTE*     Pack();
	virtual bool      Unpack(const BYTE* data);
	virtual int       Type()                  const { return TYPE;       }
	virtual int       Length()                const { return SIZE;       }

	virtual DWORD     GetObjID()              const { return objid;      }
	const Text&       GetElem()               const { return elem;       }
	int               GetIndex()              const { return index;      }

	virtual void      SetObjID(DWORD o)             { objid = o;         }
	void              SetElem(const char* e)        { elem = e;          }
	void              SetIndex(int n)               { index = n;         }

private:
	DWORD             objid;
	Text              elem;
	int               index;

	BYTE              data[SIZE];
};

// +--------------------------------------------------------------------+

class NetSelfDestruct : public NetData
{
public:
	static const char* TYPENAME() { return "NetSelfDestruct"; }

	NetSelfDestruct() : objid(0), damage(0) { }

	enum { TYPE=NET_SELF_DESTRUCT, SIZE=8 };

	virtual BYTE*     Pack();
	virtual bool      Unpack(const BYTE* data);
	virtual int       Type()                  const { return TYPE;    }
	virtual int       Length()                const { return SIZE;    }

	virtual DWORD     GetObjID()              const { return objid;   }
	virtual void      SetObjID(DWORD o)             { objid = o;      }

	float             GetDamage()             const { return damage;  }
	void              SetDamage(float d)            { damage = d;     }

private:
	DWORD    objid;
	float    damage;

	BYTE     data[SIZE];
};


// +--------------------------------------------------------------------+

#endif NetData_h


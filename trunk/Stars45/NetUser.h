/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         NetUser.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	This class represents a user connecting to the multiplayer lobby
*/


#ifndef NetUser_h
#define NetUser_h

#include "Types.h"
#include "NetAddr.h"
#include "NetLobby.h"
#include "Color.h"

// +-------------------------------------------------------------------+

class Player;

// +-------------------------------------------------------------------+

class NetUser
{
public:
	static const char* TYPENAME() { return "NetUser"; }

	NetUser(const char* name);
	NetUser(const Player* player);
	virtual ~NetUser();

	int operator == (const NetUser& u) const { return this == &u; }

	const Text&    Name()         const { return name;          }
	const Text&    Pass()         const { return pass;          }
	const NetAddr& GetAddress()   const { return addr;          }
	Color          GetColor()     const { return color;         }
	const Text&    GetSessionID() const { return session_id;    }
	DWORD          GetNetID()     const { return netid;         }
	bool           IsHost()       const { return host;          }

	int            AuthLevel()    const { return auth_level;    }
	int            AuthState()    const { return auth_state;    }
	const char*    Salt()         const { return salt;          }
	bool           IsAuthOK()     const;

	const Text&    Squadron()     const { return squadron;      }
	const Text&    Signature()    const { return signature;     }
	int            Rank()         const { return rank;          }
	int            FlightTime()   const { return flight_time;   }
	int            Missions()     const { return missions;      }
	int            Kills()        const { return kills;         }
	int            Losses()       const { return losses;        }

	void           SetName(const char* n)     { name = n;       }
	void           SetPass(const char* p)     { pass = p;       }
	void           SetAddress(const NetAddr& a)
	{ addr = a;       }

	void           SetColor(Color c)          { color = c;      }
	void           SetNetID(DWORD id)         { netid = id;     }
	void           SetSessionID(Text s)       { session_id = s; }
	void           SetHost(bool h)            { host = h;       }

	void           SetAuthLevel(int n)        { auth_level = n; }
	void           SetAuthState(int n)        { auth_state = n; }
	void           SetSalt(const char* s)     { strcpy(salt, s);}

	void           SetSquadron(const char* s) { squadron = s;   }
	void           SetSignature(const char* s){ signature = s;  }
	void           SetRank(int n)             { rank = n;       }
	void           SetFlightTime(int n)       { flight_time = n;}
	void           SetMissions(int n)         { missions = n;   }
	void           SetKills(int n)            { kills = n;      }
	void           SetLosses(int n)           { losses = n;     }

	Text           GetDescription();

protected:
	Text        name;
	Text        pass;
	Text        session_id;
	NetAddr     addr;
	DWORD       netid;
	Color       color;
	bool        host;

	// authentication:
	int         auth_state;
	int         auth_level;
	char        salt[33];

	// stats:
	Text        squadron;
	Text        signature;
	int         rank;
	int         flight_time;
	int         missions;
	int         kills;
	int         losses;
};

// +-------------------------------------------------------------------+

#endif NetUser_h
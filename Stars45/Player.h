/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         Player.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Player / Logbook class
*/


#ifndef Player_h
#define Player_h

#include "Types.h"
#include "List.h"
#include "Text.h"

// +-------------------------------------------------------------------+

class Player;
class Bitmap;
class ShipStats;
class AwardInfo;
class Sound;

// +-------------------------------------------------------------------+

class Player
{
public:
	static const char* TYPENAME() { return "Player"; }

	Player(const char* name);
	virtual ~Player();

	int operator == (const Player& u) const { return name == u.name; }

	int            Identity()     const { return uid;           }
	const Text&    Name()         const { return name;          }
	const Text&    Password()     const { return pass;          }
	const Text&    Squadron()     const { return squadron;      }
	const Text&    Signature()    const { return signature;     }
	const Text&    ChatMacro(int n) const;
	int            CreateDate()   const { return create_date;   }
	int            Rank()         const;
	int            Medal(int n)   const;
	int            Points()       const { return points;        }
	int            Medals()       const { return medals;        }
	int            FlightTime()   const { return flight_time;   }
	int            Missions()     const { return missions;      }
	int            Kills()        const { return kills;         }
	int            Losses()       const { return losses;        }
	int            Campaigns()    const { return campaigns;     }
	int            Trained()      const { return trained;       }

	int            FlightModel()  const { return flight_model;  }
	int            FlyingStart()  const { return flying_start;  }
	int            LandingModel() const { return landing_model; }
	int            AILevel()      const { return ai_level;      }
	int            HUDMode()      const { return hud_mode;      }
	int            HUDColor()     const { return hud_color;     }
	int            FriendlyFire() const { return ff_level;      }
	int            GridMode()     const { return grid;          }
	int            Gunsight()     const { return gunsight;      }

	bool           ShowAward()    const { return award != 0;    }
	Text           AwardName()    const;
	Text           AwardDesc()    const;
	Bitmap*        AwardImage()   const;
	Sound*         AwardSound()   const;

	bool           CanCommand(int ship_class);

	void           SetName(const char* n);
	void           SetPassword(const char* p);
	void           SetSquadron(const char* s);
	void           SetSignature(const char* s);
	void           SetChatMacro(int n, const char* m);
	void           SetCreateDate(int d);
	void           SetRank(int r);
	void           SetPoints(int p);
	void           SetMedals(int m);
	void           SetCampaigns(int n);
	void           SetTrained(int n);
	void           SetFlightTime(int t);
	void           SetMissions(int m);
	void           SetKills(int k);
	void           SetLosses(int l);

	void           AddFlightTime(int t);
	void           AddPoints(int p);
	void           AddMedal(int m);
	void           AddMissions(int m);
	void           AddKills(int k);
	void           AddLosses(int l);

	bool           HasTrained(int n)            const;
	bool           HasCompletedCampaign(int id) const;
	void           SetCampaignComplete(int id);

	void           SetFlightModel(int n);
	void           SetFlyingStart(int n);
	void           SetLandingModel(int n);
	void           SetAILevel(int n);
	void           SetHUDMode(int n);
	void           SetHUDColor(int n);
	void           SetFriendlyFire(int n);
	void           SetGridMode(int n);
	void           SetGunsight(int n);

	void           ClearShowAward();

	Text           EncodeStats();
	void           DecodeStats(const char* stats);

	int            GetMissionPoints(ShipStats* stats, DWORD start_time);
	void           ProcessStats(ShipStats* stats, DWORD start_time);
	bool           EarnedAward(AwardInfo* a, ShipStats* s);

	static const char*   RankName(int rank);
	static const char*   RankAbrv(int rank);
	static int           RankFromName(const char* name);
	static Bitmap*       RankInsignia(int rank, int size);
	static const char*   RankDescription(int rank);
	static const char*   MedalName(int medal);
	static Bitmap*       MedalInsignia(int medal, int size);
	static const char*   MedalDescription(int medal);
	static int           CommandRankRequired(int ship_class);

	static List<Player>& GetRoster();
	static Player*       GetCurrentPlayer();
	static void          SelectPlayer(Player* p);
	static Player*       Create(const char* name);
	static void          Destroy(Player* p);
	static Player*       Find(const char* name);
	static void          Initialize();
	static void          Close();
	static void          Load();
	static void          Save();
	static bool          ConfigExists();
	static void          LoadAwardTables();

protected:
	Player();

	void                 CreateUniqueID();

	int         uid;
	Text        name;
	Text        pass;
	Text        squadron;
	Text        signature;
	Text        chat_macros[10];
	int         mfd[4];

	// stats:
	int         create_date;
	int         points;
	int         medals;        // bitmap of earned medals
	int         flight_time;
	int         missions;
	int         kills;
	int         losses;
	int         campaigns;     // bitmap of completed campaigns
	int         trained;       // id of highest training mission completed

	// gameplay options:
	int         flight_model;
	int         flying_start;
	int         landing_model;
	int         ai_level;
	int         hud_mode;
	int         hud_color;
	int         ff_level;
	int         grid;
	int         gunsight;

	// transient:
	AwardInfo*  award;
};

#endif Player_h
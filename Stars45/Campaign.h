/*  Starshatter OpenSource Distribution
    Copyright (c) 1997-2004, Destroyer Studios LLC.
    All Rights Reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.
    * Neither the name "Destroyer Studios" nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

    SUBSYSTEM:    Stars.exe
    FILE:         Campaign.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Campaign defines a strategic military scenario.  This class
    owns (or generates) the Mission list that defines the action
    in the campaign.
*/

#ifndef Campaign_h
#define Campaign_h

#include "Types.h"
#include "Bitmap.h"
#include "Geometry.h"
#include "Text.h"
#include "Term.h"
#include "List.h"

// +--------------------------------------------------------------------+

class Campaign;
class CampaignPlan;
class Combatant;
class CombatAction;
class CombatEvent;
class CombatGroup;
class CombatUnit;
class CombatZone;
class DataLoader;
class Mission;
class MissionTemplate;
class StarSystem;

// +--------------------------------------------------------------------+

class MissionInfo
{
public:
    static const char* TYPENAME() { return "MissionInfo"; }

    MissionInfo();
    ~MissionInfo();

    int operator == (const MissionInfo& m) const { return id == m.id; }
    int operator <  (const MissionInfo& m) const { return id <  m.id; }
    int operator <= (const MissionInfo& m) const { return id <= m.id; }

    bool     IsAvailable();

    int      id;
    Text     name;
    Text     player_info;
    Text     description;
    Text     system;
    Text     region;
    Text     script;
    int      start;
    int      type;

    int      min_rank;
    int      max_rank;
    int      action_id;
    int      action_status;
    int      exec_once;
    int      start_before;
    int      start_after;

    Mission* mission;
};

class TemplateList
{
public:
    static const char* TYPENAME() { return "TemplateList"; }

    TemplateList();
    ~TemplateList();

    int               mission_type;
    int               group_type;
    int               index;
    List<MissionInfo> missions;
};

// +--------------------------------------------------------------------+

class Campaign
{
public:
    static const char* TYPENAME() { return "Campaign"; }

    enum CONSTANTS {
        TRAINING_CAMPAIGN    =    1,
        DYNAMIC_CAMPAIGN,
        MOD_CAMPAIGN         =  100,
        SINGLE_MISSIONS      = 1000,
        MULTIPLAYER_MISSIONS,
        CUSTOM_MISSIONS,

        NUM_IMAGES           = 6
    };

    enum STATUS {
        CAMPAIGN_INIT,
        CAMPAIGN_ACTIVE,
        CAMPAIGN_SUCCESS,
        CAMPAIGN_FAILED
    };

    Campaign(int id, const char* name=0);
    Campaign(int id, const char* name, const char* path);
    virtual ~Campaign();

    int operator == (const Campaign& s) const   { return name == s.name; }
    int operator <  (const Campaign& s) const   { return campaign_id < s.campaign_id; }

    // operations:
    virtual void         Load();
    virtual void         Prep();
    virtual void         Start();
    virtual void         ExecFrame();
    virtual void         Unload();

    virtual void         Clear();
    virtual void         CommitExpiredActions();
    virtual void         LockoutEvents(int seconds);
    virtual void         CheckPlayerGroup();
    void                 CreatePlanners();

    // accessors:
    const char*          Name()         const { return name;          }
    const char*          Description()  const { return description;   }
    const char*          Path()         const { return path;          }

    const char*          Situation()    const { return situation;     }
    const char*          Orders()       const { return orders;        }

    void                 SetSituation(const char* s) { situation = s; }
    void                 SetOrders(const char* o)    { orders = o;    }

    int                  GetPlayerTeamScore();
    List<MissionInfo>&   GetMissionList()     { return missions;      }
    List<Combatant>&     GetCombatants()      { return combatants;    }
    List<CombatZone>&    GetZones()           { return zones;         }
    List<StarSystem>&    GetSystemList()      { return systems;       }
    List<CombatAction>&  GetActions()         { return actions;       }
    List<CombatEvent>&   GetEvents()          { return events;        }
    CombatEvent*         GetLastEvent();

    CombatAction*        FindAction(int id);

    int                  CountNewEvents() const;

    int                  GetPlayerIFF();
    CombatGroup*         GetPlayerGroup()     { return player_group;  }
    void                 SetPlayerGroup(CombatGroup* pg);
    CombatUnit*          GetPlayerUnit()      { return player_unit;   }
    void                 SetPlayerUnit(CombatUnit* pu);

    Combatant*           GetCombatant(const char* name);
    CombatGroup*         FindGroup(int iff, int type, int id);
    CombatGroup*         FindGroup(int iff, int type, CombatGroup* near_group=0);
    CombatGroup*         FindStrikeTarget(int iff, CombatGroup* strike_group);

    StarSystem*          GetSystem(const char* sys);
    CombatZone*          GetZone(const char* rgn);
    MissionInfo*         CreateNewMission();
    void                 DeleteMission(int id);
    Mission*             GetMission();
    Mission*             GetMission(int id);
    Mission*             GetMissionByFile(const char* filename);
    MissionInfo*         GetMissionInfo(int id);
    MissionInfo*         FindMissionTemplate(int msn_type, CombatGroup* player_group);
    void                 ReloadMission(int id);
    void                 LoadNetMission(int id, const char* net_mission);
    void                 StartMission();
    void                 RollbackMission();

    void                 SetCampaignId(int id);
    int                  GetCampaignId()   const { return campaign_id; }
    void                 SetMissionId(int id);
    int                  GetMissionId()    const { return mission_id;  }
    Bitmap*              GetImage(int n)         { return &image[n];   }
    double               GetTime()         const { return time;        }
    double               GetStartTime()    const { return startTime;   }
    void                 SetStartTime(double t)  { startTime = t;      }
    double               GetLoadTime()     const { return loadTime;    }
    void                 SetLoadTime(double t)   { loadTime = t;       }
    double               GetUpdateTime()   const { return updateTime;  }
    void                 SetUpdateTime(double t) { updateTime = t;     }

    bool                 InCutscene()      const;
    bool                 IsDynamic()       const;
    bool                 IsTraining()      const;
    bool                 IsScripted()      const;
    bool                 IsSequential()    const;
    bool                 IsSaveGame()      const { return loaded_from_savegame;       }
    void                 SetSaveGame(bool s)     { loaded_from_savegame = s;          }

    bool                 IsActive()        const { return status == CAMPAIGN_ACTIVE;  }
    bool                 IsComplete()      const { return status == CAMPAIGN_SUCCESS; }
    bool                 IsFailed()        const { return status == CAMPAIGN_FAILED;  }
    void                 SetStatus(int s);
    int                  GetStatus()       const { return status;                     }

    int                  GetAllCombatUnits(int iff, List<CombatUnit>& units);

    static void          Initialize();
    static void          Close();
    static Campaign*     GetCampaign();
    static List<Campaign>&
    GetAllCampaigns();
    static int           GetLastCampaignId();
    static Campaign*     SelectCampaign(const char* name);
    static Campaign*     CreateCustomCampaign(const char* name, const char* path);

    static double        Stardate();

protected:
    void                 LoadCampaign(DataLoader* loader, bool full=false);
    void                 LoadTemplateList(DataLoader* loader);
    void                 LoadMissionList(DataLoader* loader);
    void                 LoadCustomMissions(DataLoader* loader);
    void                 ParseGroup(TermStruct*  val,
    CombatGroup* force,
    CombatGroup* clone,
    const char*  filename);
    void                 ParseAction(TermStruct* val,
    const char* filename);
    CombatGroup*         CloneOver(CombatGroup*  force,
    CombatGroup*  clone,
    CombatGroup*  group);
    void                 SelectDefaultPlayerGroup(CombatGroup* g, int type);
    TemplateList*        GetTemplateList(int msn_type, int grp_type);

    // attributes:
    int                  campaign_id;
    int                  status;
    char                 filename[64];
    char                 path[64];
    Text                 name;
    Text                 description;
    Text                 situation;
    Text                 orders;
    Bitmap               image[NUM_IMAGES];

    bool                 scripted;
    bool                 sequential;
    bool                 loaded_from_savegame;

    List<Combatant>      combatants;
    List<StarSystem>     systems;
    List<CombatZone>     zones;
    List<CampaignPlan>   planners;
    List<MissionInfo>    missions;
    List<TemplateList>   templates;
    List<CombatAction>   actions;
    List<CombatEvent>    events;
    CombatGroup*         player_group;
    CombatUnit*          player_unit;

    int                  mission_id;
    Mission*             mission;
    Mission*             net_mission;

    double               time;
    double               loadTime;
    double               startTime;
    double               updateTime;
    int                  lockout;
};

#endif Campaign_h


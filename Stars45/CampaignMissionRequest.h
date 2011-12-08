/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         CampaignMissionRequest.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    CampaignMissionRequest 
*/

#ifndef CampaignMissionRequest_h
#define CampaignMissionRequest_h

#include "Types.h"
#include "Geometry.h"
#include "List.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class Campaign;
class CombatGroup;
class CombatUnit;
class CombatZone;
class Mission;
class MissionElement;
class MissionInfo;

// +--------------------------------------------------------------------+

class CampaignMissionRequest
{
public:
   static const char* TYPENAME() { return "CampaignMissionRequest"; }

   CampaignMissionRequest(Campaign* c, int type, int start, 
                          CombatGroup* primary, CombatGroup* tgt=0);

   Campaign*         GetCampaign()        { return campaign;         }
   int               Type()               { return type;             }
   int               OpposingType()       { return opp_type;         }
   int               StartTime()          { return start;            }
   CombatGroup*      GetPrimaryGroup()    { return primary_group;    }
   CombatGroup*      GetSecondaryGroup()  { return secondary_group;  }
   CombatGroup*      GetObjective()       { return objective;        }

   bool              IsLocSpecified()     { return use_loc;          }
   const Text&       RegionName()         { return region;           }
   Point             Location()           { return location;         }
   const Text&       Script()             { return script;           }

   void              SetType(int t)                      { type = t;             }
   void              SetOpposingType(int t)              { opp_type = t;         }
   void              SetStartTime(int s)                 { start = s;            }
   void              SetPrimaryGroup(CombatGroup* g)     { primary_group = g;    }
   void              SetSecondaryGroup(CombatGroup* g)   { secondary_group = g;  }
   void              SetObjective(CombatGroup* g)        { objective = g;        }

   void              SetRegionName(const char* rgn)      { region = rgn;   use_loc = true; }
   void              SetLocation(const Point& loc)       { location = loc; use_loc = true; }
   void              SetScript(const char* s)            { script = s; }

private:
   Campaign*         campaign;

   int               type;             // type of mission
   int               opp_type;         // opposing mission type
   int               start;            // start time
   CombatGroup*      primary_group;    // player's group
   CombatGroup*      secondary_group;  // optional support group
   CombatGroup*      objective;        // target or ward

   bool              use_loc;          // use the specified location
   Text              region;
   Point             location;
   Text              script;
};

#endif CampaignMissionRequest_h


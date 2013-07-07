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


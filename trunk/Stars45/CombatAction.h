/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         CombatAction.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    A planned action (mission/story/strategy) in a dynamic campaign.
*/

#ifndef CombatAction_h
#define CombatAction_h

#include "Types.h"
#include "Geometry.h"
#include "Text.h"
#include "List.h"

// +--------------------------------------------------------------------+

class Combatant;
class CombatAction;
class CombatActionReq;

// +--------------------------------------------------------------------+

class CombatAction
{
public:
   static const char* TYPENAME() { return "CombatAction"; }

   enum  TYPE 
         {
            NO_ACTION,
            STRATEGIC_DIRECTIVE,
            ZONE_ASSIGNMENT,
            SYSTEM_ASSIGNMENT,
            MISSION_TEMPLATE,
            COMBAT_EVENT,
            INTEL_EVENT,
            CAMPAIGN_SITUATION,
            CAMPAIGN_ORDERS
         };

   enum  STATUS 
         {  
            PENDING,
            ACTIVE,
            SKIPPED,
            FAILED,
            COMPLETE
         };

   CombatAction(int id, int type, int subtype, int team);
   ~CombatAction();

   int operator == (const CombatAction& a)  const { return id == a.id; }

   bool                 IsAvailable()  const;
   void                 FireAction();
   void                 FailAction();
   void                 AddRequirement(int action, int stat, bool not = false);
   void                 AddRequirement(Combatant* c1, Combatant* c2, int comp, int score);
   void                 AddRequirement(Combatant* c1, int group_type, int group_id, int comp, int score, int intel=0);
   static int           TypeFromName(const char* n);
   static int           StatusFromName(const char* n);

   // accessors/mutators:
   int                  Identity()     const { return id;            }
   int                  Type()         const { return type;          }
   int                  Subtype()      const { return subtype;       }
   int                  OpposingType() const { return opp_type;      }
   int                  GetIFF()       const { return team;          }
   int                  Status()       const { return status;        }
   int                  Source()       const { return source;        }
   Point                Location()     const { return loc;           }
   const char*          System()       const { return system;        }
   const char*          Region()       const { return region;        }
   const char*          Filename()     const { return text_file;     }
   const char*          ImageFile()    const { return image_file;    }
   const char*          SceneFile()    const { return scene_file;    }
   int                  Count()        const { return count;         }
   int                  ExecTime()     const { return time;          }
   int                  StartBefore()  const { return start_before;  }
   int                  StartAfter()   const { return start_after;   }
   int                  MinRank()      const { return min_rank;      }
   int                  MaxRank()      const { return max_rank;      }
   int                  Delay()        const { return delay;         }
   int                  Probability()  const { return probability;   }
   int                  AssetType()    const { return asset_type;    }
   int                  AssetId()      const { return asset_id;      }
   List<Text>&          AssetKills()         { return asset_kills;   }
   int                  TargetType()   const { return target_type;   }
   int                  TargetId()     const { return target_id;     }
   int                  TargetIFF()    const { return target_iff;    }
   List<Text>&          TargetKills()        { return target_kills;  }
   const char*          GetText()      const { return text;          }

   void                 SetType(int t)        { type = (char) t;     }
   void                 SetSubtype(int s)     { subtype = (char) s;  }
   void                 SetOpposingType(int t){ opp_type = (char) t; }
   void                 SetIFF(int t)         { team = (char) t;     }
   void                 SetStatus(int s)      { status = (char) s;   }
   void                 SetSource(int s)      { source = s;          }
   void                 SetLocation(const Point& p) { loc = p;       }
   void                 SetSystem(Text sys)   { system = sys;        }
   void                 SetRegion(Text rgn)   { region = rgn;        }
   void                 SetFilename(Text f)   { text_file = f;       }
   void                 SetImageFile(Text f)  { image_file = f;      }
   void                 SetSceneFile(Text f)  { scene_file = f;      }
   void                 SetCount(int n)       { count = (char) n;    }
   void                 SetExecTime(int t)    { time = t;            }
   void                 SetStartBefore(int s) { start_before = s;    }
   void                 SetStartAfter(int s)  { start_after = s;     }
   void                 SetMinRank(int n)     { min_rank = (char) n; }
   void                 SetMaxRank(int n)     { max_rank = (char) n; }
   void                 SetDelay(int d)       { delay = d;           }
   void                 SetProbability(int n) { probability = n;     }
   void                 SetAssetType(int t)   { asset_type = t;      }
   void                 SetAssetId(int n)     { asset_id = n;        }
   void                 SetTargetType(int t)  { target_type = t;     }
   void                 SetTargetId(int n)    { target_id = n;       }
   void                 SetTargetIFF(int n)   { target_iff = n;      }
   void                 SetText(Text t)       { text = t;            }


private:
   int                  id;
   char                 type;
   char                 subtype;
   char                 opp_type;
   char                 team;
   char                 status;
   char                 min_rank;
   char                 max_rank;
   int                  source;
   Point                loc;
   Text                 system;
   Text                 region;
   Text                 text_file;
   Text                 image_file;
   Text                 scene_file;
   char                 count;
   int                  start_before;
   int                  start_after;
   int                  delay;
   int                  probability;
   int                  rval;
   int                  time;

   Text                 text;
   int                  asset_type;
   int                  asset_id;
   List<Text>           asset_kills;
   int                  target_type;
   int                  target_id;
   int                  target_iff;
   List<Text>           target_kills;

   List<CombatActionReq> requirements;
};

// +--------------------------------------------------------------------+

class CombatActionReq {
public:
   static const char* TYPENAME() { return "CombatActionReq"; }

   enum COMPARISON_OPERATOR {
      LT,  LE,  GT,  GE,  EQ,    // absolute score comparison
      RLT, RLE, RGT, RGE, REQ    // delta score comparison
   };

   CombatActionReq(int a, int s, bool n = false)
      : action(a), stat(s), not(n), c1(0), c2(0), comp(0), score(0), intel(0) { }

   CombatActionReq(Combatant* a1, Combatant* a2, int comparison, int value)
      : action(0), stat(0), not(0), c1(a1), c2(a2), group_type(0), group_id(0),
        comp(comparison), score(value), intel(0) { }

   CombatActionReq(Combatant* a1, int gtype, int gid, int comparison, int value, int intel_level=0)
      : action(0), stat(0), not(0), c1(a1), c2(0), group_type(gtype), group_id(gid),
        comp(comparison), score(value), intel(intel_level) { }

   static int CompFromName(const char* sym);

   int   action;
   int   stat;
   bool  not;

   Combatant*  c1;
   Combatant*  c2;
   int         comp;
   int         score;
   int         intel;
   int         group_type;
   int         group_id;
};

#endif CombatAction_h


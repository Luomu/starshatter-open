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
    FILE:         CombatEvent.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    A significant (newsworthy) event in the dynamic campaign.
*/

#ifndef CombatEvent_h
#define CombatEvent_h

#include "Types.h"
#include "Geometry.h"
#include "Bitmap.h"
#include "Text.h"
#include "List.h"

// +--------------------------------------------------------------------+

class Campaign;
class CombatGroup;
class CombatUnit;

// +--------------------------------------------------------------------+

class CombatEvent
{
public:
    static const char* TYPENAME() { return "CombatEvent"; }

    enum EVENT_TYPE {
        ATTACK,
        DEFEND,
        MOVE_TO,
        CAPTURE,
        STRATEGY,

        CAMPAIGN_START,
        STORY,
        CAMPAIGN_END,
        CAMPAIGN_FAIL
    };

    enum EVENT_SOURCE {
        FORCOM,
        TACNET,
        INTEL,
        MAIL,
        NEWS
    };

    CombatEvent(Campaign* c, int type, int time, int team, int source, const char* rgn);

    int operator == (const CombatEvent& u)  const { return this == &u; }

    // accessors/mutators:
    int                  Type()         const { return type;       }
    int                  Time()         const { return time;       }
    int                  GetIFF()       const { return team;       }
    int                  Points()       const { return points;     }
    int                  Source()       const { return source;     }
    Point                Location()     const { return loc;        }
    const char*          Region()       const { return region;     }
    const char*          Title()        const { return title;      }
    const char*          Information()  const { return info;       }
    const char*          Filename()     const { return file;       }
    const char*          ImageFile()    const { return image_file; }
    const char*          SceneFile()    const { return scene_file; }
    Bitmap&              Image()              { return image;      }
    const char*          SourceName()   const;
    const char*          TypeName()     const;
    bool                 Visited()      const { return visited;    }

    void                 SetType(int t)       { type = t;          }
    void                 SetTime(int t)       { time = t;          }
    void                 SetIFF(int t)        { team = t;          }
    void                 SetPoints(int p)     { points = p;        }
    void                 SetSource(int s)     { source = s;        }
    void                 SetLocation(const Point& p) { loc = p;    }
    void                 SetRegion(Text rgn)  { region = rgn;      }
    void                 SetTitle(Text t)     { title = t;         }
    void                 SetInformation(Text t) { info = t;        }
    void                 SetFilename(Text f)  { file = f;          }
    void                 SetImageFile(Text f) { image_file = f;    }
    void                 SetSceneFile(Text f) { scene_file = f;    }
    void                 SetVisited(bool v)   { visited = v;       }

    // operations:
    void                 Load();

    // utilities:
    static int           TypeFromName(const char* n);
    static int           SourceFromName(const char* n);
    static const char*   TypeName(int n);
    static const char*   SourceName(int n);

private:
    Campaign*            campaign;
    int                  type;
    int                  time;
    int                  team;
    int                  points;
    int                  source;
    bool                 visited;
    Point                loc;
    Text                 region;
    Text                 title;
    Text                 info;
    Text                 file;
    Text                 image_file;
    Text                 scene_file;
    Bitmap               image;
};

#endif CombatEvent_h


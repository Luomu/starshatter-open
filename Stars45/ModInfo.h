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
    FILE:         ModInfo.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Information block for describing and deploying third party mods
*/


#ifndef ModInfo_h
#define ModInfo_h

#include "Types.h"
#include "Bitmap.h"
#include "Text.h"
#include "List.h"

// +-------------------------------------------------------------------+

class ModInfo;
class ModCampaign;
class ModCatalog;

// +-------------------------------------------------------------------+

class ModInfo
{
public:
    static const char* TYPENAME() { return "ModInfo"; }

    ModInfo();
    ModInfo(const char* filename);
    ModInfo(const char* name, const char* version, const char* url);
    ~ModInfo();

    int      operator == (const ModInfo& m)   const { return name.length() && name == m.name; }

    const Text&    Name()         const { return name;       }
    const Text&    Description()  const { return desc;       }
    const Text&    Author()       const { return author;     }
    const Text&    URL()          const { return url;        }
    const Text&    Filename()     const { return filename;   }
    const Text&    Copyright()    const { return copyright;  }
    Bitmap*        LogoImage()    const { return logo;       }
    const Text&    Version()      const { return version;    }
    bool           Distribute()   const { return distribute; }
    bool           IsEnabled()    const { return enabled;    }

    List<ModCampaign>& GetCampaigns()   { return campaigns;  }

    bool           Load(const char* filename);
    bool           ParseModInfo(const char* buffer);

    bool           Enable();
    bool           Disable();

private:
    Text           name;
    Text           desc;
    Text           author;
    Text           url;
    Text           filename;
    Text           copyright;
    Bitmap*        logo;
    Text           logoname;
    Text           version;
    bool           distribute;
    bool           enabled;

    List<ModCampaign> campaigns;
    ModCatalog*       catalog;
};

// +-------------------------------------------------------------------+

class ModCampaign
{
    friend class ModInfo;

public:
    static const char* TYPENAME() { return "ModCampaign"; }

    ModCampaign()  : dynamic(false) { }
    ~ModCampaign() { }

    const Text&    Name()         const { return name;       }
    const Text&    Path()         const { return path;       }
    bool           IsDynamic()    const { return dynamic;    }

private:
    Text           name;
    Text           path;
    bool           dynamic;
};

// +-------------------------------------------------------------------+

class ModCatalog
{
    friend class ModInfo;

public:
    static const char* TYPENAME() { return "ModCatalog"; }

    ModCatalog()   { }
    ~ModCatalog()  { }

    const Text&    File()         const { return file;       }
    const Text&    Path()         const { return path;       }

private:
    Text           file;
    Text           path;
};

#endif ModInfo_h
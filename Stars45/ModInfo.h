/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

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
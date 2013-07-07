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
    FILE:         TerrainRegion.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Various heavenly bodies
*/

#ifndef TerrainRegion_h
#define TerrainRegion_h

#include "Types.h"
#include "StarSystem.h"
#include "Weather.h"

// +--------------------------------------------------------------------+

const double TERRAIN_ALTITUDE_LIMIT = 35e3;

class TerrainLayer;

// +--------------------------------------------------------------------+

class TerrainRegion : public OrbitalRegion
{
    friend class StarSystem;

public:
    TerrainRegion(StarSystem* sys, const char* n, double r, Orbital* prime=0);
    virtual ~TerrainRegion();

    // operations:
    virtual void      Update();

    // accessors:
    const Text&       PatchName()       const { return patch_name;    }
    const Text&       PatchTexture()    const { return patch_texture; }
    const Text&       ApronName()       const { return apron_name;    }
    const Text&       ApronTexture()    const { return apron_texture; }
    const Text&       WaterTexture()    const { return water_texture; }
    const Text&       DetailTexture0()  const { return noise_tex0;    }
    const Text&       DetailTexture1()  const { return noise_tex1;    }
    const Text&       HazeName()        const { return haze_name;     }
    const Text&       CloudsHigh()      const { return clouds_high;   }
    const Text&       ShadesHigh()      const { return shades_high;   }
    const Text&       CloudsLow()       const { return clouds_low;    }
    const Text&       ShadesLow()       const { return shades_low;    }
    const Text&       EnvironmentTexture(int n) const;

    Color             SunColor()        const { return sun_color[24]; }
    Color             SkyColor()        const { return sky_color[24]; }
    Color             FogColor()        const { return fog_color[24]; }
    Color             Ambient()         const { return ambient[24];   }
    Color             Overcast()        const { return overcast[24];  }
    Color             CloudColor()      const { return cloud_color[24];}
    Color             ShadeColor()      const { return shade_color[24];}

    double            LateralScale()    const { return scale;         }
    double            MountainScale()   const { return mtnscale;      }
    double            FogDensity()      const { return fog_density;   }
    double            FogScale()        const { return fog_scale;     }
    double            DayPhase()        const { return day_phase;     }
    double            HazeFade()        const { return haze_fade;     }
    double            CloudAltHigh()    const { return clouds_alt_high; }
    double            CloudAltLow()     const { return clouds_alt_low;  }
    Weather&          GetWeather()            { return weather;       }
    List<TerrainLayer>& GetLayers()           { return layers;        }

    bool              IsEclipsed()      const { return eclipsed;      }
    void              SetEclipsed(bool e)     { eclipsed = e;         }

    void              LoadSkyColors(const char* bmp_name);
    void              AddLayer(double h, const char* tile, const char* detail=0);

protected:
    Text              patch_name;
    Text              patch_texture;
    Text              apron_name;
    Text              apron_texture;
    Text              water_texture;
    Text              env_texture_positive_x;
    Text              env_texture_negative_x;
    Text              env_texture_positive_y;
    Text              env_texture_negative_y;
    Text              env_texture_positive_z;
    Text              env_texture_negative_z;
    Text              noise_tex0;
    Text              noise_tex1;
    Text              haze_name;
    Text              clouds_high;
    Text              clouds_low;
    Text              shades_high;
    Text              shades_low;

    Color             sun_color[25];
    Color             sky_color[25];
    Color             fog_color[25];
    Color             ambient[25];
    Color             overcast[25];
    Color             cloud_color[25];
    Color             shade_color[25];

    double            scale;
    double            mtnscale;

    double            fog_density;
    double            fog_scale;
    double            day_phase;
    double            haze_fade;
    double            clouds_alt_high;
    double            clouds_alt_low;

    Weather           weather;
    bool              eclipsed;

    List<TerrainLayer>   layers;
};

#endif TerrainRegion_h


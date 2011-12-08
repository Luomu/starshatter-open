/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2006. All Rights Reserved.

    SUBSYSTEM:    nGen.lib
    FILE:         Water.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Water surface effect w/ reflection and caustics
*/

#include "MemDebug.h"
#include "Water.h"
#include "Random.h"

// +--------------------------------------------------------------------+

struct WATER_REFRACT
{
    // Vrefract = (V + refract * N) * norm
    float refract;
    float refractNorm; 
    DWORD diffuse;
};

struct WATER_SURFACE
{
    float height;
    Vec3  normal;
};

// +--------------------------------------------------------------------+

#if defined(_X86) && !defined(_WIN64)
inline int f2i(float flt) 
{
	volatile int n; 

	__asm 
	{
		fld flt
		fistp n
	}

	return n;
}
#else
inline int f2i(float flt) 
{
	return (int) flt;
}
#endif


// +--------------------------------------------------------------------+

static WATER_REFRACT RefractionTable[512];
static bool refractInit = false;

static const int     WAVE_SIZE = 256;
static const DWORD   WAVE_MASK = 0xff;

// +--------------------------------------------------------------------+

Water::Water()
   :  size(0), depth(0), scaleTex(1), avgHeight(0),
      nVertices(0), surface(0), waves(0)
{
}

Water::~Water()
{
   delete [] surface;
   delete [] waves;
}

// +--------------------------------------------------------------------+

void
Water::Init(int n, float s, float d)
{
   size     = s;
   depth    = d;
   scaleTex = 1/size;

   // Calculate number of vertices
   nVertices = n;

   // Create refraction table
   if (!refractInit) {
      WATER_REFRACT* refract = &RefractionTable[256];

      for (UINT u = 0; u < 256; u++) {        
         float fCos0 = (float) u / (float) 256.0f;
         float f0    = acosf(fCos0);
         float fSin0 = sinf(f0);

         float fSin1 = fSin0 / 1.333f; // water
         float f1    = asinf(fSin1);
         float fCos1 = cosf(f1);

         refract[u].refract = fSin0 / fSin1 * fCos1 - fCos0;
         refract[u].refractNorm = - fSin1 / fSin0;
         refract[u].diffuse = ((((0xff - u)*(0xff - u)*(0xff - u)) << 8) & 0xff000000);

         RefractionTable[u] = RefractionTable[256];
      }

      refractInit = true;
   }

   // Create maps
   if (surface)
      delete [] surface;

   surface = new(__FILE__,__LINE__) WATER_SURFACE[n*n];
   ZeroMemory(surface, n*n * sizeof(WATER_SURFACE));

   if (waves)
      delete [] waves;

   waves = new(__FILE__,__LINE__) float[WAVE_SIZE*4];

   double f = 1.0 / (double) WAVE_SIZE;
   for (int i = 0; i < WAVE_SIZE; i++) {
      double s0 = sin(2*PI*i*f);
      double s1 = sin(4*PI*i*f);
      double s2 = sin(6*PI*i*f);
      double s3 = sin(8*PI*i*f);

      waves[0*WAVE_SIZE + i] = (float) (1.8 * s0*s0 - 0.9);
      waves[1*WAVE_SIZE + i] = (float) (1.6 * s1*s1 - 0.8);
      waves[2*WAVE_SIZE + i] = (float) (0.4 * s2);
      waves[3*WAVE_SIZE + i] = (float) (0.8 * s3*s3 - 0.4);
   }

   for (int i = 0; i < 4; i++) {
      offsets[i]   = (float) Random(0, WAVE_SIZE);
   }

   offsets[4] = 12.45f;
   offsets[5] = 14.23f;
   offsets[6] = 16.72f;
   offsets[7] = 20.31f;
}

// +--------------------------------------------------------------------+

void
Water::CalcWaves(double seconds)
{
   int  i, n[4];
   UINT SIZE = nVertices;
   UINT STEP = WAVE_SIZE / (SIZE-1);
   UINT STEP2 = STEP/2;
   UINT AREA = SIZE * SIZE;
   UINT x, y;

   for (i = 0; i < 4; i++) {
      n[i] = (int) offsets[i];
   }

   WATER_SURFACE* pSurf = surface;

   // compute heights
   for (y = 0; y < SIZE; y++) {
      for (x = 0; x < SIZE; x++) {
         float h = 0;
         h += waves[ ((n[0] + x*STEP
                            - y*STEP2) & WAVE_MASK) + 0*WAVE_SIZE ];
         h += waves[ ((n[1] + x*STEP2
                            + y*STEP)  & WAVE_MASK) + 1*WAVE_SIZE ];
         h += waves[ ((n[2] + x*STEP)  & WAVE_MASK) + 2*WAVE_SIZE ];
         h += waves[ ((n[3] + y*STEP)  & WAVE_MASK) + 3*WAVE_SIZE ];

         pSurf->height = h * depth;
         pSurf++;
      }
   }

   // compute normals
   UINT uXN, uX0, uXP;
   UINT uYN, uY0, uYP;

   uYP = AREA - SIZE;
   uY0 = 0;
   uYN = SIZE;

   for (y = 0; y < SIZE; y++) {
      uXP = SIZE - 1;
      uX0 = 0;
      uXN = 1;

      for (x = 0; x < SIZE; x++) {
         Vec3  vecN;
         float f;

         f = surface[uXN + uYN].height - surface[uXP + uYP].height; vecN.x = vecN.z = f;           
         f = surface[uX0 + uYN].height - surface[uX0 + uYP].height; vecN.z += f;
         f = surface[uXP + uYN].height - surface[uXN + uYP].height; vecN.x -= f; vecN.z += f;
         f = surface[uXN + uY0].height - surface[uXP + uY0].height; vecN.x += f;

         vecN.y = -15.0f * depth;
         vecN.Normalize();

         surface[uX0 + uY0].normal = vecN * -1.0f;

         uXP = uX0;
         uX0 = uXN;
         uXN = (uXN + 1) % SIZE;
      }

      uYP = uY0;
      uY0 = uYN;
      uYN = (uYN + SIZE) % AREA;
   }

   // update offsets
   for (i = 0; i < 4; i++) {
      offsets[i] += (float) (offsets[i+4] * seconds);

      if (offsets[i] > WAVE_SIZE)
         offsets[i] -= WAVE_SIZE;
   }

}

// +--------------------------------------------------------------------+

void
Water::UpdateSurface(Vec3& eyePos, VertexSet* vset)
{
   UINT SIZE = nVertices;
   UINT AREA = SIZE * SIZE;
   UINT x, y;
   
   WATER_SURFACE* pSurf = surface;
   Vec3*          pLoc  = vset->loc;
   Vec3*          pNorm = vset->nrm;
   DWORD*         pDiff = vset->diffuse;
   float*         pTu   = vset->tu;
   float*         pTv   = vset->tv;

   float          fInc  = 1.0f / (float) (SIZE-1);
   float          fx    = 0.0f;
   float          fz    = 0.0f;

   for (y = 0; y < SIZE; y++) {
      for (x = 0; x < SIZE; x++) {
         // update vertex height and normal
         pLoc->y += pSurf->height;
         *pNorm  =  pSurf->normal;

         /*
         // Update texture coords and diffuse based upon refraction
         Vec3 vec = eyePos - *pLoc;
         vec.Normalize();

         WATER_REFRACT *pRefract;
         pRefract = RefractionTable + 256 + f2i(vec.dot(*pNorm) * 255.0f);

         *pDiff = pRefract->diffuse;

         // compute apparent displacement
         Vec3 vecD = (pSurf->normal * pRefract->refract + vec) * pRefract->refractNorm;
         Vec3 vecP = *pLoc;
         vecP.y -= depth;

         // perturb texture coords
         float fB = vecD * vecP * 2.0f;
         float fD = fB * fB - depth;
         float fScale = (-fB + sqrtf(fD)) * 0.5f;

         *pTu = vecD.x * fScale + fx;
         *pTv = vecD.z * fScale + fz;
         */

         fx += fInc;
         pSurf++;
         pLoc++;
         pNorm++;
         pDiff++;
         pTu++;
         pTv++;
      }

      fx =  0.0f;
      fz += fInc;
   }
}



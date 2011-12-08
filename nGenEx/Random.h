/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib
    FILE:         Random.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Utility functions for generating random numbers and locations.
*/

#ifndef Random_h
#define Random_h

#include "Types.h"
#include "Geometry.h"

// +----------------------------------------------------------------------+

void     RandomInit();
Point    RandomDirection();
Point    RandomPoint();
Vec3     RandomVector(double radius);
double   Random(double min=0, double max=1);
int      RandomIndex();
bool     RandomChance(int wins=1, int tries=2);
int      RandomSequence(int current, int range);
int      RandomShuffle(int count);

// +----------------------------------------------------------------------+

#endif Random_h

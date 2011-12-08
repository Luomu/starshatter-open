/*  Project nGen
    John DiCamillo
    Copyright © 1997-2002. All Rights Reserved.

    SUBSYSTEM:    nGen.lib
    FILE:         TimeSnap.h
    AUTHOR:       John DiCamillo

*/

#ifndef TimeSnap_h
#define TimeSnap_h

// +--------------------------------------------------------------------+

#define TIMESNAP(clock) _asm push eax\
                        _asm push edx\
                        _asm _emit 0x0F\
                        _asm _emit 0x31\
                        _asm mov clock, eax\
                        _asm pop edx\
                        _asm pop eax

// +--------------------------------------------------------------------+

#endif TimeSnap_h
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
    FILE:         NetData.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Payload structures for multiplayer network packets
*/

#include "MemDebug.h"
#include "NetData.h"
#include "NetLink.h"
#include "NetMsg.h"
#include "RadioMessage.h"
#include "Ship.h"
#include "Shot.h"
#include "Sim.h"
#include "Instruction.h"
#include "Weapon.h"
#include "Element.h"
#include "System.h"
#include "Power.h"
#include "Shield.h"

#include "Game.h"

// +--------------------------------------------------------------------+
// DATA     SIZE              OFFSET
// -------- ----------------- ---------
// type:    1                  0
// size:    1                  1
// objid:   2                  2
// loc:     9 (3 x 24 bits)    4
// vel:     6 (3 x 16 bits)   13
// euler:   4 (3 x 10 bits)   19
// status:  1                 23

const int      LOCATION_OFFSET = 8000000;
const int      VELOCITY_OFFSET =   32000;
const double   EULER_SCALE     = 2*PI / (1<<10);

BYTE*
NetObjLoc::Pack()
{
    data[ 0] = TYPE;
    data[ 1] = SIZE;

    // obj id
    data[ 2] = (BYTE) ((objid & 0xff00) >> 8);
    data[ 3] = (BYTE) ((objid & 0x00ff)     );

    // location
    DWORD x = (DWORD) (((int) location.x + LOCATION_OFFSET) & 0x00ffffff);
    DWORD y = (DWORD) (((int) location.y + LOCATION_OFFSET) & 0x00ffffff);
    DWORD z = (DWORD) (((int) location.z + LOCATION_OFFSET) & 0x00ffffff);

    data[ 4] = (BYTE) ((x & 0x00ff0000) >> 16);
    data[ 5] = (BYTE) ((x & 0x0000ff00) >>  8);
    data[ 6] = (BYTE) ((x & 0x000000ff)      );

    data[ 7] = (BYTE) ((y & 0x00ff0000) >> 16);
    data[ 8] = (BYTE) ((y & 0x0000ff00) >>  8);
    data[ 9] = (BYTE) ((y & 0x000000ff)      );

    data[10] = (BYTE) ((z & 0x00ff0000) >> 16);
    data[11] = (BYTE) ((z & 0x0000ff00) >>  8);
    data[12] = (BYTE) ((z & 0x000000ff)      );

    // velocity
    WORD vx = (WORD) (((int) velocity.x + VELOCITY_OFFSET) & 0x0000ffff);
    WORD vy = (WORD) (((int) velocity.y + VELOCITY_OFFSET) & 0x0000ffff);
    WORD vz = (WORD) (((int) velocity.z + VELOCITY_OFFSET) & 0x0000ffff);

    data[13] = (BYTE) ((vx & 0xff00) >>  8);
    data[14] = (BYTE) ((vx & 0x00ff));

    data[15] = (BYTE) ((vy & 0xff00) >>  8);
    data[16] = (BYTE) ((vy & 0x00ff));

    data[17] = (BYTE) ((vz & 0xff00) >>  8);
    data[18] = (BYTE) ((vz & 0x00ff));

    // orientation
    if (_finite(euler.x)) {
        while (euler.x < 0)     euler.x += 2*PI;
        while (euler.x > 2*PI)  euler.x -= 2*PI;
    }
    else {
        euler.x = 0;
    }

    if (_finite(euler.y)) {
        while (euler.y < 0)     euler.y += 2*PI;
        while (euler.y > 2*PI)  euler.y -= 2*PI;
    }
    else {
        euler.y = 0;
    }

    if (_finite(euler.z)) {
        while (euler.z < 0)     euler.z += 2*PI;
        while (euler.z > 2*PI)  euler.z -= 2*PI;
    }
    else {
        euler.z = 0;
    }

    WORD ox = (WORD) (((int) (euler.x / EULER_SCALE)) & 0x000003ff);
    WORD oy = (WORD) (((int) (euler.y / EULER_SCALE)) & 0x000003ff);
    WORD oz = (WORD) (((int) (euler.z / EULER_SCALE)) & 0x000003ff);

    DWORD o = (ox << 20) | (oy << 10) | (oz);

    data[19] = (BYTE) ((o & 0xff000000) >> 24);
    data[20] = (BYTE) ((o & 0x00ff0000) >> 16);
    data[21] = (BYTE) ((o & 0x0000ff00) >>  8);
    data[22] = (BYTE) ((o & 0x000000ff)      );

    // status bits
    data[23] = throttle  << 7 |
    augmenter << 6 |
    gear      << 5 |
    (shield   >> 2) & 0x1f;

    return data;
}

bool
NetObjLoc::Unpack(const BYTE* p)
{
    if (p && p[0] == TYPE && p[1] == SIZE) {
        CopyMemory(data, p, SIZE);

        objid    = (data[ 2] <<  8) | 
        (data[ 3]      );

        int   x  = (data[ 4] << 16) |
        (data[ 5] <<  8) |
        (data[ 6]      );

        int   y  = (data[ 7] << 16) |
        (data[ 8] <<  8) |
        (data[ 9]      );

        int   z  = (data[10] << 16) |
        (data[11] <<  8) |
        (data[12]      );

        int   vx = (data[13] <<  8) |
        (data[14]      );

        int   vy = (data[15] <<  8) |
        (data[16]      );

        int   vz = (data[17] <<  8) |
        (data[18]      );

        DWORD o  = (data[19] << 24) |
        (data[20] << 16) |
        (data[21] <<  8) |
        (data[22]      );

        WORD ox = (WORD) ((o >> 20) & 0x03ff);
        WORD oy = (WORD) ((o >> 10) & 0x03ff);
        WORD oz = (WORD) ((o      ) & 0x03ff);

        throttle    =  data[23] & 0x80 ? true : false;
        augmenter   =  data[23] & 0x40 ? true : false;
        gear        =  data[23] & 0x20 ? true : false;
        shield      = (data[23] & 0x1f) << 2;

        location = Point(x -LOCATION_OFFSET, y -LOCATION_OFFSET, z -LOCATION_OFFSET);
        velocity = Point(vx-VELOCITY_OFFSET, vy-VELOCITY_OFFSET, vz-VELOCITY_OFFSET);
        euler    = Point(ox*EULER_SCALE,     oy*EULER_SCALE,     oz*EULER_SCALE);
        return true;
    }

    return false;
}

// +--------------------------------------------------------------------+

BYTE*
NetJoinRequest::Pack()
{
    ZeroMemory(data, SIZE);

    data[0] = TYPE;
    data[1] = SIZE;

    for (int i = 0; i < name.length() && i < 16; i++)
    data[2+i] = name[i];

    for (int i = 0; i < pass.length() && i < 16; i++)
    data[18+i] = pass[i];

    for (int i = 0; i < elem.length() && i < 31; i++)
    data[34+i] = elem[i];

    data[65] = (BYTE) index;

    for (int i = 0; i < serno.length() && i < 60; i++)
    data[66+i] = serno[i];

    return data;
}

bool
NetJoinRequest::Unpack(const BYTE* p)
{
    if (p && p[0] == TYPE && p[1] == SIZE) {
        CopyMemory(data, p, SIZE);

        char buf[64];

        CopyMemory(buf, data+2, 16);
        buf[16] = 0;
        name    = buf;

        CopyMemory(buf, data+18, 16);
        buf[16] = 0;
        pass   = buf;

        CopyMemory(buf, data+34, 31);
        buf[31] = 0;
        elem    = buf;

        index   = data[65];

        CopyMemory(buf, data+66, 60);
        buf[61] = 0;
        serno   = buf;
        return true;
    }

    return false;
}

// +--------------------------------------------------------------------+

NetJoinAnnounce::NetJoinAnnounce()
: index(0), integrity(0), respawns(0), decoys(0), probes(0), fuel(0),
shield(0), nid(0)
{
    ZeroMemory(ammo, sizeof(ammo));
}

void
NetJoinAnnounce::SetAmmo(const int* a)
{
    if (a) {
        CopyMemory(ammo, a, sizeof(ammo));
    }
}

void
NetJoinAnnounce::SetShip(Ship* s)
{
    SetName(s->Name());
    SetObjID(s->GetObjID());

    if (s->GetElement()) {
        SetElement(s->GetElement()->Name());
        SetIndex(s->GetElementIndex());
    }

    if (s->GetRegion())
    SetRegion(s->GetRegion()->Name());

    SetLocation(s->Location());
    SetVelocity(s->Velocity());
    SetIntegrity(s->Integrity());
    SetRespawns(s->RespawnCount());

    if (s->GetDecoy())
    SetDecoys(s->GetDecoy()->Ammo());

    if (s->GetProbeLauncher())
    SetProbes(s->GetProbeLauncher()->Ammo());

    if (s->Reactors().size())
    SetFuel(s->Reactors()[0]->Charge());

    Shield* shield = s->GetShield();
    if (shield)
    SetShield((int) shield->GetPowerLevel());
}

BYTE*
NetJoinAnnounce::Pack()
{
    ZeroMemory(data, SIZE);

    data[ 0] = TYPE;
    data[ 1] = SIZE;
    data[ 2] = (BYTE) ((objid & 0xff00) >> 8);
    data[ 3] = (BYTE) ((objid & 0x00ff)     );

    float* f = (float*) (data + 4);
    *f++     = (float)  loc.x;       // bytes  4 -  7
    *f++     = (float)  loc.y;       // bytes  8 - 11
    *f++     = (float)  loc.z;       // bytes 12 - 15
    *f++     = (float)  integrity;   // bytes 16 - 19

    for (int i = 0; i < name.length() && i < 16; i++)
    data[20+i] = name[i];

    for (int i = 0; i < elem.length() && i < 32; i++)
    data[36+i] = elem[i];

    for (int i = 0; i < region.length() && i < 32; i++)
    data[68+i] = region[i];

    int*   p = (int*)   (data + 100);
    *p++     = respawns;             // bytes 100 - 103
    *p++     = decoys;               // bytes 104 - 107
    *p++     = probes;               // bytes 108 - 111

    data[112]= (BYTE) fuel;          // byte  112
    data[113]= (BYTE) shield;        // byte  113

    BYTE* a  = data + 116;
    for (int i = 0; i < 16; i++) {       // bytes 116 - 179
        if (ammo[i] >= 0) {
            *a++ = ammo[i];
        }
        else {
            *a++ = 255;
        }
    }

    data[180] = (BYTE) index;

    f     = (float*) (data + 184);
    *f++  = (float)  velocity.x;
    *f++  = (float)  velocity.y;
    *f++  = (float)  velocity.z;

    return data;
}

bool
NetJoinAnnounce::Unpack(const BYTE* p)
{
    if (p && p[0] == TYPE && p[1] == SIZE) {
        CopyMemory(data, p, SIZE);

        objid = (data[2] << 8) | data[3];

        float* f = (float*) (data + 4);
        loc.x       = *f++;
        loc.y       = *f++;
        loc.z       = *f++;
        integrity   = *f++;

        char buf[64];
        CopyMemory(buf, data+20, 16);
        buf[16] = 0;
        name = buf;

        CopyMemory(buf, data+36, 32);
        buf[16] = 0;
        elem = buf;

        CopyMemory(buf, data+68, 32);
        buf[16] = 0;
        region = buf;

        int*   p = (int*)   (data + 100);
        respawns =  *p++;
        decoys   =  *p++;
        probes   =  *p++;

        fuel     =  data[112];
        shield   =  data[113];

        CopyMemory(ammo, data+116, 16);

        index = data[180];

        f = (float*) (data + 184);
        velocity.x  = *f++;
        velocity.y  = *f++;
        velocity.z  = *f++;

        return true;
    }

    return false;
}

// +--------------------------------------------------------------------+

BYTE*
NetQuitAnnounce::Pack()
{
    ZeroMemory(data, SIZE);

    data[0] = TYPE;
    data[1] = SIZE;

    data[2] = (BYTE) ((objid & 0xff00) >> 8);
    data[3] = (BYTE) ((objid & 0x00ff)     );
    data[4] = (BYTE) (disconnected);

    return data;
}

bool
NetQuitAnnounce::Unpack(const BYTE* p)
{
    if (p && p[0] == TYPE && p[1] == SIZE) {
        CopyMemory(data, p, SIZE);

        objid        = (data[2] << 8) | data[3];
        disconnected =  data[4] ? true : false;

        return true;
    }

    return false;
}

// +--------------------------------------------------------------------+

BYTE*
NetDisconnect::Pack()
{
    ZeroMemory(data, SIZE);

    data[0] = TYPE;
    data[1] = SIZE;

    return data;
}

bool
NetDisconnect::Unpack(const BYTE* p)
{
    if (p && p[0] == TYPE && p[1] == SIZE) {
        CopyMemory(data, p, SIZE);
        return true;
    }

    return false;
}

// +--------------------------------------------------------------------+

BYTE*
NetObjDamage::Pack()
{
    ZeroMemory(data, SIZE);

    data[0] = TYPE;
    data[1] = SIZE;

    data[2] = (BYTE) ((objid  & 0xff00) >> 8);
    data[3] = (BYTE) ((objid  & 0x00ff)     );
    data[4] = (BYTE) ((shotid & 0xff00) >> 8);
    data[5] = (BYTE) ((shotid & 0x00ff)     );

    float* p = (float*) (data + 6);
    *p       = damage;

    return data;
}

bool
NetObjDamage::Unpack(const BYTE* p)
{
    if (p && p[0] == TYPE && p[1] == SIZE) {
        CopyMemory(data, p, SIZE);

        objid  = (data[2] << 8) | data[3];
        shotid = (data[4] << 8) | data[5];
        damage = *(float*) (data + 6);

        return true;
    }

    return false;
}

// +--------------------------------------------------------------------+

BYTE*
NetSysDamage::Pack()
{
    ZeroMemory(data, SIZE);

    data[0] = TYPE;
    data[1] = SIZE;

    data[2] = (BYTE) ((objid & 0xff00) >> 8);
    data[3] = (BYTE) ((objid & 0x00ff)     );

    float* p = (float*) (data + 4);
    *p       = (float) damage;

    data[8] = (BYTE) (sysix+1);
    data[9] = dmgtype;

    return data;
}

bool
NetSysDamage::Unpack(const BYTE* p)
{
    if (p && p[0] == TYPE && p[1] == SIZE) {
        CopyMemory(data, p, SIZE);

        objid   = (data[2] << 8) | data[3];
        damage  = *(float*) (data + 4);
        sysix   = data[8];
        dmgtype = data[9];

        sysix--;
        return true;
    }

    return false;
}

// +--------------------------------------------------------------------+

BYTE*
NetSysStatus::Pack()
{
    ZeroMemory(data, SIZE);

    data[0]  = TYPE;
    data[1]  = SIZE;

    data[2]  = (BYTE) ((objid & 0xff00) >> 8);
    data[3]  = (BYTE) ((objid & 0x00ff)     );
    data[4]  = (BYTE) (sysix+1);
    data[5]  = (BYTE) (status);
    data[6]  = (BYTE) (power);
    data[7]  = (BYTE) (reactor);

    float* f = (float*) (data + 8);
    *f       = (float) avail;

    return data;
}

bool
NetSysStatus::Unpack(const BYTE* p)
{
    if (p && p[0] == TYPE && p[1] == SIZE) {
        CopyMemory(data, p, SIZE);

        objid    = (data[2] << 8) | data[3];
        sysix    = data[4];
        status   = data[5];
        power    = data[6];
        reactor  = data[7];

        float* f = (float*) (data + 8);
        avail    = *f;

        sysix--;
        return true;
    }

    return false;
}

// +--------------------------------------------------------------------+

BYTE*
NetObjKill::Pack()
{
    ZeroMemory(data, SIZE);

    data[0] = TYPE;
    data[1] = SIZE;

    data[2] = (BYTE) ((objid   & 0xff00) >> 8);
    data[3] = (BYTE) ((objid   & 0x00ff)     );
    data[4] = (BYTE) ((kill_id & 0xff00) >> 8);
    data[5] = (BYTE) ((kill_id & 0x00ff)     );
    data[6] = (BYTE) killtype;
    data[7] = (BYTE) respawn;

    float* f = (float*) (data + 8);
    *f++     = (float)  loc.x;
    *f++     = (float)  loc.y;
    *f++     = (float)  loc.z;

    data[20] = (BYTE) deck;

    return data;
}

bool
NetObjKill::Unpack(const BYTE* p)
{
    if (p && p[0] == TYPE && p[1] == SIZE) {
        CopyMemory(data, p, SIZE);

        objid    = (data[2] << 8) | data[3];
        kill_id  = (data[4] << 8) | data[5];
        killtype = data[6];
        respawn  = data[7] ? true : false;

        float* f = (float*) (data + 8);
        loc.x    = *f++;
        loc.y    = *f++;
        loc.z    = *f++;

        deck     = data[20];

        return true;
    }

    return false;
}

// +--------------------------------------------------------------------+

BYTE*
NetObjHyper::Pack()
{
    ZeroMemory(data, SIZE);

    data[0] = TYPE;
    data[1] = SIZE;

    data[2] = (BYTE) ((objid   & 0xff00) >> 8);
    data[3] = (BYTE) ((objid   & 0x00ff)     );
    data[4] = (BYTE) ((fc_src  & 0xff00) >> 8);
    data[5] = (BYTE) ((fc_src  & 0x00ff)     );
    data[6] = (BYTE) ((fc_dst  & 0xff00) >> 8);
    data[7] = (BYTE) ((fc_dst  & 0x00ff)     );
    data[8] = (BYTE) transtype;

    float* f = (float*) (data + 12);
    *f++     = (float)  location.x;  // bytes 12 - 15
    *f++     = (float)  location.y;  // bytes 16 - 19
    *f++     = (float)  location.z;  // bytes 20 - 23

    char*  p = (char*)  (data + 24);
    strncpy(p, region.data(), 31);

    return data;
}

bool
NetObjHyper::Unpack(const BYTE* p)
{
    if (p && p[0] == TYPE && p[1] == SIZE) {
        CopyMemory(data, p, SIZE);

        objid       = (data[2] << 8) | data[3];
        fc_src      = (data[4] << 8) | data[5];
        fc_dst      = (data[6] << 8) | data[7];
        transtype   = data[8];

        float* f = (float*) (data + 12);
        location.x  = *f++;
        location.y  = *f++;
        location.z  = *f++;

        region      = (char*) (data + 24);

        return true;
    }

    return false;
}

// +--------------------------------------------------------------------+

BYTE*
NetObjTarget::Pack()
{
    ZeroMemory(data, SIZE);

    data[0] = TYPE;
    data[1] = SIZE;

    data[2] = (BYTE) ((objid & 0xff00) >> 8);
    data[3] = (BYTE) ((objid & 0x00ff)     );
    data[4] = (BYTE) ((tgtid & 0xff00) >> 8);
    data[5] = (BYTE) ((tgtid & 0x00ff)     );
    data[6] = (BYTE) (sysix+1);

    return data;
}

bool
NetObjTarget::Unpack(const BYTE* p)
{
    if (p && p[0] == TYPE && p[1] == SIZE) {
        CopyMemory(data, p, SIZE);

        objid = (data[2] << 8) | data[3];
        tgtid = (data[4] << 8) | data[5];
        sysix = data[6];

        sysix--;
        return true;
    }

    return false;
}

// +--------------------------------------------------------------------+

BYTE*
NetObjEmcon::Pack()
{
    ZeroMemory(data, SIZE);

    data[0] = TYPE;
    data[1] = SIZE;

    data[2] = (BYTE) ((objid & 0xff00) >> 8);
    data[3] = (BYTE) ((objid & 0x00ff)     );
    data[4] = (BYTE) (emcon);

    return data;
}

bool
NetObjEmcon::Unpack(const BYTE* p)
{
    if (p && p[0] == TYPE && p[1] == SIZE) {
        CopyMemory(data, p, SIZE);

        objid = (data[2] << 8) | data[3];
        emcon = data[4];

        return true;
    }

    return false;
}

// +--------------------------------------------------------------------+

BYTE*
NetWepTrigger::Pack()
{
    ZeroMemory(data, SIZE);

    data[0] = TYPE;
    data[1] = SIZE;

    data[2] = (BYTE) ((objid & 0xff00) >> 8);
    data[3] = (BYTE) ((objid & 0x00ff)     );
    data[4] = (BYTE) ((tgtid & 0xff00) >> 8);
    data[5] = (BYTE) ((tgtid & 0x00ff)     );
    data[6] = (BYTE) (sysix+1);
    data[7] = (BYTE) index;
    data[8] = (BYTE) count;
    data[9] = ((BYTE) decoy << 1) | 
    ((BYTE) probe     );

    return data;
}

bool
NetWepTrigger::Unpack(const BYTE* p)
{
    if (p && p[0] == TYPE && p[1] == SIZE) {
        CopyMemory(data, p, SIZE);

        objid = (data[2] << 8) | data[3];
        tgtid = (data[4] << 8) | data[5];
        sysix = data[6];
        index = data[7];
        count = data[8];
        decoy = (data[9] & 0x02) ? true : false;
        probe = (data[9] & 0x01) ? true : false;

        sysix--;
        return true;
    }

    return false;
}

// +--------------------------------------------------------------------+

BYTE*
NetWepRelease::Pack()
{
    ZeroMemory(data, SIZE);

    data[0]  = TYPE;
    data[1]  = SIZE;

    data[2]  = (BYTE) ((objid & 0xff00) >> 8);
    data[3]  = (BYTE) ((objid & 0x00ff)     );
    data[4]  = (BYTE) ((tgtid & 0xff00) >> 8);
    data[5]  = (BYTE) ((tgtid & 0x00ff)     );
    data[6]  = (BYTE) ((wepid & 0xff00) >> 8);
    data[7]  = (BYTE) ((wepid & 0x00ff)     );
    data[8]  = (BYTE) (sysix+1);
    data[9]  = (BYTE) index;
    data[10] = ((BYTE) decoy << 1) | 
    ((BYTE) probe     );

    return data;
}

bool
NetWepRelease::Unpack(const BYTE* p)
{
    if (p && p[0] == TYPE && p[1] == SIZE) {
        CopyMemory(data, p, SIZE);

        objid = (data[2] << 8) | data[3];
        tgtid = (data[4] << 8) | data[5];
        wepid = (data[6] << 8) | data[7];
        sysix = data[8];
        index = data[9];
        decoy = (data[10] & 0x02) ? true : false;
        probe = (data[10] & 0x01) ? true : false;

        sysix--;
        return true;
    }

    return false;
}

// +--------------------------------------------------------------------+

BYTE*
NetWepDestroy::Pack()
{
    ZeroMemory(data, SIZE);

    data[0] = TYPE;
    data[1] = SIZE;
    data[2] = (BYTE) ((objid & 0xff00) >> 8);
    data[3] = (BYTE) ((objid & 0x00ff)     );

    return data;
}

bool
NetWepDestroy::Unpack(const BYTE* p)
{
    if (p && p[0] == TYPE && p[1] == SIZE) {
        CopyMemory(data, p, SIZE);
        objid = (data[2] << 8) | data[3];
        return true;
    }

    return false;
}

// +--------------------------------------------------------------------+

NetCommMsg::~NetCommMsg()
{
    delete radio_message;
}

void
NetCommMsg::SetRadioMessage(RadioMessage* m)
{
    radio_message = new(__FILE__,__LINE__) RadioMessage(*m);
}

BYTE*
NetCommMsg::Pack()
{
    ZeroMemory(data, SIZE);

    if (radio_message) {
        length  = 55 + radio_message->Info().length();

        if (length > SIZE)
        length = SIZE;

        data[0] = TYPE;
        data[1] = (BYTE) length;

        if (radio_message->Sender()) {
            objid   = radio_message->Sender()->GetObjID();
            data[2] = (BYTE) ((objid & 0xff00) >> 8);
            data[3] = (BYTE) ((objid & 0x00ff)     );
        }

        if (radio_message->DestinationShip()) {
            DWORD dstid   = radio_message->DestinationShip()->GetObjID();
            data[4] = (BYTE) ((dstid & 0xff00) >> 8);
            data[5] = (BYTE) ((dstid & 0x00ff)     );
        }

        data[6] = (BYTE) radio_message->Action();
        data[7] = (BYTE) radio_message->Channel();

        if (radio_message->TargetList().size() > 0) {
            SimObject* tgt   = radio_message->TargetList().at(0);
            DWORD      tgtid = tgt->GetObjID();
            data[8] = (BYTE) ((tgtid & 0xff00) >> 8);
            data[9] = (BYTE) ((tgtid & 0x00ff)     );
        }

        float* f = (float*) (data + 10);
        *f++     = (float)  radio_message->Location().x;   // bytes 10 - 13
        *f++     = (float)  radio_message->Location().y;   // bytes 14 - 17
        *f++     = (float)  radio_message->Location().z;   // bytes 18 - 21

        char*  p = (char*)  (data + 22);

        Element* dst_elem = radio_message->DestinationElem();
        if (dst_elem)
        strncpy(p, dst_elem->Name().data(), 31);

        p = (char*) (data + 55);
        strncpy(p, radio_message->Info().data(), 128);

        data[SIZE-1] = 0;
    }

    return data;
}

bool
NetCommMsg::Unpack(const BYTE* p)
{
    if (p && p[0] == TYPE) {
        length = p[1];
        ZeroMemory(data, SIZE);
        CopyMemory(data, p, length);

        DWORD dstid    = 0;
        DWORD tgtid    = 0;
        int   action   = 0;
        int   channel  = 0;
        Point loc;
        Text  elem_name;
        Text  info;

        objid    = (data[2] << 8) | data[3];
        dstid    = (data[4] << 8) | data[5];
        tgtid    = (data[8] << 8) | data[9];
        action   = data[6];
        channel  = data[7];

        float* f = (float*) (data + 10);
        loc.x    = *f++;
        loc.y    = *f++;
        loc.z    = *f++;

        elem_name = (char*) (data + 22);

        if (length > 55)
        info = (char*) (data + 55);

        Sim*     sim = Sim::GetSim();
        Ship*    src = sim->FindShipByObjID(objid);
        Ship*    dst = sim->FindShipByObjID(dstid);
        Element* elem = sim->FindElement(elem_name);

        delete radio_message;
        if (elem)
        radio_message = new(__FILE__,__LINE__) RadioMessage(elem, src, action);
        else
        radio_message = new(__FILE__,__LINE__) RadioMessage(dst, src, action);

        radio_message->SetChannel(channel);
        radio_message->SetLocation(loc);
        radio_message->SetInfo(info);

        if (tgtid) {
            SimObject* tgt = sim->FindShipByObjID(tgtid);

            if (!tgt)
            tgt = sim->FindShotByObjID(tgtid);

            if (tgt)
            radio_message->AddTarget(tgt);
        }

        return true;
    }

    return false;
}

// +--------------------------------------------------------------------+

BYTE*
NetChatMsg::Pack()
{
    ZeroMemory(data, SIZE);

    int chatlen = text.length();

    if (chatlen > MAX_CHAT)
    chatlen = MAX_CHAT;

    length  = HDR_LEN + NAME_LEN + chatlen;

    data[0] = TYPE;
    data[1] = (BYTE) length;
    data[2] = (BYTE) ((dstid & 0xff00) >> 8);
    data[3] = (BYTE) ((dstid & 0x00ff)     );

    char*  p = (char*)  (data + HDR_LEN);
    strncpy(p, name.data(), NAME_LEN);

    p = (char*)  (data + HDR_LEN + NAME_LEN);
    strncpy(p, text.data(), chatlen);

    return data;
}

bool
NetChatMsg::Unpack(const BYTE* p)
{
    if (p && p[0] == TYPE) {
        length = p[1];
        ZeroMemory(data, SIZE);
        CopyMemory(data, p, length);

        dstid    = (data[2] << 8) | data[3];

        char buffer[NAME_LEN+1];
        ZeroMemory(buffer, NAME_LEN+1);
        CopyMemory(buffer, data + HDR_LEN, NAME_LEN);

        name = buffer;

        if (length > HDR_LEN + NAME_LEN)
        text = (char*) (data + HDR_LEN + NAME_LEN);

        return true;
    }

    return false;
}

// +--------------------------------------------------------------------+

NetElemRequest::NetElemRequest()
{ }

BYTE*
NetElemRequest::Pack()
{
    ZeroMemory(data, SIZE);

    data[0] = TYPE;
    data[1] = SIZE;

    strncpy((char*) (data +   8), name.data(),      NAME_LEN-1);

    return data;
}

bool
NetElemRequest::Unpack(const BYTE* p)
{
    if (p && p[0] == TYPE) {
        ZeroMemory(data, SIZE);
        CopyMemory(data, p, SIZE);

        name  = (const char*) (data +   8);

        return true;
    }

    return false;
}

// +--------------------------------------------------------------------+

NetElemCreate::NetElemCreate()
: iff(0), type(0), intel(0), alert(false), in_flight(false)
{
    for (int i = 0; i < 16; i++)
    load[i] = -1;
}

void
NetElemCreate::SetLoadout(int* l)
{
    if (l) {
        CopyMemory(load, l, sizeof(load));
    }
    else {
        for (int i = 0; i < 16; i++)
        load[i] = -1;
    }
}

void
NetElemCreate::SetSlots(int* s)
{
    if (s) {
        CopyMemory(slots, s, sizeof(slots));
    }
    else {
        for (int i = 0; i < 4; i++)
        slots[i] = -1;
    }
}

BYTE*
NetElemCreate::Pack()
{
    ZeroMemory(data, SIZE);

    data[0] = TYPE;
    data[1] = SIZE;

    data[2] = (BYTE) iff;
    data[3] = (BYTE) type;
    data[4] = (BYTE) intel;
    data[5] = (BYTE) obj_code;

    for (int i = 0; i < 16; i++)
    data[6+i] = (BYTE) load[i];

    strncpy((char*) (data +  22), name.data(),      NAME_LEN-1);
    strncpy((char*) (data +  54), commander.data(), NAME_LEN-1);
    strncpy((char*) (data +  86), objective.data(), NAME_LEN-1);
    strncpy((char*) (data + 118), carrier.data(),   NAME_LEN-1);

    data[150] = (BYTE) squadron;
    data[151] = (BYTE) slots[0];
    data[152] = (BYTE) slots[1];
    data[153] = (BYTE) slots[2];
    data[154] = (BYTE) slots[3];
    data[155] = (BYTE) alert;
    data[156] = (BYTE) in_flight;

    return data;
}

bool
NetElemCreate::Unpack(const BYTE* p)
{
    if (p && p[0] == TYPE) {
        ZeroMemory(data, SIZE);
        CopyMemory(data, p, SIZE);

        iff      = data[2];
        type     = data[3];
        intel    = data[4];
        obj_code = data[5];

        for (int i = 0; i < 16; i++) {
            load[i] = data[6+i] == 255 ? -1 : data[6+i];
        }

        name        = (const char*) (data +  22);
        commander   = (const char*) (data +  54);
        objective   = (const char*) (data +  86);
        carrier     = (const char*) (data + 118);

        squadron    = data[150];

        for (int i = 0; i < 4; i++) {
            slots[i] = data[151+i];
            if (slots[i] >= 255)
            slots[i] = -1;
        }

        alert       = data[155] ? true : false;
        in_flight   = data[156] ? true : false;

        return true;
    }

    return false;
}

// +--------------------------------------------------------------------+

BYTE*
NetShipLaunch::Pack()
{
    ZeroMemory(data, SIZE);

    data[ 0] = TYPE;
    data[ 1] = SIZE;

    DWORD* p = (DWORD*) (data + 4);
    *p++ = (DWORD) objid;
    *p++ = (DWORD) squadron;
    *p++ = (DWORD) slot;

    return data;
}

bool
NetShipLaunch::Unpack(const BYTE* p)
{
    if (p && p[0] == TYPE) {
        ZeroMemory(data, SIZE);
        CopyMemory(data, p, SIZE);

        DWORD* p = (DWORD*) (data + 4);
        objid    = *p++;
        squadron = (int) *p++;
        slot     = (int) *p++;

        return true;
    }

    return false;
}

// +--------------------------------------------------------------------+

NetNavData::NetNavData()
: objid(0), create(true), index(0), navpoint(0)
{
}

NetNavData::~NetNavData()
{
    delete navpoint;
}

void
NetNavData::SetNavPoint(Instruction* n)
{
    if (navpoint) {
        delete navpoint;
        navpoint = 0;
    }

    if (n)
    navpoint = new(__FILE__,__LINE__) Instruction(*n);
}

BYTE*
NetNavData::Pack()
{
    ZeroMemory(data, SIZE);

    data[ 0] = TYPE;
    data[ 1] = SIZE;

    data[ 2] = (BYTE) ((objid & 0xff00) >> 8);
    data[ 3] = (BYTE) ((objid & 0x00ff)     );
    data[ 4] = (BYTE) create;
    data[ 5] = (BYTE) index;

    if (!navpoint)
    return data;

    data[ 6] = (BYTE) navpoint->Action();
    data[ 7] = (BYTE) navpoint->Formation();
    data[ 8] = (BYTE) navpoint->Status();
    data[ 9] = (BYTE) navpoint->EMCON();
    data[10] = (BYTE) navpoint->WeaponsFree();
    data[11] = (BYTE) navpoint->Farcast();

    Point loc = navpoint->Location();

    float* f = (float*) (data + 12);
    *f++     = (float)  loc.x;                // bytes 12 - 15
    *f++     = (float)  loc.y;                // bytes 16 - 19
    *f++     = (float)  loc.z;                // bytes 20 - 23
    *f++     = (float)  navpoint->HoldTime(); // bytes 24 - 27
    *f++     = (float)  navpoint->Speed();    // bytes 28 - 31

    WORD tgtid = 0;
    if (navpoint->GetTarget())
    tgtid = (WORD) navpoint->GetTarget()->GetObjID();

    data[32] = (BYTE) ((tgtid & 0xff00) >> 8);
    data[33] = (BYTE) ((tgtid & 0x00ff)     );

    strncpy((char*) (data +  34), navpoint->RegionName(), NAME_LEN-1);
    strncpy((char*) (data +  66), navpoint->TargetName(), NAME_LEN-1);
    strncpy((char*) (data +  98), elem.data(),            NAME_LEN-1);

    return data;
}

bool
NetNavData::Unpack(const BYTE* p)
{
    if (p && p[0] == TYPE) {
        ZeroMemory(data, SIZE);
        CopyMemory(data, p, SIZE);

        int   action;
        int   formation;
        int   status;
        int   emcon;
        int   wep_free;
        int   farcast;
        int   speed;
        float hold_time;
        Point loc;
        WORD  tgtid = 0;

        const char* rgn_name = 0;
        const char* tgt_name = 0;

        objid    = (data[ 2] <<  8) | 
        (data[ 3]      );

        tgtid    = (data[32] <<  8) | 
        (data[33]      );

        create      = data[ 4] ? true : false;
        index       = data[ 5];
        action      = data[ 6];
        formation   = data[ 7];
        status      = data[ 8];
        emcon       = data[ 9];
        wep_free    = data[10];
        farcast     = data[11];

        float* f    = (float*) (data + 12);
        loc.x       = *f++;
        loc.y       = *f++;
        loc.z       = *f++;
        hold_time   = *f++;
        speed       = (int) *f++;

        rgn_name    = (const char*) (data + 34);
        tgt_name    = (const char*) (data + 66);
        elem        = (const char*) (data + 98);

        if (navpoint) {
            delete navpoint;
            navpoint = 0;
        }

        Sim*        sim = Sim::GetSim();
        SimRegion*  rgn = 0;

        if (sim)
        rgn = sim->FindRegion(rgn_name);

        if (rgn)
        navpoint = new(__FILE__,__LINE__) Instruction(rgn,      loc, action);
        else
        navpoint = new(__FILE__,__LINE__) Instruction(rgn_name, loc, action);

        navpoint->SetFormation(formation);
        navpoint->SetStatus(status);
        navpoint->SetEMCON(emcon);
        navpoint->SetWeaponsFree(wep_free);
        navpoint->SetFarcast(farcast);
        navpoint->SetHoldTime(hold_time);
        navpoint->SetSpeed(speed);
        navpoint->SetTarget(tgt_name);

        if (tgtid) {
            Sim*  sim = Sim::GetSim();
            Ship* tgt = sim->FindShipByObjID(tgtid);

            if (tgt)
            navpoint->SetTarget(tgt);
        }

        if (index >= 255)
        index = -1;

        return true;
    }

    return false;
}

// +--------------------------------------------------------------------+

BYTE*
NetNavDelete::Pack()
{
    ZeroMemory(data, SIZE);

    data[ 0] = TYPE;
    data[ 1] = SIZE;

    data[ 2] = (BYTE) ((objid & 0xff00) >> 8);
    data[ 3] = (BYTE) ((objid & 0x00ff)     );
    data[ 4] = (BYTE) index;

    strncpy((char*) (data + 6), elem.data(), 31);

    return data;
}

bool
NetNavDelete::Unpack(const BYTE* p)
{
    if (p && p[0] == TYPE) {
        ZeroMemory(data, SIZE);
        CopyMemory(data, p, SIZE);
        int index = 0;

        objid    = (data[ 2] <<  8) | 
        (data[ 3]      );

        index    = data[4];
        elem     = (const char*) (data + 6);

        if (index >= 255)
        index = -1;

        return true;
    }

    return false;
}

// +--------------------------------------------------------------------+

BYTE*
NetSelfDestruct::Pack()
{
    ZeroMemory(data, SIZE);

    data[0] = TYPE;
    data[1] = SIZE;

    data[2] = (BYTE) ((objid  & 0xff00) >> 8);
    data[3] = (BYTE) ((objid  & 0x00ff)     );

    float* p = (float*) (data + 4);
    *p       = damage;

    return data;
}

bool
NetSelfDestruct::Unpack(const BYTE* p)
{
    if (p && p[0] == TYPE && p[1] == SIZE) {
        CopyMemory(data, p, SIZE);

        objid  = (data[2] << 8) | data[3];
        damage = *(float*) (data + 4);

        return true;
    }

    return false;
}

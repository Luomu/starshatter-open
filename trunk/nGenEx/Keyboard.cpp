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

    SUBSYSTEM:    nGenEx.lib
    FILE:         Keyboard.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Keyboard Input class
*/

#include "MemDebug.h"
#include "Keyboard.h"
#include "Game.h"

// +--------------------------------------------------------------------+

static Keyboard* instance = 0;
int    Keyboard::map[KEY_MAP_SIZE];
int    Keyboard::alt[KEY_MAP_SIZE];

Keyboard::Keyboard()
: x(0), y(0), z(0), p(0), r(0), w(0), c(0), p1(0), r1(0), w1(0), t(0)
{
    instance    = this;
    sensitivity =  25;
    dead_zone   = 100;

    for (int i = 0; i < MotionController::MaxActions; i++)
    action[i] = 0;

    memset(map, 0, sizeof(map));
    memset(alt, 0, sizeof(alt));

    map[KEY_PLUS_X]      = 'R';
    map[KEY_MINUS_X]     = 'E';
    map[KEY_PLUS_Y]      = VK_HOME;
    map[KEY_MINUS_Y]     = VK_END;
    map[KEY_PLUS_Z]      = VK_PRIOR;    // page up
    map[KEY_MINUS_Z]     = VK_NEXT;     // page down

    map[KEY_PITCH_UP]    = VK_DOWN;
    map[KEY_PITCH_DOWN]  = VK_UP;
    map[KEY_YAW_LEFT]    = VK_LEFT;
    map[KEY_YAW_RIGHT]   = VK_RIGHT;
    map[KEY_ROLL_ENABLE] = 0;           // used to be VK_CONTROL;
}

Keyboard::~Keyboard()
{
    instance = 0;
}

Keyboard*
Keyboard::GetInstance()
{
    return instance;
}

// +--------------------------------------------------------------------+

void
Keyboard::MapKeys(KeyMapEntry* mapping, int nkeys)
{
    for (int i = 0; i < nkeys; i++) {
        KeyMapEntry k = mapping[i];

        if (k.act >= KEY_MAP_FIRST && k.act <= KEY_MAP_LAST) {
            if (k.key == 0 || k.key > VK_MBUTTON && k.key < KEY_JOY_1) {
                map[k.act] = k.key;
                alt[k.act] = k.alt;
            }
        }
    }
}

// +--------------------------------------------------------------------+

bool Keyboard::KeyDown(int key)
{
    if (key) {
        short k = GetAsyncKeyState(key);
        return (k<0)||(k&1);
    }

    return 0;
}

// +--------------------------------------------------------------------+

bool Keyboard::KeyDownMap(int key)
{
    if (key >= KEY_MAP_FIRST && key <= KEY_MAP_LAST && map[key]) {
        short k = GetAsyncKeyState(map[key]);
        short a = -1;

        if (alt[key] > 0 && alt[key] < KEY_JOY_1) {
            a = GetAsyncKeyState(alt[key]);
        }
        else {
            a = !GetAsyncKeyState(VK_SHIFT) &&
            !GetAsyncKeyState(VK_MENU);
        }

        return ((k<0)||(k&1)) && ((a<0)||(a&1));
    }

    return 0;
}

// +--------------------------------------------------------------------+

void
Keyboard::FlushKeys()
{
    for (int i = 0; i < 255; i++)
    GetAsyncKeyState(i);
}

// +--------------------------------------------------------------------+

static inline double sqr(double a) { return a; } //*a; }

void
Keyboard::Acquire()
{
    t = x = y = z = p = r = w = c = 0;

    for (int i = 0; i < MotionController::MaxActions; i++)
    action[i] = 0;

    int speed = 10;

    // lateral translations:
    if (KeyDownMap(KEY_PLUS_Y))            y =  1;
    else if (KeyDownMap(KEY_MINUS_Y))      y = -1;

    if (KeyDownMap(KEY_PLUS_Z))            z =  1;
    else if (KeyDownMap(KEY_MINUS_Z))      z = -1;

    if (KeyDownMap(KEY_MINUS_X))           x = -1;
    else if (KeyDownMap(KEY_PLUS_X))       x =  1;

    const double steps=10;

    // if roll and yaw are swapped --------------------------
    if (swapped) {
        // yaw:
        if (KeyDownMap(KEY_ROLL_LEFT))         { if (w1<steps) w1+=1; w = -sqr(w1/steps); }
        else if (KeyDownMap(KEY_ROLL_RIGHT))   { if (w1<steps) w1+=1; w =  sqr(w1/steps); }

        // another way to yaw:
        if (KeyDownMap(KEY_ROLL_ENABLE)) {
            if (KeyDownMap(KEY_YAW_LEFT))       { if (w1<steps) w1+=1; w = -sqr(w1/steps); }
            else if (KeyDownMap(KEY_YAW_RIGHT)) { if (w1<steps) w1+=1; w =  sqr(w1/steps); }
            else w1 = 0;
        }

        // roll:
        else {
            if (KeyDownMap(KEY_YAW_LEFT))       { if (r1<steps) r1+=1; r =  sqr(r1/steps); }
            else if (KeyDownMap(KEY_YAW_RIGHT)) { if (r1<steps) r1+=1; r = -sqr(r1/steps); }
            else r1 = 0;
        }
    }

    // else roll and yaw are NOT swapped ---------------------
    else {
        // roll:
        if (KeyDownMap(KEY_ROLL_LEFT))         { if (r1<steps) r1+=1; r =  sqr(r1/steps); }
        else if (KeyDownMap(KEY_ROLL_RIGHT))   { if (r1<steps) r1+=1; r = -sqr(r1/steps); }

        // another way to roll:
        if (KeyDownMap(KEY_ROLL_ENABLE)) {
            if (KeyDownMap(KEY_YAW_LEFT))       { if (r1<steps) r1+=1; r =  sqr(r1/steps); }
            else if (KeyDownMap(KEY_YAW_RIGHT)) { if (r1<steps) r1+=1; r = -sqr(r1/steps); }
            else r1 = 0;
        }

        // yaw left-right
        else {
            if (KeyDownMap(KEY_YAW_LEFT))       { if (w1<steps) w1+=1; w = -sqr(w1/steps); }
            else if (KeyDownMap(KEY_YAW_RIGHT)) { if (w1<steps) w1+=1; w =  sqr(w1/steps); }
            else w1 = 0;
        }
    }

    // if pitch is inverted ----------------------------------
    if (inverted) {
        if (KeyDownMap(KEY_PITCH_DOWN))        { if (p1<steps) p1+=1; p = -sqr(p1/steps); }
        else if (KeyDownMap(KEY_PITCH_UP))     { if (p1<steps) p1+=1; p =  sqr(p1/steps); }
        else p1 = 0;
    }

    // else pitch is NOT inverted ----------------------------
    else {
        if (KeyDownMap(KEY_PITCH_UP))          { if (p1<steps) p1+=1; p = -sqr(p1/steps); }
        else if (KeyDownMap(KEY_PITCH_DOWN))   { if (p1<steps) p1+=1; p =  sqr(p1/steps); }
        else p1 = 0;
    }

    if (KeyDownMap(KEY_CENTER))            c = 1;

    // actions
    if (KeyDownMap(KEY_ACTION_0))          action[0] = 1;
    if (KeyDownMap(KEY_ACTION_1))          action[1] = 1;
    if (KeyDownMap(KEY_ACTION_2))          action[2] = 1;
    if (KeyDownMap(KEY_ACTION_3))          action[3] = 1;
}

// +--------------------------------------------------------------------+




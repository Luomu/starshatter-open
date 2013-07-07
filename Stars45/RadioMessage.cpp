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
    FILE:         RadioMessage.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Radio communication message class implementation
*/

#include "MemDebug.h"
#include "RadioMessage.h"
#include "Ship.h"
#include "Text.h"

// +----------------------------------------------------------------------+

RadioMessage::RadioMessage(Ship*    dst, const Ship* s, int a)
: dst_ship(dst), dst_elem(0), sender(s), action(a), channel(0)
{
    if (s)
    channel = s->GetIFF();
}

RadioMessage::RadioMessage(Element* dst, const Ship* s, int a)
: dst_ship(0), dst_elem(dst), sender(s), action(a), channel(0)
{
    if (s)
    channel = s->GetIFF();
}

RadioMessage::RadioMessage(const RadioMessage& rm)
: dst_ship(rm.dst_ship), dst_elem(rm.dst_elem),
sender(rm.sender), action(rm.action), channel(rm.channel),
info(rm.info), location(rm.location)
{
    if (rm.target_list.size() > 0) {
        for (int i = 0; i < rm.target_list.size(); i++) {
            SimObject* obj = rm.target_list.at(i);
            target_list.append(obj);
        }
    }
}

RadioMessage::~RadioMessage()
{ }

// +----------------------------------------------------------------------+

const char*
RadioMessage::ActionName(int a)
{
    if (a == ACK) {
        int coin = rand();
        if (coin < 10000)       return "Acknowledged";
        if (coin < 17000)       return "Roger that";
        if (coin < 20000)       return "Understood";
        if (coin < 22000)       return "Copy that";
        return "Affirmative";
    }

    if (a == DISTRESS) {
        int coin = rand();
        if (coin < 15000)       return "Mayday! Mayday!";
        if (coin < 18000)       return "She's breaking up!";
        if (coin < 21000)       return "Checking out!";
        return "We're going down!";
    }

    if (a == WARN_ACCIDENT) {
        int coin = rand();
        if (coin < 15000)       return "Check your fire!";
        if (coin < 18000)       return "Watch it!";
        if (coin < 21000)       return "Hey! We're on your side!";
        return "Confirm your targets!";
    }

    if (a == WARN_TARGETED) {
        int coin = rand();
        if (coin < 15000)       return "Break off immediately!";
        if (coin < 20000)       return "Buddy spike!";
        return "Abort! Abort!";
    }

    switch (a) {
    case NONE:                 return "";

    case NACK:                 return "Negative, Unable";

    case ATTACK:               return "Engage";
    case ESCORT:               return "Escort";
    case BRACKET:              return "Bracket";
    case IDENTIFY:             return "Identify";

    case COVER_ME:             return "Cover Me";
    case MOVE_PATROL:          return "Vector";
    case SKIP_NAVPOINT:        return "Skip Navpoint";
    case RESUME_MISSION:       return "Resume Mission";
    case RTB:                  return "Return to Base";
    case DOCK_WITH:            return "Dock With";
    case QUANTUM_TO:           return "Jump to";
    case FARCAST_TO:           return "Farcast to";

    case GO_DIAMOND:           return "Goto Diamond Formation";
    case GO_SPREAD:            return "Goto Spread Formation";
    case GO_BOX:               return "Goto Box Formation";
    case GO_TRAIL:             return "Goto Trail Formation";

    case WEP_FREE:             return "Break and Attack";
    case WEP_HOLD:             return "Hold All Weapons";
    case FORM_UP:              return "Return to Formation";
    case SAY_POSITION:         return "Say Your Position";

    case LAUNCH_PROBE:         return "Launch Probe";
    case GO_EMCON1:            return "Goto EMCON 1";
    case GO_EMCON2:            return "Goto EMCON 2";
    case GO_EMCON3:            return "Goto EMCON 3";

    case REQUEST_PICTURE:      return "Request Picture";
    case REQUEST_SUPPORT:      return "Request Support";
    case PICTURE:              return "Picture is clear";

    case CALL_INBOUND:         return "Calling Inbound";
    case CALL_APPROACH:        return "Roger your approach";
    case CALL_CLEARANCE:       return "You have clearance";
    case CALL_FINALS:          return "On final approach";
    case CALL_WAVE_OFF:        return "Wave off - Runway is closed";

    case DECLARE_ROGUE:        return "Prepare to be destroyed!";

    case CALL_ENGAGING:        return "Engaging";
    case FOX_1:                return "Fox One!";
    case FOX_2:                return "Fox Two!";
    case FOX_3:                return "Fox Three!";
    case SPLASH_1:             return "Splash One!";
    case SPLASH_2:             return "Splash Two!";
    case SPLASH_3:             return "Splash Three!";
    case SPLASH_4:             return "Splash Four!";
    case SPLASH_5:             return "Target Destroyed!";
    case SPLASH_6:             return "Enemy Destroyed!";
    case SPLASH_7:             return "Confirmed Kill!";
    case BREAK_ORBIT:          return "Breaking Orbit";
    case MAKE_ORBIT:           return "Heading for Orbit";
    case QUANTUM_JUMP:         return "Going Quantum";

    default:                   return "Unknown";
    }
}

// +----------------------------------------------------------------------+

void
RadioMessage::AddTarget(SimObject* obj)
{
    if (obj && !target_list.contains(obj)) {
        target_list.append(obj);
    }
}




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
    FILE:         RadioTraffic.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    RadioTraffic maintains a history of all messages sent between ships
    in the simulation.  This class also handles displaying relevant
    traffic to the player.
*/

#ifndef RadioTraffic_h
#define RadioTraffic_h

#include "Types.h"
#include "Geometry.h"
#include "SimObject.h"
#include "List.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class Element;
class RadioMessage;
class Ship;
class SimObject;

// +--------------------------------------------------------------------+

class RadioTraffic
{
public:
    RadioTraffic();
    ~RadioTraffic();

    // accessors:
    static void          Initialize();
    static void          Close();

    static RadioTraffic* GetInstance()  { return radio_traffic; }

    static void          SendQuickMessage(Ship* ship, int msg);
    static void          Transmit(RadioMessage* msg);
    static void          DiscardMessages();
    static Text          TranslateVox(const char* phrase);

    void                 SendMessage(RadioMessage* msg);
    void                 DisplayMessage(RadioMessage* msg);


protected:
    List<RadioMessage>   traffic;

    static RadioTraffic* radio_traffic;
};

// +--------------------------------------------------------------------+

#endif RadioTraffic_h


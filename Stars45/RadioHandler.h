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
    FILE:         RadioHandler.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    RadioHandler (radio comms) class declaration
*/

#ifndef RadioHandler_h
#define RadioHandler_h

#include "Types.h"
#include "Geometry.h"
#include "SimObject.h"

// +--------------------------------------------------------------------+

class RadioMessage;
class Ship;

// +--------------------------------------------------------------------+

class RadioHandler
{
public:
    RadioHandler();
    virtual ~RadioHandler();

    virtual bool      ProcessMessage(RadioMessage* msg,       Ship*   s);
    virtual void      AcknowledgeMessage(RadioMessage* msg,   Ship*   s);

protected:
    virtual bool      IsOrder(int action);
    virtual bool      ProcessMessageOrders(RadioMessage* msg, Ship*   s);
    virtual bool      ProcessMessageAction(RadioMessage* msg, Ship*   s);

    virtual bool      Inbound(RadioMessage* msg, Ship* s);
    virtual bool      Picture(RadioMessage* msg, Ship* s);
    virtual bool      Support(RadioMessage* msg, Ship* s);
    virtual bool      SkipNavpoint(RadioMessage* msg, Ship* s);
    virtual bool      LaunchProbe(RadioMessage* msg, Ship* s);
};

// +--------------------------------------------------------------------+

#endif RadioHandler_h


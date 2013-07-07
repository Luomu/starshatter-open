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
    FILE:         EventTarget.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Event Target interface class
*/

#ifndef EventTarget_h
#define EventTarget_h

#include "Types.h"
#include "Geometry.h"

// +--------------------------------------------------------------------+

class EventTarget
{
public:
    static const char* TYPENAME() { return "EventTarget"; }

    virtual ~EventTarget() { }

    int operator == (const EventTarget& t) const { return this == &t; }

    virtual int  OnMouseMove(int x, int y)          { return 0; }
    virtual int  OnLButtonDown(int x, int y)        { return 0; }
    virtual int  OnLButtonUp(int x, int y)          { return 0; }
    virtual int  OnClick()                          { return 0; }
    virtual int  OnSelect()                         { return 0; }
    virtual int  OnRButtonDown(int x, int y)        { return 0; }
    virtual int  OnRButtonUp(int x, int y)          { return 0; }
    virtual int  OnMouseEnter(int x, int y)         { return 0; }
    virtual int  OnMouseExit(int x, int y)          { return 0; }
    virtual int  OnMouseWheel(int wheel)            { return 0; }

    virtual int  OnKeyDown(int vk, int flags)       { return 0; }

    virtual void SetFocus()                         { }
    virtual void KillFocus()                        { }
    virtual bool HasFocus() const                   { return false; }

    virtual bool IsEnabled() const                  { return true; }
    virtual bool IsVisible() const                  { return true; }
    virtual bool IsFormActive() const               { return true; }

    virtual Rect TargetRect() const                 { return Rect(); }

    virtual const char* GetDescription() const      { return "EventTarget"; }
};

#endif EventTarget_h


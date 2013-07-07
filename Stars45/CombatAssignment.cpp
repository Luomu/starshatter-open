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
    FILE:         CombatAssignment.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    High level assignment of one group to damage another
*/

#include "MemDebug.h"
#include "CombatAssignment.h"
#include "CombatGroup.h"
#include "Mission.h"

// +--------------------------------------------------------------------+

CombatAssignment::CombatAssignment(int t, CombatGroup* obj, CombatGroup* rsc)
    : type(t), objective(obj), resource(rsc)
{
}

// +--------------------------------------------------------------------+

CombatAssignment::~CombatAssignment()
{
}

// +--------------------------------------------------------------------+
// This is used to sort assignments into a priority list.
// Higher priorities should come first in the list, so the
// sense of the operator is "backwards" from the usual.

int
CombatAssignment::operator < (const CombatAssignment& a) const
{
    if (!objective)
    return 0;

    if (!a.objective)
    return 1;

    return objective->GetPlanValue() > a.objective->GetPlanValue();
}

// +--------------------------------------------------------------------+

const char*
CombatAssignment::GetDescription() const
{
    static char desc[256];

    if (!resource)
        sprintf_s(desc, "%s %s", 
            (const char*) Mission::RoleName(type),
            (const char*) objective->Name());
    else
        sprintf_s(desc, "%s %s %s", 
            (const char*) resource->Name(),
            (const char*) Mission::RoleName(type),
            (const char*) objective->Name());

    return desc;
}

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
    FILE:         ParseUtil.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Parser utility functions
*/

#ifndef ParseUtil_h
#define ParseUtil_h

#include <vector>
#include "Types.h"
#include "Geometry.h"
#include "Color.h"

#include "Text.h"
#include "Parser.h"
#include "Reader.h"
#include "Token.h"

// +--------------------------------------------------------------------+

bool GetDefBool(bool& dst, TermDef* def, const char* file);
bool GetDefText(Text& dst, TermDef* def, const char* file);
bool GetDefText(char* dst, TermDef* def, const char* file);
bool GetDefNumber(int& dst, TermDef* def, const char* file);
bool GetDefNumber(DWORD& dst, TermDef* def, const char* file);
bool GetDefNumber(float& dst, TermDef* def, const char* file);
bool GetDefNumber(double& dst, TermDef* def, const char* file);
bool GetDefVec(Vec3& dst, TermDef* def, const char* file);
bool GetDefColor(Color& dst, TermDef* def, const char* file);
bool GetDefColor(ColorValue& dst, TermDef* def, const char* file);
bool GetDefRect(Rect& dst, TermDef* def, const char* file);
bool GetDefInsets(Insets& dst, TermDef* def, const char* file);
bool GetDefTime(int& dst, TermDef* def, const char* file);

bool GetDefArray(int*    dst, int size, TermDef* def, const char* file);
bool GetDefArray(float*  dst, int size, TermDef* def, const char* file);
bool GetDefArray(double* dst, int size, TermDef* def, const char* file);
bool GetDefArray(std::vector<DWORD>& array,      TermDef* def, const char* file);
bool GetDefArray(std::vector<float>& array,      TermDef* def, const char* file);

// +--------------------------------------------------------------------+

#endif ParseUtil_h

/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib
    FILE:         ParseUtil.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Parser utility functions
*/

#ifndef ParseUtil_h
#define ParseUtil_h

#include "Types.h"
#include "Geometry.h"
#include "Color.h"
#include "ArrayList.h"

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
bool GetDefArray(ArrayList& array,      TermDef* def, const char* file);
bool GetDefArray(FloatList& array,      TermDef* def, const char* file);

// +--------------------------------------------------------------------+

#endif ParseUtil_h

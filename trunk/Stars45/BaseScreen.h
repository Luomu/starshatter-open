/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         BaseScreen.h
    AUTHOR:       John DiCamillo

*/

#ifndef BaseScreen_h
#define BaseScreen_h

#include "Types.h"
#include "Bitmap.h"
#include "Screen.h"

// +--------------------------------------------------------------------+

class Screen;
class Sim;
class Window;
class Font;
class NavDlg;
class MsnElemDlg;
class AudDlg;
class VidDlg;
class ModDlg;
class ModInfoDlg;
class OptDlg;
class CtlDlg;
class KeyDlg;
class JoyDlg;
class MsgDlg;

// +--------------------------------------------------------------------+

class BaseScreen
{
public:
   BaseScreen()                           { }
   virtual ~BaseScreen()                  { }

   virtual void         ShowNavDlg()      { }
   virtual void         HideNavDlg()      { }
   virtual bool         IsNavShown()      { return false; }
   virtual NavDlg*      GetNavDlg()       { return 0; }

   virtual void         ShowMsnElemDlg()  { }
   virtual void         HideMsnElemDlg()  { }
   virtual MsnElemDlg*  GetMsnElemDlg()   { return 0; }

   virtual AudDlg*      GetAudDlg() const { return 0; }
   virtual VidDlg*      GetVidDlg() const { return 0; }
   virtual ModDlg*      GetModDlg() const { return 0; }
   virtual ModInfoDlg*  GetModInfoDlg() const { return 0; }
   virtual OptDlg*      GetOptDlg() const { return 0; }
   virtual CtlDlg*      GetCtlDlg() const { return 0; }
   virtual JoyDlg*      GetJoyDlg() const { return 0; }
   virtual KeyDlg*      GetKeyDlg() const { return 0; }

   virtual void         ShowAudDlg()      { }
   virtual void         ShowVidDlg()      { }
   virtual void         ShowModDlg()      { }
   virtual void         ShowModInfoDlg()  { }
   virtual void         ShowOptDlg()      { }
   virtual void         ShowCtlDlg()      { }
   virtual void         ShowJoyDlg()      { }
   virtual void         ShowKeyDlg()      { }

   virtual void         ShowMsgDlg()      { }
   virtual void         HideMsgDlg()      { }
   virtual bool         IsMsgShown()      { return false; }
   virtual MsgDlg*      GetMsgDlg()       { return 0; }

   virtual void         ApplyOptions()    { }
   virtual void         CancelOptions()   { }
};

// +--------------------------------------------------------------------+

#endif BaseScreen_h


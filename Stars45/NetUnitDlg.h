/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         NetUnitDlg.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Multiplayer Unit Selection Dialog Active Window class
*/

#ifndef NetUnitDlg_h
#define NetUnitDlg_h

#include "Types.h"
#include "FormWindow.h"
#include "Bitmap.h"
#include "Button.h"
#include "ComboBox.h"
#include "ListBox.h"
#include "EditBox.h"
#include "Font.h"

// +--------------------------------------------------------------------+

class MenuScreen;
class NetClientConfig;
class NetLobby;
class NetChatEntry;
class NetUser;

// +--------------------------------------------------------------------+

class NetUnitDlg : public FormWindow
{
public:
   NetUnitDlg(Screen* s, FormDef& def, MenuScreen* mgr);
   virtual ~NetUnitDlg();

   virtual void      RegisterControls();
   virtual void      Show();
   virtual void      ExecFrame();

   // Operations:
   virtual void      OnSelect(AWEvent* event);
   virtual void      OnUnit(AWEvent* event);
   virtual void      OnMap(AWEvent* event);
   virtual void      OnUnMap(AWEvent* event);
   virtual void      OnBan(AWEvent* event);
   virtual void      OnBanConfirm(AWEvent* event);
   virtual void      OnApply(AWEvent* event);
   virtual void      OnCancel(AWEvent* event);

   virtual void      ExecLobbyFrame();

   virtual bool      GetHostMode()  const { return host_mode; }
   virtual void      SetHostMode(bool h)  { host_mode = h;    }

protected:
   virtual void      GetAvailable();
   virtual void      GetUnits();
   virtual void      GetChat();
   virtual void      SendChat(Text msg);
   virtual void      CheckUnitMapping();

   MenuScreen*       manager;

   ListBox*          lst_players;
   ListBox*          lst_units;
   ListBox*          lst_chat;
   EditBox*          edt_chat;

   Button*           btn_select;
   Button*           btn_map;
   Button*           btn_unmap;
   Button*           btn_ban;
   Button*           btn_apply;
   Button*           btn_cancel;

   NetLobby*         net_lobby;

   int               last_chat;
   int               unit_index;
   bool              host_mode;
};

// +--------------------------------------------------------------------+

#endif NetUnitDlg_h


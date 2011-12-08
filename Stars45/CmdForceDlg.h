/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         CmdForceDlg.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Operational Command Dialog (Order of Battle Tab)
*/

#ifndef CmdForceDlg_h
#define CmdForceDlg_h

#include "Types.h"
#include "FormWindow.h"
#include "CmdDlg.h"
#include "Bitmap.h"
#include "Button.h"
#include "ComboBox.h"
#include "ListBox.h"
#include "Font.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class CmdForceDlg : public FormWindow,
                    public CmdDlg
{
public:
   CmdForceDlg(Screen* s, FormDef& def, CmpnScreen* mgr);
   virtual ~CmdForceDlg();

   virtual void      RegisterControls();
   virtual void      Show();
   virtual void      ExecFrame();

   // Operations:
   virtual void      OnMode(AWEvent* event);
   virtual void      OnSave(AWEvent* event);
   virtual void      OnExit(AWEvent* event);
   virtual void      OnForces(AWEvent* event);
   virtual void      OnCombat(AWEvent* event);
   virtual void      OnTransfer(AWEvent* event);

protected:
   void              ShowCombatant(Combatant* c);
   void              AddCombatGroup(CombatGroup* grp, bool last_child=false);
   bool              CanTransfer(CombatGroup* grp);
   bool              IsVisible(Combatant* c);

   CmpnScreen*       manager;

   ComboBox*         cmb_forces;
   ListBox*          lst_combat;
   ListBox*          lst_desc;
   Button*           btn_transfer;

   Starshatter*      stars;
   Campaign*         campaign;
   CombatGroup*      current_group;
   CombatUnit*       current_unit;
};

#endif CmdForceDlg_h


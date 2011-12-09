/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         CmdForceDlg.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Operational Command Dialog (Order of Battle Tab)
*/

#include "MemDebug.h"
#include "CmdForceDlg.h"
#include "CmdMsgDlg.h"
#include "CmpnScreen.h"
#include "Starshatter.h"
#include "Campaign.h"
#include "Combatant.h"
#include "CombatAssignment.h"
#include "CombatGroup.h"
#include "CombatUnit.h"
#include "CombatZone.h"
#include "Ship.h"
#include "ShipDesign.h"
#include "Player.h"
#include "Weapon.h"

#include "Game.h"
#include "DataLoader.h"
#include "Button.h"
#include "ComboBox.h"
#include "ListBox.h"
#include "Slider.h"
#include "Video.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "ParseUtil.h"
#include "FormatUtil.h"

// +--------------------------------------------------------------------+
// DECLARE MAPPING FUNCTIONS:

DEF_MAP_CLIENT(CmdForceDlg, OnMode);
DEF_MAP_CLIENT(CmdForceDlg, OnSave);
DEF_MAP_CLIENT(CmdForceDlg, OnExit);
DEF_MAP_CLIENT(CmdForceDlg, OnForces);
DEF_MAP_CLIENT(CmdForceDlg, OnCombat);
DEF_MAP_CLIENT(CmdForceDlg, OnTransfer);

// +--------------------------------------------------------------------+

CmdForceDlg::CmdForceDlg(Screen* s, FormDef& def, CmpnScreen* mgr)
: FormWindow(s, 0, 0, s->Width(), s->Height()), manager(mgr),
CmdDlg(mgr),
cmb_forces(0), lst_combat(0), lst_desc(0),
stars(0), campaign(0), current_group(0), current_unit(0),
btn_transfer(0)
{
	stars    = Starshatter::GetInstance();
	campaign = Campaign::GetCampaign();

	Init(def);
}

CmdForceDlg::~CmdForceDlg()
{
}

// +--------------------------------------------------------------------+

void
CmdForceDlg::RegisterControls()
{
	cmb_forces     = (ComboBox*) FindControl(400);
	lst_combat     = (ListBox*)  FindControl(401);
	lst_desc       = (ListBox*)  FindControl(402);
	btn_transfer   = (Button*)   FindControl(403);

	RegisterCmdControls(this);

	if (btn_save)
	REGISTER_CLIENT(EID_CLICK,  btn_save,    CmdForceDlg, OnSave);

	if (btn_exit)
	REGISTER_CLIENT(EID_CLICK,  btn_exit,    CmdForceDlg, OnExit);

	for (int i = 0; i < 5; i++) {
		if (btn_mode[i])
		REGISTER_CLIENT(EID_CLICK,  btn_mode[i], CmdForceDlg, OnMode);
	}

	if (cmb_forces)
	REGISTER_CLIENT(EID_SELECT, cmb_forces,  CmdForceDlg, OnForces);

	if (lst_combat) {
		lst_combat->AddColumn("datatype", 0);
		REGISTER_CLIENT(EID_SELECT, lst_combat,  CmdForceDlg, OnCombat);
	}

	if (btn_transfer) {
		btn_transfer->SetEnabled(false);
		REGISTER_CLIENT(EID_CLICK, btn_transfer, CmdForceDlg, OnTransfer);
	}
}

// +--------------------------------------------------------------------+

void
CmdForceDlg::ExecFrame()
{
	CmdDlg::ExecFrame();
}

// +--------------------------------------------------------------------+

void
CmdForceDlg::Show()
{
	mode = MODE_FORCES;

	FormWindow::Show();
	ShowCmdDlg();

	cmb_forces->ClearItems();

	campaign = Campaign::GetCampaign();

	if (campaign) {
		List<Combatant>& combatants = campaign->GetCombatants();

		if (combatants.size() > 0) {
			for (int i = 0; i < combatants.size(); i++) {
				if (IsVisible(combatants[i])) {
					cmb_forces->AddItem(combatants[i]->Name());
				}
			}

			cmb_forces->SetSelection(0);
			Combatant* c = combatants[0];
			ShowCombatant(c);
		}
	}
}

// +--------------------------------------------------------------------+

bool
CmdForceDlg::IsVisible(Combatant* c)
{
	int nvis = 0;

	if (c) {
		CombatGroup* force = c->GetForce();

		if (force) {
			List<CombatGroup>& groups = force->GetComponents();
			for (int i = 0; i < groups.size(); i++) {
				CombatGroup* g = groups[i];

				if (g->Type()       < CombatGroup::CIVILIAN &&
						g->CountUnits() > 0                     && 
						g->IntelLevel() >= Intel::KNOWN)

				nvis++;
			}
		}
	}

	return nvis > 0;
}

// +--------------------------------------------------------------------+

static char pipe_stack[32];
static bool blank_line = false;

void
CmdForceDlg::ShowCombatant(Combatant* c)
{
	if (!lst_combat || !c) return;

	lst_combat->ClearItems();
	ZeroMemory(pipe_stack, 32);

	CombatGroup* force = c->GetForce();

	if (force) {
		List<CombatGroup>& groups = force->GetComponents();
		for (int i = 0; i < groups.size(); i++) {
			CombatGroup* g = groups[i];
			if (g->Type() < CombatGroup::CIVILIAN && g->CountUnits() > 0)
			AddCombatGroup(g);
		}
	}

	current_group = 0;
	current_unit  = 0;

	if (lst_desc)     lst_desc->ClearItems();
	if (btn_transfer) btn_transfer->SetEnabled(false);
}

void
CmdForceDlg::AddCombatGroup(CombatGroup* grp, bool last_child)
{
	if (!lst_combat || !grp || grp->IntelLevel() < Intel::KNOWN) return;

	char prefix[4];

	if (!grp->GetParent() || grp->GetParent()->Type() == CombatGroup::FORCE) {
		if (!grp->IsExpanded()) {
			prefix[0] = Font::PIPE_PLUS;
			prefix[1] = 0;
		}
		else {
			prefix[0] = Font::PIPE_MINUS;
			prefix[1] = 0;
		}
	}
	else {
		prefix[0] = last_child ? Font::PIPE_LL : Font::PIPE_LT;
		prefix[1] = Font::PIPE_HORZ;
		prefix[2] = 0;
		prefix[3] = 0;

		if (grp->GetLiveComponents().size() > 0 || grp->GetUnits().size() > 0) {
			if (grp->IsExpanded())
			prefix[1] = Font::PIPE_MINUS;
			else
			prefix[1] = Font::PIPE_PLUS;
		}
	}

	int index = lst_combat->AddItemWithData(
	Text(pipe_stack) + 
	Text(prefix) + 
	grp->GetDescription(), 
	(DWORD) grp);
	lst_combat->SetItemData(index-1, 1, 0);
	blank_line = false;

	int stacklen = strlen(pipe_stack);

	if (!grp->GetParent() || grp->GetParent()->Type() == CombatGroup::FORCE)
	; // no stack after first entry
	else if (prefix[0] == Font::PIPE_LT)
	pipe_stack[stacklen++] = Font::PIPE_VERT;
	else
	pipe_stack[stacklen++] = Font::PIPE_NBSP;
	pipe_stack[stacklen] = 0;

	if (grp->IsExpanded() && grp->GetUnits().size() > 0) {
		prefix[0] = (grp->GetLiveComponents().size() > 0) ? Font::PIPE_VERT : Font::PIPE_NBSP;
		prefix[1] = Font::PIPE_NBSP;
		prefix[2] = 0;
		prefix[3] = 0;

		ListIter<CombatUnit>  unit_iter  = grp->GetUnits();
		while (++unit_iter) {
			CombatUnit* unit = unit_iter.value();
			char        info[512];
			int         damage = (int) (100 * unit->GetSustainedDamage() / unit->GetDesign()->integrity);
			
			if (damage < 1 || unit->DeadCount() >= unit->Count()) {
				sprintf(info, "%s%s%s", pipe_stack, prefix, unit->GetDescription());
			}
			else {
				sprintf(info, "%s%s%s %d%% damage", pipe_stack, prefix, unit->GetDescription(),
				damage);
			}

			int index = lst_combat->AddItemWithData(info, (DWORD) unit);

			lst_combat->SetItemData(index-1, 1, 1);
			lst_combat->SetItemColor(index-1, lst_combat->GetForeColor() * 0.65);
		}

		// blank line after last unit in group:
		lst_combat->AddItem(Text(pipe_stack) + Text(prefix));
		blank_line = true;
	}

	if (grp->IsExpanded() && grp->GetLiveComponents().size() > 0) {
		List<CombatGroup>& groups = grp->GetLiveComponents();
		for (int i = 0; i < groups.size(); i++) {
			AddCombatGroup(groups[i], i == groups.size()-1);
		}

		// blank line after last group in group:
		if (!blank_line) {
			lst_combat->AddItem(pipe_stack);
			blank_line = true;
		}
	}

	pipe_stack[stacklen-1] = 0;
}

// +--------------------------------------------------------------------+

void
CmdForceDlg::OnForces(AWEvent* event)
{
	Text name = cmb_forces->GetSelectedItem();

	campaign = Campaign::GetCampaign();

	if (campaign) {
		ListIter<Combatant> iter = campaign->GetCombatants();

		while (++iter) {
			Combatant* c = iter.value();
			
			if (name == c->Name()) {
				ShowCombatant(c);
				break;
			}
		}
	}
}

// +--------------------------------------------------------------------+

struct WepGroup {
	Text name;
	int  count;

	WepGroup() : count(0) { }
};

WepGroup* FindWepGroup(WepGroup* weapons, const char* name)
{
	WepGroup*   group = 0;
	WepGroup*   iter  = weapons;
	int         w     = 0;
	int         first = -1;

	while (!group && w < 8) {
		if (first < 0 && iter->name.length() == 0)
		first = w;

		if (!stricmp(iter->name, name))
		group = iter;

		iter++;
		w++;
	}

	if (!group && first >= 0) {
		group = weapons + first;
		group->name = name;
	}

	return group;
}

void
CmdForceDlg::OnCombat(AWEvent* event)
{
	static int     old_index  = -1;

	int   top_index   = 0;
	bool  expand      = false;
	DWORD data        = 0;
	DWORD type        = 0;

	current_group     = 0;
	current_unit      = 0;

	if (lst_combat) {
		top_index = lst_combat->GetTopIndex();

		int   index = lst_combat->GetListIndex();
		data  = lst_combat->GetItemData(index);
		type  = lst_combat->GetItemData(index, 1);
		Text  item  = lst_combat->GetItemText(index);
		int   nplus = item.indexOf(Font::PIPE_PLUS);
		int   xplus = -1;
		int   dx    = 10000;

		if (nplus < 0)
		nplus = item.indexOf(Font::PIPE_MINUS);

		if (nplus >= 0 && nplus < 64) {
			char pipe[64];
			strncpy(pipe, item.data(), nplus+1);
			pipe[nplus+1] = 0;

			Rect rect(0, 0, 1000, 20);
			lst_combat->DrawText(pipe, 0, rect, DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_CALCRECT);

			xplus = rect.w;
		}

		if (xplus > 0) {
			dx = Mouse::X() - (lst_combat->GetRect().x + xplus - 16);
		}

		// look for click on plus/minus in pipe
		if (dx >= 0 && dx < 16) {
			if (data && type == 0) {
				CombatGroup* grp = (CombatGroup*) data;
				grp->SetExpanded(!grp->IsExpanded());
				expand = true;

				current_group = grp;
				current_unit  = 0;
			}
		}

		old_index  = index;
	}

	if (campaign && data) {
		if (!expand) {
			lst_desc->ClearItems();

			// combat group
			if (type == 0) {
				CombatGroup* grp = (CombatGroup*) data;

				current_group = grp;
				current_unit  = 0;

				// if has units, show location and assignment
				if (grp->GetUnits().size() > 0) {
					char txt[64];
					int  n;

					n = lst_desc->AddItem("Group:") - 1;
					lst_desc->SetItemText(n, 1, grp->GetDescription());
					n = lst_desc->AddItem("Sector:") - 1;
					lst_desc->SetItemText(n, 1, grp->GetRegion());

					lst_desc->AddItem(" ");
					n = lst_desc->AddItem("Sorties:") - 1;

					if (grp->Sorties() >= 0)
					sprintf(txt, "%d", grp->Sorties());
					else
					strcpy(txt, "Unavail");

					lst_desc->SetItemText(n, 1, txt);

					n = lst_desc->AddItem("Kills:") - 1;

					if (grp->Kills() >= 0)
					sprintf(txt, "%d", grp->Kills());
					else
					strcpy(txt, "Unavail");

					lst_desc->SetItemText(n, 1, txt);

					n = lst_desc->AddItem("Eff Rating:") - 1;

					if (grp->Points() >= 0) {
						if (grp->Sorties() > 0)
						sprintf(txt, "%.1f", (double) grp->Points() / grp->Sorties());
						else
						sprintf(txt, "%.1f", (double) grp->Points());
					}
					else {
						strcpy(txt, "Unavail");
					}

					lst_desc->SetItemText(n, 1, txt);
				}

				// else (if high-level) show components
				else {
					int n;

					n = lst_desc->AddItem("Group:") - 1;
					lst_desc->SetItemText(n, 1, grp->GetDescription());

					ListIter<CombatGroup> c = grp->GetLiveComponents();
					while (++c) {
						n = lst_desc->AddItem("-") - 1;
						lst_desc->SetItemText(n, 1, c->GetDescription());
					}
				}
			}
			// combat unit
			else {
				CombatUnit* unit = (CombatUnit*) data;
				current_group = unit->GetCombatGroup();
				current_unit  = unit;

				int  n;
				char txt[64];

				n = lst_desc->AddItem("Unit:") - 1;
				lst_desc->SetItemText(n, 1, unit->GetDescription());
				n = lst_desc->AddItem("Sector:") - 1;
				lst_desc->SetItemText(n, 1, unit->GetRegion());

				const ShipDesign* design = unit->GetDesign();
				if (design) {
					lst_desc->AddItem(" ");
					n = lst_desc->AddItem("Type:") - 1;
					lst_desc->SetItemText(n, 1, Ship::ClassName(design->type));
					n = lst_desc->AddItem("Class:") - 1;
					lst_desc->SetItemText(n, 1, design->DisplayName());

					if (design->type < Ship::STATION)
					FormatNumber(txt, design->radius/2);
					else
					FormatNumber(txt, design->radius*2);

					strcat(txt, " m");

					n = lst_desc->AddItem("Length:") - 1;
					lst_desc->SetItemText(n, 1, txt);

					FormatNumber(txt, design->mass);
					strcat(txt, " T");

					n = lst_desc->AddItem("Mass:") - 1;
					lst_desc->SetItemText(n, 1, txt);

					FormatNumber(txt, design->integrity);
					n = lst_desc->AddItem("Hull:") - 1;
					lst_desc->SetItemText(n, 1, txt);

					if (design->weapons.size()) {
						lst_desc->AddItem(" ");
						n = lst_desc->AddItem("Weapons:") - 1;

						WepGroup groups[8];
						for (int w = 0; w < design->weapons.size(); w++) {
							Weapon*     gun   = design->weapons[w];
							WepGroup*   group = FindWepGroup(groups, gun->Group());

							if (group)
							group->count++;
						}

						for (int g = 0; g < 8; g++) {
							WepGroup* group = &groups[g];
							if (group && group->count) {
								sprintf(txt, "%s (%d)", group->name.data(), group->count);
								if (g > 0) n = lst_desc->AddItem(" ") - 1;
								lst_desc->SetItemText(n, 1, txt);
							}
						}
					}
				}
			}
		}

		else {
			List<Combatant>& combatants = campaign->GetCombatants();
			Combatant* c = combatants[0];

			if (cmb_forces) {
				Text name = cmb_forces->GetSelectedItem();

				for (int i = 0; i < combatants.size(); i++) {
					c = combatants[i];
					
					if (name == c->Name()) {
						break;
					}
				}
			}

			if (c) {
				ShowCombatant(c);

				lst_combat->ScrollTo(top_index);
				lst_combat->SetSelected(old_index);
			}
		}
	}

	if (btn_transfer && campaign && current_group)
	btn_transfer->SetEnabled( campaign->IsActive() && 
	CanTransfer(current_group) );
}

// +--------------------------------------------------------------------+

bool
CmdForceDlg::CanTransfer(CombatGroup* grp)
{
	if (!grp || !campaign)
	return false;

	if (grp->Type() < CombatGroup::WING)
	return false;

	if (grp->Type() > CombatGroup::CARRIER_GROUP)
	return false;

	if (grp->Type() == CombatGroup::FLEET ||
			grp->Type() == CombatGroup::LCA_SQUADRON)
	return false;

	CombatGroup* player_group = campaign->GetPlayerGroup();
	if (player_group->GetIFF() != grp->GetIFF())
	return false;

	return true;
}


// +--------------------------------------------------------------------+

void
CmdForceDlg::OnSave(AWEvent* event)
{
	CmdDlg::OnSave(event);
}

void
CmdForceDlg::OnExit(AWEvent* event)
{
	CmdDlg::OnExit(event);
}

void
CmdForceDlg::OnMode(AWEvent* event)
{
	CmdDlg::OnMode(event);
}


// +--------------------------------------------------------------------+

void
CmdForceDlg::OnTransfer(AWEvent* event)
{
	if (campaign && current_group) {

		// check player rank/responsibility:
		Player* player    = Player::GetCurrentPlayer();
		int     cmd_class = Ship::FIGHTER;

		switch (current_group->Type()) {
		case CombatGroup::WING:
		case CombatGroup::INTERCEPT_SQUADRON:
		case CombatGroup::FIGHTER_SQUADRON:
			cmd_class = Ship::FIGHTER;
			break;

		case CombatGroup::ATTACK_SQUADRON:
			cmd_class = Ship::ATTACK;
			break;

		case CombatGroup::LCA_SQUADRON:
			cmd_class = Ship::LCA;
			break;

		case CombatGroup::DESTROYER_SQUADRON:
			cmd_class = Ship::DESTROYER;
			break;

		case CombatGroup::BATTLE_GROUP:
			cmd_class = Ship::CRUISER;
			break;

		case CombatGroup::CARRIER_GROUP:
		case CombatGroup::FLEET:
			cmd_class = Ship::CARRIER;
			break;
		}

		char transfer_info[512];

		if (player->CanCommand(cmd_class)) {
			if (current_unit) {
				campaign->SetPlayerUnit(current_unit);

				sprintf(transfer_info, "Your transfer request has been approved, %s %s.  You are now assigned to the %s.  Good luck.\n\nFleet Admiral A. Evars FORCOM\nCommanding",
				Player::RankName(player->Rank()),
				player->Name().data(),
				current_unit->GetDescription());
			}
			else {
				campaign->SetPlayerGroup(current_group);

				sprintf(transfer_info, "Your transfer request has been approved, %s %s.  You are now assigned to the %s.  Good luck.\n\nFleet Admiral A. Evars FORCOM\nCommanding",
				Player::RankName(player->Rank()),
				player->Name().data(),
				current_group->GetDescription());
			}

			Button::PlaySound(Button::SND_ACCEPT);

			CmdMsgDlg* msgdlg = manager->GetCmdMsgDlg();
			msgdlg->Title()->SetText("Transfer Approved");
			msgdlg->Message()->SetText(transfer_info);
			msgdlg->Message()->SetTextAlign(DT_LEFT);

			manager->ShowCmdMsgDlg();
		}

		else {
			Button::PlaySound(Button::SND_REJECT);

			sprintf(transfer_info, "Your transfer request has been denied, %s %s.  The %s requires a command rank of %s.  Please return to your unit and your duties.\n\nFleet Admiral A. Evars FORCOM\nCommanding",
			Player::RankName(player->Rank()),
			player->Name().data(),
			current_group->GetDescription(),
			Player::RankName(Player::CommandRankRequired(cmd_class)));

			CmdMsgDlg* msgdlg = manager->GetCmdMsgDlg();
			msgdlg->Title()->SetText("Transfer Denied");
			msgdlg->Message()->SetText(transfer_info);
			msgdlg->Message()->SetTextAlign(DT_LEFT);

			manager->ShowCmdMsgDlg();
		}
	}
}

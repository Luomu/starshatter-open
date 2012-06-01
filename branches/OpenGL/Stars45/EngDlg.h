/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         EngDlg.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Engineering (Power/Maint) Dialog Active Window class
*/

#ifndef EngDlg_h
#define EngDlg_h

#include "Types.h"
#include "FormWindow.h"
#include "Bitmap.h"
#include "Button.h"
#include "ComboBox.h"
#include "ListBox.h"
#include "Font.h"

// +--------------------------------------------------------------------+

class GameScreen;
class Ship;
class PowerSource;
class System;
class Component;

class PowerClient;

// +--------------------------------------------------------------------+

class EngDlg : public FormWindow
{
public:
	EngDlg(Screen* s, FormDef& def, GameScreen* mgr);
	virtual ~EngDlg();

	virtual void      Show();
	virtual void      Hide();
	virtual void      RegisterControls();

	// Operations:
	virtual void      OnSource(AWEvent* event);
	virtual void      OnClient(AWEvent* event);
	virtual void      OnRouteStart(AWEvent* event);
	virtual void      OnRouteComplete(AWEvent* event);
	virtual void      OnPowerOff(AWEvent* event);
	virtual void      OnPowerOn(AWEvent* event);
	virtual void      OnOverride(AWEvent* event);
	virtual void      OnPowerLevel(AWEvent* event);
	virtual void      OnComponent(AWEvent* event);
	virtual void      OnAutoRepair(AWEvent* event);
	virtual void      OnRepair(AWEvent* event);
	virtual void      OnReplace(AWEvent* event);
	virtual void      OnQueue(AWEvent* event);
	virtual void      OnPriorityIncrease(AWEvent* event);
	virtual void      OnPriorityDecrease(AWEvent* event);
	virtual void      OnClose(AWEvent* event);

	virtual void      ExecFrame();
	void              UpdateRouteTables();
	void              UpdateSelection();
	void              SetShip(Ship* s);

protected:
	Ship*             ship;
	GameScreen*       manager;

	Button*           close_btn;
	Button*           sources[4];
	Slider*           source_levels[4];
	ListBox*          clients[4];
	ListBox*          components;
	ListBox*          repair_queue;
	ActiveWindow*     selected_name;
	Button*           power_off;
	Button*           power_on;
	Button*           override;
	Slider*           power_level;
	Slider*           capacity;
	Button*           auto_repair;
	Button*           repair;
	Button*           replace;
	ActiveWindow*     repair_time;
	ActiveWindow*     replace_time;
	Button*           priority_increase;
	Button*           priority_decrease;

	PowerSource*      route_source;
	List<System>      route_list;

	PowerSource*      selected_source;
	List<System>      selected_clients;

	System*           selected_repair;
	Component*        selected_component;
};

#endif EngDlg_h


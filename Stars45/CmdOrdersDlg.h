/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         CmdOrdersDlg.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Operational Command Dialog (Campaign Orders Tab)
*/

#ifndef CmdOrdersDlg_h
#define CmdOrdersDlg_h

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

class CmdOrdersDlg : public FormWindow,
public CmdDlg
{
public:
	CmdOrdersDlg(Screen* s, FormDef& def, CmpnScreen* mgr);
	virtual ~CmdOrdersDlg();

	virtual void      RegisterControls();
	virtual void      ExecFrame();
	virtual void      Show();

	// Operations:
	virtual void      OnMode(AWEvent* event);
	virtual void      OnSave(AWEvent* event);
	virtual void      OnExit(AWEvent* event);

protected:
	CmpnScreen*       manager;

	ActiveWindow*     lbl_orders;

	Starshatter*      stars;
	Campaign*         campaign;
};

#endif CmdOrdersDlg_h


/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         NetPassDlg.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Network Server Password Dialog Active Window class
*/

#ifndef NetPassDlg_h
#define NetPassDlg_h

#include "Types.h"
#include "FormWindow.h"
#include "Bitmap.h"
#include "Button.h"
#include "ComboBox.h"
#include "ListBox.h"
#include "Font.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class MenuScreen;

// +--------------------------------------------------------------------+

class NetPassDlg : public FormWindow
{
public:
	NetPassDlg(Screen* s, FormDef& def, MenuScreen* mgr);
	virtual ~NetPassDlg();

	virtual void      RegisterControls();
	virtual void      Show();

	// Operations:
	virtual void      ExecFrame();

	virtual void      OnApply(AWEvent* event);
	virtual void      OnCancel(AWEvent* event);

protected:
	MenuScreen*       manager;

	Button*           btn_apply;
	Button*           btn_cancel;
	EditBox*          edt_pass;
	ActiveWindow*     lbl_name;
};

#endif NetPassDlg_h


/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         NetAddrDlg.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Network Server Address/Port Dialog Active Window class
*/

#ifndef NetAddrDlg_h
#define NetAddrDlg_h

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

class NetAddrDlg : public FormWindow
{
public:
	NetAddrDlg(Screen* s, FormDef& def, MenuScreen* mgr);
	virtual ~NetAddrDlg();

	virtual void      RegisterControls();
	virtual void      Show();

	// Operations:
	virtual void      ExecFrame();

	virtual void      OnSave(AWEvent* event);
	virtual void      OnCancel(AWEvent* event);

protected:
	MenuScreen*       manager;

	Button*           btn_save;
	Button*           btn_cancel;
	EditBox*          edt_name;
	EditBox*          edt_addr;
	EditBox*          edt_port;
	EditBox*          edt_pass;
};

#endif NetAddrDlg_h


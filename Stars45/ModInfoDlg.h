/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         ModInfoDlg.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Mod Info Splash Dialog Active Window class
*/

#ifndef ModInfoDlg_h
#define ModInfoDlg_h

#include "Types.h"
#include "FormWindow.h"
#include "Bitmap.h"
#include "Button.h"
#include "ComboBox.h"
#include "ListBox.h"
#include "Font.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class BaseScreen;
class ModConfig;
class ModInfo;

// +--------------------------------------------------------------------+

class ModInfoDlg : public FormWindow
{
public:
	ModInfoDlg(Screen* s, FormDef& def, BaseScreen* mgr);
	virtual ~ModInfoDlg();

	virtual void      RegisterControls();
	virtual void      Show();
	virtual void      ExecFrame();

	// Operations:
	virtual void      SetModInfo(ModInfo* info);
	virtual void      OnAccept(AWEvent* event);

protected:
	BaseScreen*       manager;

	ActiveWindow*     lbl_name;
	ActiveWindow*     lbl_desc;
	ActiveWindow*     lbl_copy;
	ImageBox*         img_logo;

	Button*           btn_accept;

	ModInfo*          mod_info;

	Bitmap            bmp_default;
};

#endif ModInfoDlg_h


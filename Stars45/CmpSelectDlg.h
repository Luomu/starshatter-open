/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         CmpSelectDlg.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Mission Select Dialog Active Window class
*/

#ifndef CmpSelectDlg_h
#define CmpSelectDlg_h

#include "Types.h"
#include "FormWindow.h"
#include "Bitmap.h"
#include "Button.h"
#include "ComboBox.h"
#include "ListBox.h"
#include "Font.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class  MenuScreen;
class  Campaign;
class  Starshatter;

// +--------------------------------------------------------------------+

class CmpSelectDlg : public FormWindow
{
public:
	CmpSelectDlg(Screen* s, FormDef& def, MenuScreen* mgr);
	virtual ~CmpSelectDlg();

	virtual void      RegisterControls();
	virtual void      Show();
	virtual void      ExecFrame();
	virtual bool      CanClose();

	// Operations:
	virtual void      OnCampaignSelect(AWEvent* event);
	virtual void      OnNew(AWEvent* event);
	virtual void      OnSaved(AWEvent* event);
	virtual void      OnDelete(AWEvent* event);
	virtual void      OnConfirmDelete(AWEvent* event);
	virtual void      OnAccept(AWEvent* event);
	virtual void      OnCancel(AWEvent* event);

	virtual DWORD     LoadProc();

protected:
	virtual void      StartLoadProc();
	virtual void      StopLoadProc();
	virtual void      ShowNewCampaigns();
	virtual void      ShowSavedCampaigns();

	MenuScreen*       manager;

	Button*           btn_new;
	Button*           btn_saved;
	Button*           btn_delete;
	Button*           btn_accept;
	Button*           btn_cancel;

	ListBox*          lst_campaigns;

	ActiveWindow*     description;

	Starshatter*      stars;
	Campaign*         campaign;
	int               selected_mission;
	HANDLE            hproc;
	ThreadSync        sync;
	bool              loading;
	bool              loaded;
	Text              load_file;
	int               load_index;
	bool              show_saved;
	List<Bitmap>      images;

	Text              select_msg;
};

#endif CmpSelectDlg_h


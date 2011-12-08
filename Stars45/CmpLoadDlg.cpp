/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         CmpLoadDlg.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
*/

#include "MemDebug.h"
#include "CmpLoadDlg.h"
#include "Campaign.h"
#include "Starshatter.h"
#include "FormatUtil.h"

#include "Game.h"
#include "DataLoader.h"
#include "Video.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "ImageBox.h"
#include "Slider.h"

// +--------------------------------------------------------------------+

CmpLoadDlg::CmpLoadDlg(Screen* s, FormDef& def)
   : FormWindow(s, 0, 0, s->Width(), s->Height()),
     lbl_progress(0), lbl_activity(0), lbl_title(0), img_title(0), show_time(0)
{
   Init(def);
}

CmpLoadDlg::~CmpLoadDlg()
{
}

void
CmpLoadDlg::RegisterControls()
{
   img_title    = (ImageBox*) FindControl(100);
   lbl_title    =             FindControl(200);
   lbl_activity =             FindControl(101);
   lbl_progress = (Slider*)   FindControl(102);
}

void
CmpLoadDlg::Show()
{
   FormWindow::Show();

   Campaign* campaign = Campaign::GetCampaign();

   if (campaign) {
      Bitmap* bmp = campaign->GetImage(3);
      if (img_title && bmp) {
         Rect tgt_rect;
         tgt_rect.w = img_title->Width();
         tgt_rect.h = img_title->Height();

         img_title->SetTargetRect(tgt_rect);
         img_title->SetPicture(*bmp);
      }

      if (lbl_title)
         lbl_title->SetText(campaign->Name());
   }

   show_time = Game::RealTime();
}

// +--------------------------------------------------------------------+

void
CmpLoadDlg::ExecFrame()
{
   Starshatter* stars = Starshatter::GetInstance();

   if (stars) {
      if (lbl_activity) lbl_activity->SetText(stars->GetLoadActivity());
      if (lbl_progress) lbl_progress->SetValue(stars->GetLoadProgress());
   }
}

// +--------------------------------------------------------------------+

void
CmpLoadDlg::MoveTo(const Rect& r)
{
   FormWindow::MoveTo(r);

   Campaign* campaign = Campaign::GetCampaign();

   if (campaign && img_title && campaign->GetImage(3)) {
      Bitmap* bmp = campaign->GetImage(3);

      Rect tgt_rect;
      tgt_rect.w = img_title->Width();
      tgt_rect.h = img_title->Height();

      img_title->SetTargetRect(tgt_rect);
      img_title->SetPicture(*bmp);
   }
}

// +--------------------------------------------------------------------+

bool
CmpLoadDlg::IsDone()
{
   if (Game::RealTime() - show_time < 5000)
      return false;

   return true;
}

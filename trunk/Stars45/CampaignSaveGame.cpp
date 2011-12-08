/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         CampaignSaveGame.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    CampaignSaveGame contains the logic needed to save and load
    campaign games in progress.
*/

#include "MemDebug.h"
#include "CampaignSaveGame.h"
#include "Campaign.h"
#include "Combatant.h"
#include "CombatAction.h"
#include "CombatEvent.h"
#include "CombatGroup.h"
#include "CombatUnit.h"
#include "CombatZone.h"
#include "Galaxy.h"
#include "Mission.h"
#include "StarSystem.h"
#include "Player.h"

#include "Game.h"
#include "DataLoader.h"
#include "ParseUtil.h"
#include "FormatUtil.h"

static const char* SAVE_DIR = "SaveGame";

// +--------------------------------------------------------------------+

CampaignSaveGame::CampaignSaveGame(Campaign* c)
   : campaign(c)
{ }

// +--------------------------------------------------------------------+

CampaignSaveGame::~CampaignSaveGame()
{ }

// +--------------------------------------------------------------------+

Text
CampaignSaveGame::GetSaveDirectory()
{
   return GetSaveDirectory(Player::GetCurrentPlayer());
}

Text
CampaignSaveGame::GetSaveDirectory(Player* player)
{
   if (player) {
      char save_dir[32];
      sprintf(save_dir, "%s/%02d", SAVE_DIR, player->Identity());

      return save_dir;
   }

   return SAVE_DIR;
}

void
CampaignSaveGame::CreateSaveDirectory()
{
   HANDLE hDir = CreateFile(SAVE_DIR, 0, 0, 0, OPEN_EXISTING, 0, 0);

   if (hDir == INVALID_HANDLE_VALUE)
      CreateDirectory(SAVE_DIR, NULL);
   else
      CloseHandle(hDir);

   hDir = CreateFile(GetSaveDirectory(), 0, 0, 0, OPEN_EXISTING, 0, 0);

   if (hDir == INVALID_HANDLE_VALUE)
      CreateDirectory(GetSaveDirectory(), NULL);
   else
      CloseHandle(hDir);
}

// +--------------------------------------------------------------------+

static char multiline[4096];
static char* FormatMultiLine(const char* s)
{
   int   i = 4095;
   char* p = multiline;

   while (*s && i > 0) {
      if (*s == '\n') {
         *p++ = '\\';
         *p++ = 'n';
         s++;
         i -= 2;
      }
      else if (*s == '"') {
         *p++ = '\\';
         *p++ = '"';
         s++;
         i -= 2;
      }
      else {
         *p++ = *s++;
         i--;
      }
   }

   *p = 0;
   return multiline;
}

static char* ParseMultiLine(const char* s)
{
   int   i = 4095;
   char* p = multiline;

   while (*s && i > 0) {
      if (*s == '\\') {
         s++;
         if (*s == 'n') {
            *p++ = '\n';
            *s++;
            i--;
         }
         else if (*s == '"') {
            *p++ = '"';
            *s++;
            i--;
         }
         else {
            *p++ = *s++;
            i--;
         }
      }
      else {
         *p++ = *s++;
         i--;
      }
   }

   *p = 0;
   return multiline;
}

void
CampaignSaveGame::Load(const char* filename)
{
   Print("-------------------------\nLOADING SAVEGAME (%s).\n", filename);
   campaign = 0;

   if (!filename || !filename[0]) return;

   DataLoader* loader = DataLoader::GetLoader();
   bool use_file_sys = loader->IsFileSystemEnabled();
   loader->UseFileSystem(true);
   loader->SetDataPath(GetSaveDirectory() + "/");

   BYTE* block;
   loader->LoadBuffer(filename, block, true);
   loader->UseFileSystem(use_file_sys);

   Sleep(10);

   Parser parser(new(__FILE__,__LINE__) BlockReader((const char*) block));
   Term*  term = parser.ParseTerm();
   
   if (!term) {
      Print("ERROR: could not parse save game '%s'\n", filename);
      loader->SetDataPath(0);
      return;
   }
   else {
      TermText* file_type = term->isText();
      if (!file_type || file_type->value() != "SAVEGAME") {
         Print("ERROR: invalid save game file '%s'\n", filename);
         term->print(10);
         loader->SetDataPath(0);
         delete term;
         return;
      }
   }

   int    grp_iff  = 0;
   int    grp_type = 0;
   int    grp_id   = 0;
   int    status   = 0;
   double baseTime = 0;
   double time     = 0;
   Text   unit;
   Text   sitrep;
   Text   orders;

   do {
      Sleep(5);
      delete term; term = 0;
      term = parser.ParseTerm();
      
      if (term) {
         TermDef* def = term->isDef();
         if (def) {
            if (def->name()->value() == "campaign") {
               Text cname;
               int  cid=0;

               if (def->term()) {
                  if (def->term()->isText())
                     cname = def->term()->isText()->value();
                  else if (def->term()->isNumber())
                     cid   = (int) def->term()->isNumber()->value();
               }

               if (!campaign) {
                  List<Campaign>& list = Campaign::GetAllCampaigns();

                  for (int i = 0; i < list.size() && !campaign; i++) {
                     Campaign* c = list.at(i);

                     if (cname == c->Name() || cid == c->GetCampaignId()) {
                        campaign = c;
                        campaign->Load();
                        campaign->Prep(); // restore campaign to pristine state

                        loader->SetDataPath(GetSaveDirectory() + "/");
                     }
                  }
               }
            }

            else if (def->name()->value() == "grp_iff") {
               GetDefNumber(grp_iff, def, filename);
            }

            else if (def->name()->value() == "grp_type") {
               GetDefNumber(grp_type, def, filename);
            }

            else if (def->name()->value() == "grp_id") {
               GetDefNumber(grp_id, def, filename);
            }

            else if (def->name()->value() == "unit") {
               GetDefText(unit, def, filename);
            }

            else if (def->name()->value() == "status") {
               GetDefNumber(status, def, filename);
            }

            else if (def->name()->value() == "basetime") {
               GetDefNumber(baseTime, def, filename);
            }

            else if (def->name()->value() == "time") {
               GetDefNumber(time, def, filename);
            }

            else if (def->name()->value() == "sitrep") {
               GetDefText(sitrep, def, filename);
            }

            else if (def->name()->value() == "orders") {
               GetDefText(orders, def, filename);
            }

            else if (def->name()->value() == "combatant") {
               if (!def->term() || !def->term()->isStruct()) {
                  ::Print("WARNING: combatant struct missing in '%s/%s'\n", loader->GetDataPath(), filename);
               }
               else {
                  TermStruct* val = def->term()->isStruct();

                  char  name[64];
                  int   iff   = 0;
                  int   score = 0;

                  ZeroMemory(name,  sizeof(name));

                  for (int i = 0; i < val->elements()->size(); i++) {
                     TermDef* pdef = val->elements()->at(i)->isDef();
                     if (pdef) {
                        if (pdef->name()->value() == "name")
                           GetDefText(name, pdef, filename);

                        else if (pdef->name()->value() == "iff") {
                           GetDefNumber(iff, pdef, filename);
                        }

                        else if (pdef->name()->value() == "score") {
                           GetDefNumber(score, pdef, filename);
                        }
                     }
                  }

                  if (campaign && name[0]) {
                     Combatant* combatant = campaign->GetCombatant(name);

                     if (combatant) {
                        CombatGroup::MergeOrderOfBattle(block, filename, iff, combatant, campaign);
                        combatant->SetScore(score);
                     }
                     else {
                        ::Print("WARNING: could not find combatant '%s' in campaign.\n", name);
                     }
                  }
               }
            }
            else if (def->name()->value() == "event") {
               if (!def->term() || !def->term()->isStruct()) {
                  ::Print("WARNING: event struct missing in '%s/%s'\n", loader->GetDataPath(), filename);
               }
               else {
                  TermStruct* val = def->term()->isStruct();

                  char  type[64];
                  char  source[64];
                  char  region[64];
                  char  title[256];
                  char  file[256];
                  char  image[256];
                  char  scene[256];
                  char  info[4096];
                  int   time = 0;
                  int   team = 0;
                  int   points = 0;

                  type[0]   = 0;
                  info[0]   = 0;
                  file[0]   = 0;
                  image[0]  = 0;
                  scene[0]  = 0;
                  title[0]  = 0;
                  region[0] = 0;
                  source[0] = 0;

                  for (int i = 0; i < val->elements()->size(); i++) {
                     TermDef* pdef = val->elements()->at(i)->isDef();
                     if (pdef) {
                        if (pdef->name()->value() == "type")
                           GetDefText(type, pdef, filename);

                        else if (pdef->name()->value() == "source")
                           GetDefText(source, pdef, filename);

                        else if (pdef->name()->value() == "region")
                           GetDefText(region, pdef, filename);

                        else if (pdef->name()->value() == "title")
                           GetDefText(title, pdef, filename);

                        else if (pdef->name()->value() == "file")
                           GetDefText(file, pdef, filename);

                        else if (pdef->name()->value() == "image")
                           GetDefText(image, pdef, filename);

                        else if (pdef->name()->value() == "scene")
                           GetDefText(scene, pdef, filename);

                        else if (pdef->name()->value() == "info")
                           GetDefText(info, pdef, filename);

                        else if (pdef->name()->value() == "time")
                           GetDefNumber(time, pdef, filename);

                        else if (pdef->name()->value() == "team")
                           GetDefNumber(team, pdef, filename);

                        else if (pdef->name()->value() == "points")
                           GetDefNumber(points, pdef, filename);
                     }
                  }

                  if (campaign && type[0]) {
                     loader->SetDataPath(campaign->Path());

                     CombatEvent* event = new(__FILE__,__LINE__)
                        CombatEvent(campaign,
                                    CombatEvent::TypeFromName(type),
                                    time,
                                    team,
                                    CombatEvent::SourceFromName(source),
                                    region);

                     if (event) {
                        event->SetTitle(title);
                        event->SetFilename(file);
                        event->SetImageFile(image);
                        event->SetSceneFile(scene);
                        event->Load();

                        if (info[0])
                           event->SetInformation(ParseMultiLine(info));

                        event->SetVisited(true);
                        campaign->GetEvents().append(event);
                     }
                  }
               }
            }
            else if (def->name()->value() == "action") {
               if (!def->term() || !def->term()->isStruct()) {
                  ::Print("WARNING: action struct missing in '%s/%s'\n", loader->GetDataPath(), filename);
               }
               else {
                  TermStruct* val = def->term()->isStruct();

                  int   id    = -1;
                  int   stat  = 0;
                  int   count = 0;
                  int   after = 0;

                  for (int i = 0; i < val->elements()->size(); i++) {
                     TermDef* pdef = val->elements()->at(i)->isDef();
                     if (pdef) {
                        if (pdef->name()->value() == "id")
                           GetDefNumber(id, pdef, filename);

                        else if (pdef->name()->value() == "stat")
                           GetDefNumber(stat, pdef, filename);

                        else if (pdef->name()->value() == "count")
                           GetDefNumber(count, pdef, filename);

                        else if (pdef->name()->value().contains("after"))
                           GetDefNumber(after, pdef, filename);
                     }
                  }

                  if (campaign && id >= 0) {
                     ListIter<CombatAction> a_iter = campaign->GetActions();
                     while (++a_iter) {
                        CombatAction* a = a_iter.value();
                        if (a->Identity() == id) {
                           a->SetStatus(stat);

                           if (count)
                              a->SetCount(count);

                           if (after)
                              a->SetStartAfter(after);

                           break;
                        }
                     }
                  }
               }
            }
         }
      }
   }
   while (term);

   if (term) {
      delete term;
      term = 0;
   }

   if (campaign) {
      campaign->SetSaveGame(true);

      List<Campaign>& list = Campaign::GetAllCampaigns();

      if (status < Campaign::CAMPAIGN_SUCCESS) {
         campaign->SetStatus(status);
         if (sitrep.length()) campaign->SetSituation(sitrep);
         if (orders.length()) campaign->SetOrders(orders);
         campaign->SetStartTime(baseTime);
         campaign->SetLoadTime(baseTime + time);
         campaign->LockoutEvents(3600);
         campaign->Start();

         if (grp_type >= CombatGroup::FLEET && grp_type <= CombatGroup::PRIVATE) {
            CombatGroup* player_group = campaign->FindGroup(grp_iff, grp_type, grp_id);
            if (player_group) {
               CombatUnit* player_unit = 0;
               
               if (unit.length())
                  player_unit = player_group->FindUnit(unit);

               if (player_unit)
                  campaign->SetPlayerUnit(player_unit);
               else
                  campaign->SetPlayerGroup(player_group);
            }
         }
      }

      // failed - restart current campaign:
      else if (status == Campaign::CAMPAIGN_FAILED) {
         Print("CampaignSaveGame: Loading FAILED campaign, restarting '%s'\n",
                  campaign->Name());

         campaign->Load();
         campaign->Prep(); // restore campaign to pristine state
         campaign->SetSaveGame(false);

         loader->SetDataPath(GetSaveDirectory() + "/");
      }

      // start next campaign:
      else if (status == Campaign::CAMPAIGN_SUCCESS) {
         Print("CampaignSaveGame: Loading COMPLETED campaign '%s', searching for next campaign...\n",
                  campaign->Name());

         bool found = false;

         for (int i = 0; i < list.size() && !found; i++) {
            Campaign* c = list.at(i);

            if (c->GetCampaignId() == campaign->GetCampaignId()+1) {
               campaign = c;
               campaign->Load();
               campaign->Prep(); // restore campaign to pristine state

               Print("Advanced to campaign %d '%s'\n",
                        campaign->GetCampaignId(),
                        campaign->Name());

               loader->SetDataPath(GetSaveDirectory() + "/");
               found = true;
            }
         }

         // if no next campaign found, start over from the beginning:
         for (int i = 0; i < list.size() && !found; i++) {
            Campaign* c = list.at(i);

            if (c->IsDynamic()) {
               campaign = c;
               campaign->Load();
               campaign->Prep(); // restore campaign to pristine state

               Print("Completed full series, restarting at %d '%s'\n",
                        campaign->GetCampaignId(),
                        campaign->Name());

               loader->SetDataPath(GetSaveDirectory() + "/");
               found = true;
            }
         }
      }
   }

   loader->ReleaseBuffer(block);
   loader->SetDataPath(0);
   Print("SAVEGAME LOADED (%s).\n\n", filename);
}

// +--------------------------------------------------------------------+

void
CampaignSaveGame::Save(const char* name)
{
   if (!campaign) return;

   CreateSaveDirectory();

   Text s = GetSaveDirectory() + Text("/") + Text(name);

   FILE* f = fopen(s, "w");
   if (f) {
      char timestr[32];
      FormatDayTime(timestr, campaign->GetTime());

      CombatGroup* player_group = campaign->GetPlayerGroup();
      CombatUnit*  player_unit  = campaign->GetPlayerUnit();

      fprintf(f, "SAVEGAME\n\n");
      fprintf(f, "campaign: \"%s\"\n\n",  campaign->Name());
      fprintf(f, "grp_iff:  %d\n",        (int) player_group->GetIFF());
      fprintf(f, "grp_type: %d\n",        (int) player_group->Type());
      fprintf(f, "grp_id:   %d\n",        (int) player_group->GetID());
      if (player_unit)
         fprintf(f, "unit:     \"%s\"\n", player_unit->Name().data());

      fprintf(f, "status:   %d\n",        (int) campaign->GetStatus());
      fprintf(f, "basetime: %f\n",        campaign->GetStartTime());
      fprintf(f, "time:     %f // %s\n\n",
                                          campaign->GetTime(),
                                          timestr);

      fprintf(f, "sitrep:   \"%s\"\n",    campaign->Situation());
      fprintf(f, "orders:   \"%s\"\n\n",  campaign->Orders());

      ListIter<Combatant> c_iter = campaign->GetCombatants();
      while (++c_iter) {
         Combatant*   c = c_iter.value();

         fprintf(f, "combatant: {");
         fprintf(f, " name:\"%s\",", c->Name());
         fprintf(f, " iff:%d,", c->GetIFF());
         fprintf(f, " score:%d,", c->Score());
         fprintf(f, " }\n");
      }

      fprintf(f, "\n");

      ListIter<CombatAction> a_iter = campaign->GetActions();
      while (++a_iter) {
         CombatAction* a = a_iter.value();
         fprintf(f, "action: { id:%4d, stat:%d", a->Identity(), a->Status());
         
         if (a->Status() == CombatAction::PENDING) {
            if (a->Count())
               fprintf(f, ", count:%d", a->Count());

            if (a->StartAfter())
               fprintf(f, ", after:%d", a->StartAfter());
         }

         fprintf(f, " }\n");
      }

      fprintf(f, "\n");

      ListIter<CombatEvent> e_iter = campaign->GetEvents();
      while (++e_iter) {
         CombatEvent* e = e_iter.value();

         fprintf(f, "event: {");
         fprintf(f, " type:%-18s,",      e->TypeName());
         fprintf(f, " time:0x%08x,",     e->Time());
         fprintf(f, " team:%d,",         e->GetIFF());
         fprintf(f, " points:%d,",       e->Points());
         fprintf(f, " source:\"%s\",",   e->SourceName());
         fprintf(f, " region:\"%s\",",   e->Region());
         fprintf(f, " title:\"%s\",",    e->Title());
         if (e->Filename())
         fprintf(f, " file:\"%s\",",     e->Filename());
         if (e->ImageFile())
         fprintf(f, " image:\"%s\",",    e->ImageFile());
         if (e->SceneFile())
         fprintf(f, " scene:\"%s\",",    e->SceneFile());
         if (!e->Filename() || *e->Filename() == 0)
         fprintf(f, " info:\"%s\"",      FormatMultiLine(e->Information()));
         fprintf(f, " }\n");
      }

      fprintf(f, "\n// ORDER OF BATTLE:\n\n");
      fclose(f);

      c_iter.reset();
      while (++c_iter) {
         Combatant*   c = c_iter.value();
         CombatGroup* g = c->GetForce();
         CombatGroup::SaveOrderOfBattle(s, g);
      }
   }
}

void
CampaignSaveGame::Delete(const char* name)
{
   DeleteFile(GetSaveDirectory() + "/" + name);
}

void
CampaignSaveGame::RemovePlayer(Player* p)
{
   List<Text> save_list;
   Text       save_dir = GetSaveDirectory(p) + "/";

   DataLoader* loader = DataLoader::GetLoader();
   bool use_file_sys = loader->IsFileSystemEnabled();
   loader->UseFileSystem(true);
   loader->SetDataPath(save_dir);
   loader->ListFiles("*.*", save_list);
   loader->SetDataPath(0);
   loader->UseFileSystem(use_file_sys);

   for (int i = 0; i < save_list.size(); i++) {
      Text* filename = save_list[i];
      DeleteFile(save_dir + filename->data());
   }

   save_list.destroy();

   RemoveDirectory(GetSaveDirectory(p));
}

// +--------------------------------------------------------------------+

void
CampaignSaveGame::SaveAuto()
{
   Save("AutoSave");
}

void
CampaignSaveGame::LoadAuto()
{
   Load("AutoSave");
}

// +--------------------------------------------------------------------+

Text
CampaignSaveGame::GetResumeFile()
{
   // check for auto save game:
   FILE* f = ::fopen(GetSaveDirectory() + "/AutoSave", "r");
   if (f) {
      ::fclose(f);

      return "AutoSave";
   }

   return Text();
}

int
CampaignSaveGame::GetSaveGameList(List<Text>& save_list)
{
   DataLoader* loader = DataLoader::GetLoader();
   bool use_file_sys = loader->IsFileSystemEnabled();
   loader->UseFileSystem(true);
   loader->SetDataPath(GetSaveDirectory() + "/");
   loader->ListFiles("*.*", save_list);
   loader->SetDataPath(0);
   loader->UseFileSystem(use_file_sys);

   return save_list.size();
}
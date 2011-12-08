/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         Player.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Player / Logbook class
*/


#include "MemDebug.h"
#include "Player.h"
#include "NetLobbyServer.h"
#include "NetLayer.h"
#include "Ship.h"
#include "SimEvent.h"
#include "Campaign.h"
#include "CampaignSaveGame.h"
#include "Random.h"
#include "HUDView.h"
#include "MFD.h"

#include "DataLoader.h"
#include "Encrypt.h"
#include "ParseUtil.h"
#include "FormatUtil.h"
#include "Bitmap.h"
#include "Game.h"

// +-------------------------------------------------------------------+

class AwardInfo
{
public:
   static const char* TYPENAME() { return "AwardInfo"; }

   AwardInfo() : 
      type(RANK), 
      id(0),
      large_insignia(0),
      small_insignia(0),
      granted_ship_classes(0x7),
      total_points(0),
      mission_points(0),
      total_missions(0),
      kills(0),
      lost(0),
      collision(0),
      campaign_id(0),
      campaign_complete(false),
      dynamic_campaign(false),
      ceremony(true),
      required_awards(0),
      lottery(0),
      min_rank(0),
      max_rank((int) 1e9),
      min_ship_class(0),
      max_ship_class((int) 1e9)
   { }
   ~AwardInfo() { }

   enum TYPE { RANK, MEDAL };

   int      type;
   int      id;
   Text     name;
   Text     abrv;
   Text     desc;
   Text     grant;
   Text     desc_sound;
   Text     grant_sound;
   Bitmap*  large_insignia;
   Bitmap*  small_insignia;
   int      granted_ship_classes;

   int      total_points;
   int      mission_points;
   int      total_missions;
   int      kills;
   int      lost;
   int      collision;
   int      campaign_id;
   bool     campaign_complete;
   bool     dynamic_campaign;
   bool     ceremony;

   int      required_awards;
   int      lottery;
   int      min_rank;
   int      max_rank;
   int      min_ship_class;
   int      max_ship_class;
};

static List<AwardInfo> rank_table;
static List<AwardInfo> medal_table;
static bool            config_exists = false;

// +-------------------------------------------------------------------+

Player::Player(const char* n)
   : uid(0), name(n), create_date(0), points(0), medals(0), flight_time(0),
     missions(0), kills(0), losses(0), campaigns(0), trained(0),
     flight_model(0), flying_start(0), landing_model(0), 
     ai_level(1), hud_mode(0), hud_color(1),
     ff_level(4), grid(1), gunsight(0), award(0)
{
   name.setSensitive(false);

   mfd[0] = -1;
   mfd[1] = -1;
   mfd[2] = -1;
   mfd[3] = -1;
}

Player::Player()
   : uid(0), create_date(0), points(0), medals(0), flight_time(0),
     missions(0), kills(0), losses(0), campaigns(0), trained(0),
     flight_model(0), flying_start(0), landing_model(0), 
     ai_level(1), hud_mode(0), hud_color(1),
     ff_level(4), grid(1), gunsight(0), award(0)
{
   name.setSensitive(false);

   mfd[0] = -1;
   mfd[1] = -1;
   mfd[2] = -1;
   mfd[3] = -1;
}

Player::~Player()
{ }

// +-------------------------------------------------------------------+

void
Player::SetName(const char* n)
{
   if (n && *n)
      name = n;
}

void
Player::SetPassword(const char* p)
{
   if (p && *p) {
      pass = p;

      if (pass.length() > 16)
         pass = pass.substring(0, 16);
   }
}

void
Player::SetSquadron(const char* s)
{
   if (s && *s)
      squadron = s;
}

void
Player::SetSignature(const char* s)
{
   if (s && *s)
      signature = s;
}

const Text&
Player::ChatMacro(int n) const
{
   if (n >= 0 && n < 10)
      return chat_macros[n];

   return chat_macros[0];
}

void
Player::SetChatMacro(int n, const char* m)
{
   if (n >= 0 && n < 10 && m && *m)
      chat_macros[n] = m;
}

void
Player::SetPoints(int p)
{
   if (p >= 0)
      points = p;
}

void
Player::SetMedals(int m)
{
   medals = m;
}

void
Player::SetCampaigns(int n)
{
   campaigns = n;
}

void
Player::SetTrained(int n)
{
   if (n == 0)
      trained = 0;

   else if (n > 0 && n <= 20)
      trained = trained | (1 << (n-1));

   else if (n > 20)
      trained = n;
}

bool
Player::HasTrained(int n) const
{
   if (n > 0 && n <= 20)
      return (trained & (1 << (n-1))) ? true : false;

   return false;
}

bool
Player::HasCompletedCampaign(int id) const
{
   if (id > 0 && id < 30)
      return (campaigns & (1 << id)) ? true : false;

   return false;
}

void
Player::SetCampaignComplete(int id)
{
   if (id > 0 && id < 30) {
      campaigns = campaigns | (1 << id);
      Save();
   }
}

void
Player::SetCreateDate(int d)
{
   if (d >= 0)
      create_date = d;
}

void
Player::SetFlightTime(int t)
{
   if (t >= 0)
      flight_time = t;
}

void
Player::SetMissions(int m)
{
   if (m >= 0)
      missions = m;
}

void
Player::SetKills(int k)
{
   if (k >= 0)
      kills = k;
}

void
Player::SetLosses(int l)
{
   if (l >= 0)
      losses = l;
}

// +-------------------------------------------------------------------+

void
Player::AddFlightTime(int t)
{
   if (t > 0)
      flight_time += t;
}

void
Player::AddPoints(int p)
{
   if (p > 0)
      points += p;
}

void
Player::AddMissions(int m)
{
   if (m > 0)
      missions += m;
}

void
Player::AddKills(int k)
{
   if (k > 0)
      kills += k;
}

void
Player::AddLosses(int l)
{
   if (l > 0)
      losses += l;
}

// +-------------------------------------------------------------------+

void
Player::SetFlightModel(int n)
{
   if (n >= Ship::FM_STANDARD && n <= Ship::FM_ARCADE) {
      flight_model = n;
      Ship::SetFlightModel(n);
   }
}

void
Player::SetFlyingStart(int n)
{
   flying_start = n;
}

void
Player::SetLandingModel(int n)
{
   if (n >= Ship::LM_STANDARD && n <= Ship::LM_EASIER) {
      landing_model = n;
      Ship::SetLandingModel(landing_model);
   }
}

void
Player::SetAILevel(int n)
{
   ai_level = n;
}

void
Player::SetHUDMode(int n)
{
   hud_mode = n;
   HUDView::SetArcade(n > 0);
}

void
Player::SetHUDColor(int n)
{
   hud_color = n;
   HUDView::SetDefaultColorSet(n);
}

void
Player::SetFriendlyFire(int n)
{
   if (n >= 0 && n <= 4) {
      ff_level = n;
      Ship::SetFriendlyFireLevel(n/4.0);
   }
}

void
Player::SetGridMode(int n)
{
   if (n >= 0 && n <= 1) {
      grid = n;
   }
}

void
Player::SetGunsight(int n)
{
   if (n >= 0 && n <= 1) {
      gunsight = n;
   }
}

void
Player::ClearShowAward()
{
   award = 0;
}

Text
Player::AwardName() const
{
   if (award)
      return award->name;

   return Text();
}

Text
Player::AwardDesc() const
{
   if (award)
      return award->grant;

   return Text();
}

Bitmap*
Player::AwardImage() const
{
   if (award)
      return award->large_insignia;

   return 0;
}

Sound*
Player::AwardSound() const
{
   if (award && award->grant_sound.length()) {
      DataLoader* loader = DataLoader::GetLoader();
      Sound*      result = 0;

      loader->LoadSound(award->grant_sound, result);
      return result;
   }

   return 0;
}

// +-------------------------------------------------------------------+

const char*
Player::RankName(int rank)
{
   ListIter<AwardInfo> iter = rank_table;
   while (++iter) {
      AwardInfo* award = iter.value();
      if (award->id == rank)
         return award->name;
   }

   return "Conscript";
}

const char*
Player::RankAbrv(int rank)
{
   ListIter<AwardInfo> iter = rank_table;
   while (++iter) {
      AwardInfo* award = iter.value();
      if (award->id == rank)
         return award->abrv;
   }

   return "";
}

Bitmap*
Player::RankInsignia(int rank, int size)
{
   ListIter<AwardInfo> iter = rank_table;
   while (++iter) {
      AwardInfo* award = iter.value();
      if (award->id == rank) {
         if (size == 0)
            return award->small_insignia;

         if (size == 1)
            return award->large_insignia;
      }
   }

   return 0;
}

const char*
Player::RankDescription(int rank)
{
   ListIter<AwardInfo> iter = rank_table;
   while (++iter) {
      AwardInfo* award = iter.value();
      if (award->id == rank)
         return award->desc;
   }

   return "";
}

int
Player::RankFromName(const char* name)
{
   ListIter<AwardInfo> iter = rank_table;
   while (++iter) {
      AwardInfo* award = iter.value();
      if (award->name == name)
         return award->id;
   }

   return 0;
}

int
Player::Rank() const
{
   for (int i = rank_table.size()-1; i >= 0; i--) {
      AwardInfo* award = rank_table[i];
      if (points >= award->total_points)
         return award->id;
   }

   return 0;
}

void
Player::SetRank(int r)
{
   ListIter<AwardInfo> iter = rank_table;
   while (++iter) {
      AwardInfo* award = iter.value();
      if (r == award->id)
         points = award->total_points;
   }
}

int
Player::Medal(int n) const
{
   if (n < 0)
      return 0;

   for (int i = 0; i < 16; i++) {
      int selector = 1 << (15-i);

      // found a medal:
      if (medals & selector) {

         // and it's the nth medal!
         if (n == 0) {
            return selector;
         }

         n--;
      }
   }

   return 0;
}

// +-------------------------------------------------------------------+

const char*
Player::MedalName(int medal)
{
   ListIter<AwardInfo> iter = medal_table;
   while (++iter) {
      AwardInfo* award = iter.value();
      if (award->id == medal)
         return award->name;
   }

   return "";
}

Bitmap*
Player::MedalInsignia(int medal, int size)
{
   ListIter<AwardInfo> iter = medal_table;
   while (++iter) {
      AwardInfo* award = iter.value();
      if (award->id == medal) {
         if (size == 0)
            return award->small_insignia;

         if (size == 1)
            return award->large_insignia;
      }
   }

   return 0;
}

const char*
Player::MedalDescription(int medal)
{
   ListIter<AwardInfo> iter = medal_table;
   while (++iter) {
      AwardInfo* award = iter.value();
      if (award->id == medal)
         return award->desc;
   }

   return "";
}

// +-------------------------------------------------------------------+

bool
Player::CanCommand(int ship_class)
{
   if (ship_class <= Ship::ATTACK)
      return true;

   for (int i = rank_table.size()-1; i >= 0; i--) {
      AwardInfo* award = rank_table[i];
      if (points > award->total_points) {
         return (ship_class & award->granted_ship_classes) != 0;
      }
   }

   return false;
}

int
Player::CommandRankRequired(int ship_class)
{
   for (int i = 0; i < rank_table.size(); i++) {
      AwardInfo* award = rank_table[i];
      if ((ship_class & award->granted_ship_classes) != 0) {
         return i;
      }
   }

   return rank_table.size()-1;
}

// +-------------------------------------------------------------------+

int
Player::GetMissionPoints(ShipStats* s, DWORD start_time)
{
   int result = 0;

   if (s) {
      result = s->GetPoints();

      int flight_time = (Game::GameTime() - start_time) / 1000;

      // if player survived mission, award one experience point
      // for each minute of action, in ten point blocks:
      if (!s->GetDeaths() && !s->GetColls()) {
         int minutes = flight_time / 60;
         minutes /= 10;
         minutes *= 10;
         result += minutes;

         if (s->HasEvent(SimEvent::DOCK))
            result += 100;
      }
      else {
         result -= (int) (2.5 * Ship::Value(s->GetShipClass()));
      }

      if (result < 0)
         result = 0;
   }

   return result;
}

// +-------------------------------------------------------------------+

void
Player::ProcessStats(ShipStats* s, DWORD start_time)
{
   if (!s) return;

   int old_rank = Rank();
   int pts      = GetMissionPoints(s, start_time);

   AddPoints(pts);
   AddPoints(s->GetCommandPoints());
   AddKills(s->GetGunKills());
   AddKills(s->GetMissileKills());
   AddLosses(s->GetDeaths());
   AddLosses(s->GetColls());
   AddMissions(1);
   AddFlightTime((Game::GameTime() - start_time) / 1000);

   int rank = Rank();

   // did the player earn a promotion?
   if (old_rank != rank) {
      ListIter<AwardInfo> iter = rank_table;
      while (++iter) {
         AwardInfo* a = iter.value();
         if (rank == a->id) {
            award = a;
         }
      }
   }

   // if not, did the player earn a medal?
   else {
      ListIter<AwardInfo> iter = medal_table;
      while (++iter) {
         AwardInfo* a = iter.value();

         if (EarnedAward(a, s) && a->ceremony) {
            award = a;
            break;
         }
      }
   }

   // persist all stats, promotions, and medals:
   Save();
}

bool
Player::EarnedAward(AwardInfo* a, ShipStats* s)
{
   if (!a || !s)
      return false;

   // already earned this medal?
   if (a->id & medals)
      return false;

   // eligible for this medal?
   int rank = Rank();
   if (a->min_rank > rank || a->max_rank < rank)
      return false;

   if ((a->required_awards & medals) < a->required_awards)
      return false;

   if (a->min_ship_class > s->GetShipClass() || a->max_ship_class < s->GetShipClass())
      return false;

   if (a->total_points > points)
      return false;

   if (a->total_missions > missions)
      return false;

   if (a->campaign_id && a->campaign_complete) {
      if (!HasCompletedCampaign(a->campaign_id))
         return false;
   }

   else {
      // campaign related requirements
      Campaign* c = Campaign::GetCampaign();

      if (c) {
         if (a->dynamic_campaign && !c->IsDynamic())
            return false;
      }
   }

   // sufficient merit for this medal?
   if (a->mission_points > s->GetPoints())
      return false;

   if (a->kills > s->GetGunKills() + s->GetMissileKills())
      return false;

   if (a->mission_points > s->GetPoints())
      return false;

   // player must survive mission if lost = -1
   if (a->lost < 0 && (s->GetDeaths() || s->GetColls()))
      return false;

   // do we need to be wounded in battle?
   if (a->lost > s->GetDeaths() || a->collision > s->GetColls())
      return false;

   // final lottery check:
   if (a->lottery < 2 || RandomChance(1, a->lottery)) {
      medals |= a->id;
      return true;
   }

   // what do we have for the losers, judge?
   return false;
}

// +-------------------------------------------------------------------+

static List<Player>  player_roster;
static Player*       current_player = 0;

List<Player>&
Player::GetRoster()
{
   return player_roster;
}

Player*
Player::GetCurrentPlayer()
{
   return current_player;
}

void
Player::SelectPlayer(Player* p)
{
   HUDView* hud = HUDView::GetInstance();

   if (current_player && current_player != p) {
      if (hud) {
         for (int i = 0; i < 3; i++) {
            MFD* mfd = hud->GetMFD(i);

            if (mfd)
               current_player->mfd[i] = mfd->GetMode();
         }
      }
   }

   if (player_roster.contains(p)) {
      current_player = p;

      Ship::SetFlightModel(p->flight_model);
      Ship::SetLandingModel(p->landing_model);
      HUDView::SetArcade(p->hud_mode > 0);
      HUDView::SetDefaultColorSet(p->hud_color);

      if (hud) {
         for (int i = 0; i < 3; i++) {
            if (p->mfd[i] >= 0) {
               MFD* mfd = hud->GetMFD(i);

               if (mfd)
                  mfd->SetMode(p->mfd[i]);
            }
         }
      }
   }
}

Player*
Player::Find(const char* name)
{
   for (int i = 0; i < player_roster.size(); i++) {
      Player* p = player_roster.at(i);
      if (p->Name() == name)
         return p;
   }

   return 0;
}

Player*
Player::Create(const char* name)
{
   if (name && *name) {
      // check for existence:
      if (Find(name))
         return 0;

      Player* newbie = new(__FILE__,__LINE__) Player(name);
      newbie->SetCreateDate(NetLayer::GetUTC());

      player_roster.append(newbie);
      newbie->CreateUniqueID();
      return newbie;
   }

   return 0;
}

void
Player::Destroy(Player* p)
{
   if (p) {
      player_roster.remove(p);

      if (p == current_player) {
         current_player = 0;

         if (player_roster.size())
            current_player = player_roster.at(0);
      }

      CampaignSaveGame::RemovePlayer(p);
      delete p;
   }
}

// +-------------------------------------------------------------------+

void
Player::Initialize()
{
   LoadAwardTables();
   Load();

   if (!current_player) {
      if (!player_roster.size()) {
         Create("Pilot");
      }

      SelectPlayer(player_roster.at(0));
   }
}

void
Player::Close()
{
   if (current_player && !player_roster.contains(current_player))
      delete current_player;

   player_roster.destroy();
   current_player = 0;

   rank_table.destroy();
   medal_table.destroy();
}

// +-------------------------------------------------------------------+

bool
Player::ConfigExists()
{
   return config_exists;
}

// +-------------------------------------------------------------------+

#define GET_DEF_BOOL(x) if(pdef->name()->value()==(#x))GetDefBool(player->x,pdef,filename)
#define GET_DEF_TEXT(x) if(pdef->name()->value()==(#x))GetDefText(player->x,pdef,filename)
#define GET_DEF_NUM(x)  if(pdef->name()->value()==(#x))GetDefNumber(player->x,pdef,filename)

void
Player::Load()
{
   config_exists = false;

   // read the config file:
   BYTE*       block    = 0;
   int         blocklen = 0;

   char        filename[64];
   strcpy(filename, "player.cfg");

   FILE* f = ::fopen(filename, "rb");

   if (f) {
      config_exists = true;

      ::fseek(f, 0, SEEK_END);
      blocklen = ftell(f);
      ::fseek(f, 0, SEEK_SET);

      block = new(__FILE__,__LINE__) BYTE[blocklen+1];
      block[blocklen] = 0;

      ::fread(block, blocklen, 1, f);
      ::fclose(f);
   }

   if (blocklen == 0)
      return;

   Parser parser(new(__FILE__,__LINE__) BlockReader((const char*) block, blocklen));
   Term*  term = parser.ParseTerm();

   if (!term) {
      Print("ERROR: could not parse '%s'.\n", filename);
      return;
   }
   else {
      TermText* file_type = term->isText();
      if (!file_type || file_type->value() != "PLAYER_CONFIG") {
         Print("WARNING: invalid '%s' file.  Using defaults\n", filename);
         return;
      }
   }

   if (current_player && !player_roster.contains(current_player))
      delete current_player;
   player_roster.destroy();
   current_player = 0;

   do {
      delete term;

      term = parser.ParseTerm();

      if (term) {
         TermDef* def = term->isDef();
         if (def) {
            if (def->name()->value() == "player") {

               if (!def->term() || !def->term()->isStruct()) {
                  Print("WARNING: player structure missing in '%s'\n", filename);
               }
               else {
                  Player*     player  = new(__FILE__,__LINE__) Player;
                  bool        current = false;
                  TermStruct* val     = def->term()->isStruct();

                  for (int i = 0; i < val->elements()->size(); i++) {
                     TermDef* pdef = val->elements()->at(i)->isDef();
                     if (pdef) {
                              GET_DEF_TEXT(name);
                        else  GET_DEF_TEXT(squadron);
                        else  GET_DEF_TEXT(signature);

                        else  GET_DEF_NUM(uid);
                        else  GET_DEF_NUM(flight_model);
                        else  GET_DEF_NUM(flying_start);
                        else  GET_DEF_NUM(landing_model);
                        else  GET_DEF_NUM(ai_level);
                        else  GET_DEF_NUM(hud_mode);
                        else  GET_DEF_NUM(hud_color);
                        else  GET_DEF_NUM(ff_level);
                        else  GET_DEF_NUM(grid);
                        else  GET_DEF_NUM(gunsight);

                        else if (pdef->name()->value() == ("chat_0"))
                           GetDefText(player->chat_macros[0], pdef, filename);

                        else if (pdef->name()->value() == ("chat_1"))
                           GetDefText(player->chat_macros[1], pdef, filename);

                        else if (pdef->name()->value() == ("chat_2"))
                           GetDefText(player->chat_macros[2], pdef, filename);

                        else if (pdef->name()->value() == ("chat_3"))
                           GetDefText(player->chat_macros[3], pdef, filename);

                        else if (pdef->name()->value() == ("chat_4"))
                           GetDefText(player->chat_macros[4], pdef, filename);

                        else if (pdef->name()->value() == ("chat_5"))
                           GetDefText(player->chat_macros[5], pdef, filename);

                        else if (pdef->name()->value() == ("chat_6"))
                           GetDefText(player->chat_macros[6], pdef, filename);

                        else if (pdef->name()->value() == ("chat_7"))
                           GetDefText(player->chat_macros[7], pdef, filename);

                        else if (pdef->name()->value() == ("chat_8"))
                           GetDefText(player->chat_macros[8], pdef, filename);

                        else if (pdef->name()->value() == ("chat_9"))
                           GetDefText(player->chat_macros[9], pdef, filename);

                        else if (pdef->name()->value() == ("mfd0"))
                           GetDefNumber(player->mfd[0], pdef, filename);

                        else if (pdef->name()->value() == ("mfd1"))
                           GetDefNumber(player->mfd[1], pdef, filename);

                        else if (pdef->name()->value() == ("mfd2"))
                           GetDefNumber(player->mfd[2], pdef, filename);

                        else if (pdef->name()->value() == ("current"))
                           GetDefBool(current, pdef, filename);

                        else if (pdef->name()->value() == ("trained"))
                           GetDefNumber(player->trained, pdef, filename);

                        else if (pdef->name()->value() == ("stats")) {
                           Text stats;
                           GetDefText(stats, pdef, filename);
                           player->DecodeStats(stats);
                        }

                        else if (pdef->name()->value().indexOf("XXX_CHEAT_A1B2C3_") == 0) {
                           if (pdef->name()->value().contains("points"))
                              GetDefNumber(player->points, pdef, filename);

                           else if (pdef->name()->value().contains("rank")) {
                              int rank=0;
                              GetDefNumber(rank, pdef, filename);
                              player->SetRank(rank);
                           }

                           else if (pdef->name()->value().contains("medals"))
                              GetDefNumber(player->medals, pdef, filename);

                           else if (pdef->name()->value().contains("campaigns"))
                              GetDefNumber(player->campaigns, pdef, filename);

                           else if (pdef->name()->value().contains("missions"))
                              GetDefNumber(player->missions, pdef, filename);

                           else if (pdef->name()->value().contains("kills"))
                              GetDefNumber(player->kills, pdef, filename);

                           else if (pdef->name()->value().contains("losses"))
                              GetDefNumber(player->losses, pdef, filename);

                           else if (pdef->name()->value().contains("flight_time"))
                              GetDefNumber(player->flight_time, pdef, filename);
                        }
                     }
                  }

                  player_roster.append(player);
                  player->CreateUniqueID();

                  if (current)
                     SelectPlayer(player);
               }

            }
            else {
               Print("WARNING: unknown label '%s' in '%s'\n",
                        def->name()->value().data(), filename);
            }
         }
         else {
            Print("WARNING: term ignored in '%s'\n", filename);
            term->print();
         }
      }
   }
   while (term);

   delete [] block;
}

// +-------------------------------------------------------------------+

void
Player::Save()
{
   HUDView* hud = HUDView::GetInstance();
   if (hud && current_player) {
      for (int i = 0; i < 3; i++) {
         MFD* mfd = hud->GetMFD(i);

         if (mfd)
            current_player->mfd[i] = mfd->GetMode();
      }
   }

   FILE* f = fopen("player.cfg", "w");
   if (f) {
      fprintf(f, "PLAYER_CONFIG\n\n");

      ListIter<Player> iter = player_roster;
      while (++iter) {
         Player* p = iter.value();

         fprintf(f, "player: {\n");
         fprintf(f, "   uid:           %d,\n",     p->uid);
         fprintf(f, "   name:          \"%s\",\n", SafeQuotes(p->name.data()));
         fprintf(f, "   squadron:      \"%s\",\n", SafeQuotes(p->squadron.data()));
         fprintf(f, "   signature:     \"%s\",\n", SafeQuotes(p->signature.data()));

         Text stat_data = p->EncodeStats();

         if (stat_data.length() > 32) {
            char tmp[64];
            int  len = stat_data.length();

            for (int n = 0; n < len; n += 32) {
               ZeroMemory(tmp, sizeof(tmp));
               const char* p = stat_data.data() + n;
               strncpy(tmp, p, 32);

               if (n == 0)
                  fprintf(f, "   stats:         \"%s\"\n",  tmp);
               else if (n < len-32)
                  fprintf(f, "                  \"%s\"\n", tmp);
               else
                  fprintf(f, "                  \"%s\",\n", tmp);
            }
         }

         if (p == current_player)
            fprintf(f, "   current:       true,\n");
         else
            fprintf(f, "   current:       false,\n");

         fprintf(f, "   trained:       %d,\n", p->trained);
         fprintf(f, "   flight_model:  %d,\n", p->flight_model);
         fprintf(f, "   flying_start:  %d,\n", p->flying_start);
         fprintf(f, "   landing_model: %d,\n", p->landing_model);
         fprintf(f, "   ai_level:      %d,\n", p->ai_level);
         fprintf(f, "   hud_mode:      %d,\n", p->hud_mode);
         fprintf(f, "   hud_color:     %d,\n", p->hud_color);
         fprintf(f, "   ff_level:      %d,\n", p->ff_level);
         fprintf(f, "   grid:          %d,\n", p->grid);
         fprintf(f, "   gunsight:      %d,\n", p->gunsight);

         for (int i = 0; i < 10; i++) {
            fprintf(f, "   chat_%d:       \"%s\",\n", i, SafeQuotes(p->chat_macros[i].data()));
         }

         for (i = 0; i < 3; i++) {
            if (p->mfd[i] >= 0) {
               fprintf(f, "   mfd%d:         %d,\n", i, p->mfd[i]);
            }
         }

         fprintf(f, "}\n\n");
      }

      fclose(f);

      config_exists = true;
   }
}

// +-------------------------------------------------------------------+

static char stat_buf[280];
static char code_buf[280];

Text
Player::EncodeStats()
{
   ZeroMemory(stat_buf, 280);
   ZeroMemory(code_buf, 280);

   sprintf(stat_buf, "%-16s%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x",
      pass.data(),
      create_date,
      points,
      flight_time,
      missions,
      kills,
      losses,
      medals,
      campaigns,
      11, 12, 13, 14, 15, 16,
      17, 18, 19, 20, 21, 22, 23, 24,
      25, 26, 27, 28, 29, 30, 31, 32);

   for (int i = 0; i < 16; i++)
      for (int j = 0; j < 16; j++)
         code_buf[i*16 + j] = stat_buf[j*16 + i];

   return Encryption::Encode(Encryption::Encrypt(code_buf));
}

void
Player::DecodeStats(const char* stats)
{
   ZeroMemory(stat_buf, 280);
   ZeroMemory(code_buf, 280);

   if (!stats || !*stats) {
      Print("Player::DecodeStats() invalid or missing stats\n");
      create_date = NetLayer::GetUTC();
      return;
   }

   Text plain = Encryption::Decrypt(Encryption::Decode(stats));

   if (plain.length() == 64) {
      for (int i = 0; i < 8; i++)
         for (int j = 0; j < 8; j++)
            stat_buf[j*8 + i] = plain[i*8 +j];
   }

   else if (plain.length() == 256) {
      for (int i = 0; i < 16; i++)
         for (int j = 0; j < 16; j++)
            stat_buf[j*16 + i] = plain[i*16 +j];
   }

   else {
      Print("Player::DecodeStats() invalid plain text length %d\n", plain.length());
      create_date = NetLayer::GetUTC();
      return;
   }

   char work[32];
   ZeroMemory(work, 32);
   CopyMemory(work, stat_buf, 16);
   for (int i = 15; i > 0; i--)
      if (work[i] == ' ') work[i] = 0;
      else break;
   pass = work;

   ZeroMemory(work, 16);
   CopyMemory(work, stat_buf+16, 8);
   sscanf(work, "%x", &create_date);

   ZeroMemory(work, 16);
   CopyMemory(work, stat_buf+24, 8);
   sscanf(work, "%x", &points);
   if (points < 0) points = 0;

   ZeroMemory(work, 16);
   CopyMemory(work, stat_buf+32, 8);
   sscanf(work, "%x", &flight_time);
   if (flight_time < 0) flight_time = 0;

   ZeroMemory(work, 16);
   CopyMemory(work, stat_buf+40, 8);
   sscanf(work, "%x", &missions);
   if (missions < 0) missions = 0;

   ZeroMemory(work, 16);
   CopyMemory(work, stat_buf+48, 8);
   sscanf(work, "%x", &kills);
   if (kills < 0) kills = 0;

   ZeroMemory(work, 16);
   CopyMemory(work, stat_buf+56, 8);
   sscanf(work, "%x", &losses);
   if (losses < 0) losses = 0;

   if (plain.length() > 64) {
      ZeroMemory(work, 16);
      CopyMemory(work, stat_buf+64, 8);
      sscanf(work, "%x", &medals);

      ZeroMemory(work, 16);
      CopyMemory(work, stat_buf+72, 8);
      sscanf(work, "%x", &campaigns);
   }

   if (create_date == 0) {
      ::Print("WARNING - loaded player with zero stats '%s'\n", name.data());
      create_date = NetLayer::GetUTC();
   }
}

// +-------------------------------------------------------------------+

void
Player::LoadAwardTables()
{
   DataLoader* loader   = DataLoader::GetLoader();

   if (!loader) return;

   BYTE*       block    = 0;
   const char* filename = "awards.def";

   loader->SetDataPath("Awards/");
   loader->LoadBuffer(filename, block, true);
   Parser parser(new(__FILE__,__LINE__) BlockReader((const char*) block));

   Term*  term = parser.ParseTerm();

   if (!term) {
      return;
   }
   else {
      TermText* file_type = term->isText();
      if (!file_type || file_type->value() != "AWARDS") {
         return;
      }
   }

   rank_table.destroy();
   medal_table.destroy();

   ::Print("Loading Ranks and Medals\n");

   do {
      delete term; term = 0;
      term = parser.ParseTerm();

      if (term) {
         TermDef* def = term->isDef();
         if (def) {
            if (def->name()->value() == "award") {

               if (!def->term() || !def->term()->isStruct()) {
                  Print("WARNING: award structure missing in '%s'\n", filename);
               }
               else {
                  AwardInfo*  award = new(__FILE__,__LINE__) AwardInfo;
                  TermStruct* val   = def->term()->isStruct();

                  for (int i = 0; i < val->elements()->size(); i++) {
                     TermDef* pdef = val->elements()->at(i)->isDef();
                     if (pdef) {
                        if (pdef->name()->value() == ("name")) {
                           GetDefText(award->name, pdef, filename);
                           award->name = Game::GetText(award->name);
                        }

                        else if (pdef->name()->value() == ("abrv")) {
                           GetDefText(award->abrv, pdef, filename);
                           award->abrv = Game::GetText(award->abrv);
                        }

                        else if (pdef->name()->value() == ("desc")) {
                           GetDefText(award->desc, pdef, filename);
                           if (award->desc.length() <= 40)
                              award->desc = Game::GetText(award->desc);
                        }

                        else if (pdef->name()->value() == ("award")) {
                           GetDefText(award->grant, pdef, filename);
                           if (award->grant.length() <= 40)
                              award->grant = Game::GetText(award->grant);
                        }

                        else if (pdef->name()->value() == ("desc_sound"))
                           GetDefText(award->desc_sound, pdef, filename);

                        else if (pdef->name()->value() == ("award_sound"))
                           GetDefText(award->grant_sound, pdef, filename);

                        else if (pdef->name()->value().indexOf("large") == 0) {
                           Text txt;
                           GetDefText(txt, pdef, filename);
                           txt.setSensitive(false);

                           if (!txt.contains(".pcx"))
                              txt.append(".pcx");

                           loader->CacheBitmap(txt, award->large_insignia);
                        }

                        else if (pdef->name()->value().indexOf("small") == 0) {
                           Text txt;
                           GetDefText(txt, pdef, filename);
                           txt.setSensitive(false);

                           if (!txt.contains(".pcx"))
                              txt.append(".pcx");

                           loader->CacheBitmap(txt, award->small_insignia);

                           if (award->small_insignia)
                              award->small_insignia->AutoMask();
                        }

                        else if (pdef->name()->value() == ("type")) {
                           Text txt;
                           GetDefText(txt, pdef, filename);
                           txt.setSensitive(false);

                           if (txt == "rank")
                              award->type = AwardInfo::RANK;

                           else if (txt == "medal")
                              award->type = AwardInfo::MEDAL;
                        }

                        else if (pdef->name()->value() == ("id"))
                           GetDefNumber(award->id, pdef, filename);

                        else if (pdef->name()->value() == ("total_points"))
                           GetDefNumber(award->total_points, pdef, filename);

                        else if (pdef->name()->value() == ("mission_points"))
                           GetDefNumber(award->mission_points, pdef, filename);

                        else if (pdef->name()->value() == ("total_missions"))
                           GetDefNumber(award->total_missions, pdef, filename);

                        else if (pdef->name()->value() == ("kills"))
                           GetDefNumber(award->kills, pdef, filename);

                        else if (pdef->name()->value() == ("lost"))
                           GetDefNumber(award->lost, pdef, filename);

                        else if (pdef->name()->value() == ("collision"))
                           GetDefNumber(award->collision, pdef, filename);

                        else if (pdef->name()->value() == ("campaign_id"))
                           GetDefNumber(award->campaign_id, pdef, filename);

                        else if (pdef->name()->value() == ("campaign_complete"))
                           GetDefBool(award->campaign_complete, pdef, filename);

                        else if (pdef->name()->value() == ("dynamic_campaign"))
                           GetDefBool(award->dynamic_campaign, pdef, filename);

                        else if (pdef->name()->value() == ("ceremony"))
                           GetDefBool(award->ceremony, pdef, filename);

                        else if (pdef->name()->value() == ("required_awards"))
                           GetDefNumber(award->required_awards, pdef, filename);

                        else if (pdef->name()->value() == ("lottery"))
                           GetDefNumber(award->lottery, pdef, filename);

                        else if (pdef->name()->value() == ("min_rank"))
                           GetDefNumber(award->min_rank, pdef, filename);

                        else if (pdef->name()->value() == ("max_rank"))
                           GetDefNumber(award->max_rank, pdef, filename);

                        else if (pdef->name()->value() == ("min_ship_class")) {
                           Text classname;
                           GetDefText(classname, pdef, filename);
                           award->min_ship_class = Ship::ClassForName(classname);
                        }

                        else if (pdef->name()->value() == ("max_ship_class")) {
                           Text classname;
                           GetDefText(classname, pdef, filename);
                           award->max_ship_class = Ship::ClassForName(classname);
                        }

                        else if (pdef->name()->value().indexOf("grant") == 0)
                           GetDefNumber(award->granted_ship_classes, pdef, filename);
                     }
                  }

                  if (award->type == AwardInfo::RANK) {
                     rank_table.append(award);
                  }

                  else if (award->type == AwardInfo::MEDAL) {
                     medal_table.append(award);
                  }

                  else {
                     delete award;
                  }
               }
            }
            else {
               Print("WARNING: unknown label '%s' in '%s'\n",
                        def->name()->value().data(), filename);
            }
         }
         else {
            Print("WARNING: term ignored in '%s'\n", filename);
            term->print();
         }
      }
   }
   while (term);

   loader->ReleaseBuffer(block);
   loader->SetDataPath(0);
}

// +-------------------------------------------------------------------+

void
Player::CreateUniqueID()
{
   ListIter<Player> iter = player_roster;
   while (++iter) {
      Player* p = iter.value();

      if (p != this && p->uid >= uid)
         uid = p->uid + 1;
   }

   if (uid < 1)
      uid = 1;
}

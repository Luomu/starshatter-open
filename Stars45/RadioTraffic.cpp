/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         RadioTraffic.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Radio message handler class implementation
*/

#include "MemDebug.h"
#include "RadioTraffic.h"
#include "RadioMessage.h"
#include "RadioView.h"
#include "RadioVox.h"
#include "Instruction.h"
#include "Ship.h"
#include "Contact.h"
#include "Element.h"
#include "Sim.h"
#include "NetGame.h"
#include "NetData.h"

#include "Game.h"
#include "Text.h"

// +----------------------------------------------------------------------+

RadioTraffic*  RadioTraffic::radio_traffic = 0;

// +----------------------------------------------------------------------+

RadioTraffic::RadioTraffic()
{
	radio_traffic = this;
}

RadioTraffic::~RadioTraffic()
{
	traffic.destroy();
}

// +----------------------------------------------------------------------+

void
RadioTraffic::Initialize()
{
	if (!radio_traffic)
	radio_traffic = new(__FILE__,__LINE__) RadioTraffic;
}

void
RadioTraffic::Close()
{
	delete radio_traffic;
	radio_traffic = 0;
}

// +--------------------------------------------------------------------+

void
RadioTraffic::SendQuickMessage(Ship* ship, int action)
{
	if (!ship) return;

	Element* elem  = ship->GetElement();

	if (elem) {
		if (action >= RadioMessage::REQUEST_PICTURE) {
			Ship* controller = ship->GetController();

			if (controller && !ship->IsStarship()) {
				RadioMessage* msg = new(__FILE__,__LINE__) RadioMessage(controller, ship, action);
				Transmit(msg);
			}
		}
		else if (action >= RadioMessage::SPLASH_1 && action <= RadioMessage::DISTRESS) {
			RadioMessage* msg = new(__FILE__,__LINE__) RadioMessage((Element*) 0, ship, action);
			Transmit(msg);
		}
		else {
			RadioMessage* msg = new(__FILE__,__LINE__) RadioMessage(elem, ship, action);

			if (action == RadioMessage::ATTACK || action == RadioMessage::ESCORT)
			msg->AddTarget(ship->GetTarget());

			Transmit(msg);
		}
	}
}

// +----------------------------------------------------------------------+

void
RadioTraffic::Transmit(RadioMessage* msg)
{
	if (msg && radio_traffic) {
		NetGame* net_game = NetGame::GetInstance();
		if (net_game) {
			NetCommMsg net_msg;
			net_msg.SetRadioMessage(msg);
			net_game->SendData(&net_msg);
		}

		radio_traffic->SendMessage(msg);
	}
}

// +----------------------------------------------------------------------+

void
RadioTraffic::SendMessage(RadioMessage* msg)
{
	if (!msg) return;

	Sim*  sim    = Sim::GetSim();
	Ship* player = sim->GetPlayerShip();
	int   iff    = 0;

	if (player)
	iff = player->GetIFF();

	if (msg->DestinationShip()) {
		traffic.append(msg);

		if (msg->Channel() == 0 || msg->Channel() == iff)
		DisplayMessage(msg);

		if (!NetGame::IsNetGameClient())
		msg->DestinationShip()->HandleRadioMessage(msg);
	}

	else if (msg->DestinationElem()) {
		traffic.append(msg);

		if (msg->Channel() == 0 || msg->Channel() == iff)
		DisplayMessage(msg);

		if (!NetGame::IsNetGameClient())
		msg->DestinationElem()->HandleRadioMessage(msg);
	}

	else {
		if (msg->Channel() == 0 || msg->Channel() == iff)
		DisplayMessage(msg);

		delete msg;
	}
}

// +----------------------------------------------------------------------+

Text
RadioTraffic::TranslateVox(const char* phrase)
{
	Text vox = "vox.";
	vox += phrase;
	vox.toLower();
	vox = Game::GetText(vox);

	if (vox.contains("vox."))  // failed to translate
	return Text(phrase);    // return the original text

	return vox;
}

void
RadioTraffic::DisplayMessage(RadioMessage* msg)
{
	if (!msg) return;

	char txt_buf[256];   txt_buf[0] = 0;
	char msg_buf[128];   msg_buf[0] = 0;
	char src_buf[64];    src_buf[0] = 0;
	char dst_buf[64];    dst_buf[0] = 0;
	char act_buf[64];    act_buf[0] = 0;
	int  vox_channel = 0;

	Ship*    dst_ship = msg->DestinationShip();
	Element* dst_elem = msg->DestinationElem();

	// BUILD SRC AND DST BUFFERS -------------------

	if (msg->Sender()) {
		const Ship* sender = msg->Sender();

		// orders to self?
		if (dst_elem && dst_elem->NumShips() == 1 && dst_elem->GetShip(1) == sender) {
			if (msg->Action() >= RadioMessage::CALL_ENGAGING) {
				sprintf(src_buf, "%s", sender->Name());

				if (sender->IsStarship())
				vox_channel = (sender->Identity()%3) + 5;
			}
		}

		// orders to other ships:
		else {
			if (sender->IsStarship()) {
				vox_channel = (sender->Identity()%3) + 5;
			}
			else {
				vox_channel = sender->GetElementIndex();
			}

			if (msg->Action() >= RadioMessage::CALL_ENGAGING) {
				sprintf(src_buf, "%s", sender->Name());
			}
			else {
				sprintf(src_buf, "This is %s", sender->Name());

				if (dst_ship) {
					// internal announcement
					if (dst_ship->GetElement() == sender->GetElement()) {
						dst_elem  = sender->GetElement();
						int index = sender->GetElementIndex();

						if (index > 1 && dst_elem) {
							sprintf(dst_buf, "%s Leader",     (const char*) dst_elem->Name());
							sprintf(src_buf, "this is %s %d", (const char*) dst_elem->Name(), index);
						}
						else {
							sprintf(src_buf, "this is %s leader", (const char*) dst_elem->Name());
						}
					}

					else {
						strcpy(dst_buf, (const char*) dst_ship->Name());
						src_buf[0] = tolower(src_buf[0]);
					}
				}

				else if (dst_elem) {
					// flight
					if (dst_elem->NumShips() > 1) {
						sprintf(dst_buf, "%s Flight", (const char*) dst_elem->Name());

						// internal announcement
						if (sender->GetElement() == dst_elem) {
							int index = sender->GetElementIndex();

							if (index > 1) {
								sprintf(dst_buf, "%s Leader",     (const char*) dst_elem->Name());
								sprintf(src_buf, "this is %s %d", (const char*) dst_elem->Name(), index);
							}
							else {
								sprintf(src_buf, "this is %s leader", (const char*) dst_elem->Name());
							}
						}
					}

					// solo
					else {
						strcpy(dst_buf, (const char*) dst_elem->Name());
						src_buf[0] = tolower(src_buf[0]);
					}
				}
			}
		}
	}

	// BUILD ACTION AND TARGET BUFFERS -------------------

	SimObject* target = 0;

	strcpy(act_buf, RadioMessage::ActionName(msg->Action()));

	if (msg->TargetList().size() > 0)
	target = msg->TargetList()[0];

	if (msg->Action() == RadioMessage::ACK ||
			msg->Action() == RadioMessage::NACK) {

		if (dst_ship == msg->Sender()) {
			src_buf[0] = 0;
			dst_buf[0] = 0;

			if (msg->Action() == RadioMessage::ACK)
			sprintf(msg_buf, "%s.", TranslateVox("Acknowledged").data());
			else
			sprintf(msg_buf, "%s.", TranslateVox("Unable").data());
		}
		else if (msg->Sender()) {
			dst_buf[0] = 0;

			if (msg->Info().length()) {
				sprintf(msg_buf, "%s. %s", 
				TranslateVox(act_buf).data(),
				(const char*) msg->Info());
			}
			else {
				sprintf(msg_buf, "%s.", TranslateVox(act_buf).data());
			}
		}
		else {
			if (msg->Info().length()) {
				sprintf(msg_buf, "%s. %s", 
				TranslateVox(act_buf).data(),
				(const char*) msg->Info());
			}
			else {
				sprintf(msg_buf, "%s.", TranslateVox(act_buf).data());
			}
		}
	}

	else if (msg->Action() == RadioMessage::MOVE_PATROL) {
		sprintf(msg_buf, TranslateVox("Move patrol.").data());
	}

	else if (target && dst_ship && msg->Sender()) {
		Contact* c = msg->Sender()->FindContact(target);

		if (c && c->GetIFF(msg->Sender()) > 10) {
			sprintf(msg_buf, "%s %s.", TranslateVox(act_buf).data(), TranslateVox("unknown contact").data());
		}

		else {
			sprintf(msg_buf, "%s %s.", 
			TranslateVox(act_buf).data(),
			target->Name());
		}
	}

	else if (target) {
		sprintf(msg_buf, "%s %s.", 
		TranslateVox(act_buf).data(),
		target->Name());
	}

	else if (msg->Info().length()) {
		sprintf(msg_buf, "%s %s", 
		TranslateVox(act_buf).data(),
		(const char*) msg->Info());
	}

	else {
		strcpy(msg_buf, TranslateVox(act_buf).data());
	}

	char last_char = msg_buf[strlen(msg_buf)-1];
	if (last_char != '!' && last_char != '.' && last_char != '?')
	strcat(msg_buf, ".");

	// final format:
	if (dst_buf[0] && src_buf[0]) {
		sprintf(txt_buf, "%s %s. %s", TranslateVox(dst_buf).data(), TranslateVox(src_buf).data(), msg_buf);
		txt_buf[0] = toupper(txt_buf[0]);
	}

	else if (src_buf[0]) {
		sprintf(txt_buf, "%s. %s", TranslateVox(src_buf).data(), msg_buf);
		txt_buf[0] = toupper(txt_buf[0]);
	}

	else if (dst_buf[0]) {
		sprintf(txt_buf, "%s %s", TranslateVox(dst_buf).data(), msg_buf);
		txt_buf[0] = toupper(txt_buf[0]);
	}

	else {
		strcpy(txt_buf, msg_buf);
	}

	// vox:
	const char* path[8] = { "1", "1", "2", "3", "4", "5", "6", "7" };

	RadioVox* vox = new(__FILE__,__LINE__) RadioVox(vox_channel, path[vox_channel], txt_buf);

	vox->AddPhrase(dst_buf);
	vox->AddPhrase(src_buf);
	vox->AddPhrase(act_buf);

	if (vox && !vox->Start()) {
		RadioView::Message(txt_buf);
		delete vox;
	}
}

// +----------------------------------------------------------------------+

void
RadioTraffic::DiscardMessages()
{
	if (radio_traffic)
	radio_traffic->traffic.destroy();
}

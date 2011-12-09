/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         ModInfo.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Information block for describing and deploying third party mods
*/


#include "MemDebug.h"
#include "ModInfo.h"
#include "Campaign.h"
#include "ShipDesign.h"
#include "ParseUtil.h"

#include "Archive.h"
#include "DataLoader.h"
#include "PCX.h"
#include "Bitmap.h"

// +-------------------------------------------------------------------+

ModInfo::ModInfo()
: logo(0), distribute(false), enabled(false), catalog(0)
{ }

ModInfo::ModInfo(const char* fname)
: logo(0), distribute(false), enabled(false), catalog(0)
{
	if (fname && *fname) {
		Load(fname);
	}
}

ModInfo::ModInfo(const char* n, const char* v, const char* u)
: name(n), logo(0), version(v), url(u), distribute(false), enabled(false), catalog(0)
{ }

ModInfo::~ModInfo()
{
	if (enabled)
	Disable();

	delete logo;
	delete catalog;
	campaigns.destroy();
}

// +-------------------------------------------------------------------+

bool
ModInfo::Load(const char* fname)
{
	bool ok = false;

	filename = fname;
	DataArchive a(filename);

	int n = a.FindEntry("mod_info.def");
	if (n > -1) {
		BYTE* buf = 0;
		int   len = a.ExpandEntry(n, buf, true);

		if (len > 0 && buf != 0) {
			ok = ParseModInfo((const char*) buf);
			delete [] buf;
		}
	}

	if (logoname.length()) {
		PcxImage pcx;

		logo = new(__FILE__,__LINE__) Bitmap;

		n = a.FindEntry(logoname);
		if (n > -1) {
			BYTE* buf = 0;
			int   len = a.ExpandEntry(n, buf, true);

			pcx.LoadBuffer(buf, len);
			delete [] buf;
		}

		// now copy the image into the bitmap:
		if (pcx.bitmap) {
			logo->CopyImage(pcx.width, pcx.height, pcx.bitmap);
		}

		else if (pcx.himap) {
			logo->CopyHighColorImage(pcx.width, pcx.height, pcx.himap);
		}

		else {
			logo->ClearImage();
		}
	}

	return ok;
}

bool
ModInfo::ParseModInfo(const char* block)
{
	bool   ok = false;
	Parser parser(new(__FILE__,__LINE__) BlockReader(block));
	Term*  term = parser.ParseTerm();

	if (!term) {
		Print("ERROR: could not parse '%s'\n", filename.data());
		return ok;
	}
	else {
		TermText* file_type = term->isText();
		if (!file_type || file_type->value() != "MOD_INFO") {
			Print("ERROR: invalid mod_info file '%s'\n", filename.data());
			term->print(10);
			return ok;
		}
	}

	ok = true;

	do {
		delete term; term = 0;
		term = parser.ParseTerm();

		if (term) {
			TermDef* def = term->isDef();
			if (def) {
				Text defname = def->name()->value();

				if (defname == "name")
				GetDefText(name, def, filename);

				else if (defname == "desc" || defname == "description")
				GetDefText(desc, def, filename);

				else if (defname == "author")
				GetDefText(author, def, filename);

				else if (defname == "url")
				GetDefText(url, def, filename);

				else if (defname == "copyright")
				GetDefText(copyright, def, filename);

				else if (defname == "logo")
				GetDefText(logoname, def, filename);

				else if (defname == "version") {
					if (def->term()) {
						if (def->term()->isNumber()) {
							int  v = 0;
							char buf[32];
							GetDefNumber(v, def, filename);
							sprintf_s(buf, "%d", v);

							version = buf;
						}

						else if (def->term()->isText()) {
							GetDefText(version, def, filename);
						}
					}
				}

				else if (defname == "distribute")
				GetDefBool(distribute, def, filename);

				else if (defname == "campaign") {
					if (!def->term() || !def->term()->isStruct()) {
						Print("WARNING: campaign structure missing in mod_info.def for '%s'\n", name.data());
					}
					else {
						TermStruct* val = def->term()->isStruct();

						ModCampaign* c = new(__FILE__,__LINE__) ModCampaign;

						for (int i = 0; i < val->elements()->size(); i++) {
							TermDef* pdef = val->elements()->at(i)->isDef();
							if (pdef) {
								defname = pdef->name()->value();

								if (defname == "name") {
									GetDefText(c->name, pdef, filename);
								}

								else if (defname == "path") {
									GetDefText(c->path, pdef, filename);
								}

								else if (defname == "dynamic") {
									GetDefBool(c->dynamic, pdef, filename);
								}
							}
						}

						if (c->name.length() && c->path.length())
						campaigns.append(c);
						else
						delete c;
					}
				}

				else if (defname == "catalog") {
					if (!def->term() || !def->term()->isStruct()) {
						Print("WARNING: catalog structure missing in mod_info.def for '%s'\n", name.data());
					}
					else {
						TermStruct* val = def->term()->isStruct();

						ModCatalog* c = new(__FILE__,__LINE__) ModCatalog;

						for (int i = 0; i < val->elements()->size(); i++) {
							TermDef* pdef = val->elements()->at(i)->isDef();
							if (pdef) {
								defname = pdef->name()->value();

								if (defname == "file") {
									GetDefText(c->file, pdef, filename);
								}

								else if (defname == "path") {
									GetDefText(c->path, pdef, filename);

									char last_char = c->path[c->path.length()-1];

									if (last_char != '/' && last_char != '\\')
									c->path += "/";
								}
							}
						}

						if (c->file.length() && c->path.length() && !catalog)
						catalog = c;
						else
						delete c;
					}
				}
			}     // def
		}        // term
	}
	while (term);

	return ok;
}

// +-------------------------------------------------------------------+

bool
ModInfo::Enable()
{
	DataLoader* loader = DataLoader::GetLoader();

	if (loader && !enabled) {
		loader->EnableDatafile(filename);
		enabled = true;

		if (catalog)
		ShipDesign::LoadCatalog(catalog->Path(), catalog->File(), true);

		ShipDesign::LoadSkins("/Mods/Skins/", filename);

		for (int i = 0; i < campaigns.size(); i++) {
			ModCampaign* c = campaigns[i];
			Campaign::CreateCustomCampaign(c->Name(), c->Path());
		}
	}

	return enabled;
}

bool
ModInfo::Disable()
{
	DataLoader* loader = DataLoader::GetLoader();

	if (loader) {
		loader->DisableDatafile(filename);
		enabled = false;
	}

	return !enabled;
}

// +-------------------------------------------------------------------+

#include <filesystem>
#include "PugiXML/pugixml.hpp"
#include "game.h"
#include "battle.h"

/* Default File Names */
constexpr auto GAME_XML = "game.xml";
constexpr auto VARS_XML = "vars.xml";
constexpr auto MONSTERS = "enemies.xml";
constexpr auto ENCO_XML = "encounters.xml";
constexpr auto MOVE_XML = "moves.xml";
constexpr auto SAVE_XML = "save.xml";

/*
    Enabling this will display the <banner>
    tags in all XML files, not just game.xml.
*/
constexpr bool XTRA_BANNERS = false;

/* Print XML Errors to the screen. */
void XML_Error(const pugi::char_t* source, pugi::xml_parse_result result);

/* Attempt to load a different file. */
void Game::InterpretArg(std::string arg)
{
    altXML = arg;
}

bool Game::ReadFile(bool firstBoot)
{
    std::string mFile = LoadString(altXML, GAME_XML);

    const pugi::char_t* main_source = mFile.c_str();
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(main_source);

    if (result)
    {
        /* Read the script's startup banner, if available. */
        if (firstBoot)
        {
            if (doc.child("banner"))
                std::cout << doc.child_value("banner") << std::endl;

            /* Determine if a custom enemy file was specified. */
            std::string eFile = MONSTERS;
            for (pugi::xml_node data = doc.child("file"); data; data = data.next_sibling("file"))
                if (std::string(data.attribute("data").value()) == "enemies")
                    eFile = doc.child_value("file");

            /* Determine if a custom move file was specified. */
            std::string mFile = MOVE_XML;
            for (pugi::xml_node data = doc.child("file"); data; data = data.next_sibling("file"))
                if (std::string(data.attribute("data").value()) == "moves")
                    mFile = doc.child_value("file");

            /* Determine if a custom var file was specified. */
            std::string vFile = VARS_XML;
            for (pugi::xml_node data = doc.child("file"); data; data = data.next_sibling("file"))
                if (std::string(data.attribute("data").value()) == "variables")
                    vFile = doc.child_value("file");

            /* Determine if a custom encounter file was specified. */
            std::string nFile = ENCO_XML;
            for (pugi::xml_node data = doc.child("file"); data; data = data.next_sibling("file"))
                if (std::string(data.attribute("data").value()) == "encounters")
                    nFile = doc.child_value("file");

            /* Determine if a custom save file was specified. */
            saveXML = SAVE_XML;
            for (pugi::xml_node data = doc.child("file"); data; data = data.next_sibling("file"))
            {
                if (std::string(data.attribute("data").value()) == "save")
                {
                    saveXML = doc.child_value("file");
                }
            }

            /* Read Variable XML file. */
            if (!ReadVarFile(vFile))
            {
                std::cout << "Variable file error!" << std::endl;
                return false;
            }

            /* Read Enemy XML file. */
            if (!combatSys->ReadFile(eFile))
            {
                std::cout << "Enemy file error!" << std::endl;
                return false;
            }

            /* Read Moves XML file. */
            if (!combatSys->ReadMoveFile(mFile))
            {
                std::cout << "Move file error!" << std::endl;
                return false;
            }

            /* Read Encounter XML file. */
            if (!ReadEncounterFile(nFile))
            {
                std::cout << "Encounter file error!" << std::endl;
                return false;
            }
        }

        /* Read all the "location" tags in the file. */
        std::string desc = "You are in an empty room.";
        for (pugi::xml_node room = doc.child("location"); room; room = room.next_sibling("location"))
            /* Find the one for the current room. */
            if (room.attribute("name").value() == GetRoom())
            {
                for (pugi::xml_node description = room.child("description"); description; description = description.next_sibling("description"))
                {
                    /* Load the description. */
                    bool allowed = true;
                    if (description.attribute("prohibit"))
                    {
                        allowed = false;
                        if (FindGameVar(description.attribute("prohibit").value()))
                            allowed = LoadGameVar(description.attribute("prohibit").value(), true) == "0";
                    }
                    if (description.attribute("require"))
                    {
                        allowed = false;
                        if (FindGameVar(description.attribute("require").value()))
                            allowed = LoadGameVar(description.attribute("require").value(), true) == "1";
                    }
                    if (allowed)
                        desc = LoadString(description.child_value(), desc);
                }

                /* Search the description for replaceable words. */
                size_t position = desc.find("$");
                while (position != std::string::npos)
                {
                    size_t nlen = desc.find("$", position + 1);
                    std::string word = desc.substr(position, nlen - position + 1);
                    if (word.substr(1, word.length() - 2) != "")
                    {
                        std::string s_var = word.substr(1, word.length() - 2);
                        if (FindGameVar(s_var))
                            desc = ReplaceSubstring(desc, word, ColoredString(LoadGameVar(s_var, true), Color::COLOR_WHITE));
                    }
                    position = desc.find("$", position + 1);
                }

                /* Add the rooms name and description to room list. */
                AddRoom(room.attribute("name").value(), desc, !room.child("return"));

                /* Check whether the room sets a variable. */
                if (room.child("variable"))
                {
                    if (room.child("variable").attribute("name").value())
                        AddRoomVar(room.attribute("name").value(),
                            room.child("variable").attribute("name").value(),
                            LoadString(room.child_value("variable"), "1"),
                            LoadString(room.child("variable").attribute("op").value(), "SET"));
                }

                /* Find out if this room gives items. */
                // TODO: Item removal
                if (room.child("inventory"))
                    AddInventoryItem(room.child("inventory").attribute("name").value(),
                        atoi(LoadString(room.child_value("inventory"), "1").c_str()));

                /* This room heals. */
                if (room.child("heal"))
                {
                    for (auto m = 0; m < PARTYSIZE; m++)
                    {
                        if (party[m])
                        {
                            party[m]->Heal();
                        }
                    }
                }

                if (room.child("options"))
                    /* Read all of its options. */
                    for (pugi::xml_node option = room.child("options").child("option"); option; option = option.next_sibling("option"))
                    {
                        bool allowed = true;
                        if (option.attribute("prohibit"))
                        {
                            allowed = false;
                            if (FindGameVar(option.attribute("prohibit").value()))
                                allowed = LoadGameVar(option.attribute("prohibit").value(), true) == "0";
                        }
                        if (option.attribute("require"))
                        {
                            allowed = false;
                            if (FindGameVar(option.attribute("require").value()))
                                allowed = LoadGameVar(option.attribute("require").value(), true) == "1";
                        }
                        if (allowed)
                            AddChoice(option.child_value(), option.attribute("room").value());
                    }
            }
        /* These options are always available. */
        AddChoice("BACK", GetLastRoom());
        AddChoice("HELP", "Help");
        AddChoice("SUMMARY", "Summary");
        AddChoice("RESTART", "RESTART");
        AddChoice("FLASK", "YE FLASK");
        AddChoice("LOAD", "LoadGame");
        if (Room != "GameOver")
        {
            AddChoice("SAVE", "SaveGame");
        }
        /* These default rooms can be found in Game::Setup() from Game.cpp */
    }
    else
    {
        XML_Error(main_source, result);
    }
    return result;
}

/* Read Enemy File */
bool CombatSys::ReadFile(std::string file)
{
    const pugi::char_t* source = file.c_str();
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(source);

    if (result)
    {
        /* Read the script's startup banner, if available. */
        if (doc.child("banner") && XTRA_BANNERS)
            std::cout << doc.child_value("banner") << std::endl;

        /* Read all the "enemy" tags in the file. */
        for (pugi::xml_node enemy = doc.child("enemy"); enemy; enemy = enemy.next_sibling("enemy"))
        {
            if (enemy.child("name"))
            {
                enemies[eCount]->Setup(
                    enemy.child_value("name"), eCount,
                    atoi(enemy.child_value("health")),
                    atoi(enemy.child_value("attack")),
                    atoi(enemy.child_value("defense")),
                    atoi(enemy.child_value("specatk")),
                    atoi(enemy.child_value("specdef")),
                    atoi(enemy.child_value("speed")),
                    atoi(enemy.child_value("xpcurve")),
                    atoi(enemy.child_value("yieldxp")),
                    atoi(enemy.child_value("catchrate")),
                    atoi(enemy.child("evolve").attribute("level").value()),
                    TypeFromName(enemy.child_value("type1")),
                    TypeFromName(enemy.child_value("type2"))
                );
                /* Load the leanset. */
                for (pugi::xml_node move = enemy.child("move"); move; move = move.next_sibling("move"))
                    enemies[eCount]->AddMove(LoadString(move.child_value(), "Unnamed"), atoi(move.attribute("level").value()));
                eCount++;
            }
        }
    }
    else
    {
        XML_Error(source, result);
    }
    return result;
}

/* Read Move File */
bool CombatSys::ReadMoveFile(std::string file)
{
    const pugi::char_t* source = file.c_str();
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(source);

    if (result)
    {
        /* Read the script's startup banner, if available. */
        if (doc.child("banner") && XTRA_BANNERS)
            std::cout << doc.child_value("banner") << std::endl;

        /* Read all the "enemy" tags in the file. */
        for (pugi::xml_node move = doc.child("move"); move; move = move.next_sibling("move"))
        {
            if (move.child("name"))
            {
                moveList[mCount]->Setup(
                    move.child_value("name"),
                    move.child_value("type"),
                    atoi(move.child_value("pp")),
                    atoi(move.child_value("power")),
                    atoi(move.child_value("accuracy")),
                    move.child_value("effect")
                );
                mCount++;
            }
        }
        std::cout << std::endl;
    }
    else
    {
        XML_Error(source, result);
    }
    return result;
}

/* Read Var File */
bool Game::ReadVarFile(std::string file)
{
    const pugi::char_t* source = file.c_str();
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(source);

    if (result)
    {
        /* Read the script's startup banner, if available. */
        if (doc.child("banner") && XTRA_BANNERS)
            std::cout << doc.child_value("banner") << std::endl;

        /* Read all the "variable" tags in the file. */
        if (pugi::xml_node varblock = doc.child("variables"))
            for (pugi::xml_node gamevar = varblock.child("variable"); gamevar; gamevar = gamevar.next_sibling("variable"))
                if (gamevar.attribute("name").value())
                    AddGameVar(gamevar.attribute("name").value(), LoadString(gamevar.child_value(), "0"));
    }
    else
    {
        XML_Error(source, result);
    }
    return result;
}

/* Read Encounter File */
bool Game::ReadEncounterFile(std::string file)
{
    const pugi::char_t* source = file.c_str();
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(source);

    if (result)
    {
        /* Read the script's startup banner, if available. */
        if (doc.child("banner") && XTRA_BANNERS)
            std::cout << doc.child_value("banner") << std::endl;

        /* Read all the "area" tags in the file. */
        for (pugi::xml_node area = doc.child("area"); area; area = area.next_sibling("area"))
        {
            /* Create a struct to organize the data. */
            encounterData curArea;

            /* Is this a trainer encounter? */
            curArea.trainer = area.child("trainer");

            /* Read all the "enemy" tags in the area. */
            auto e = 0;
            for (pugi::xml_node enemy = area.child("enemy"); enemy; enemy = enemy.next_sibling("enemy"))
            {
                /* Read each enemy. */
                curArea.enemies[e] = atoi(enemy.child_value());

                /* Allow an enemy index offset for a dynamic monster type. */
                if (enemy.attribute("offset"))
                {
                    curArea.offset[e] = enemy.attribute("offset").value();
                }

                /* Read the chance, if present. */
                if (enemy.attribute("chance"))
                    curArea.chance[e] = atoi(enemy.attribute("chance").value());

                /* Read the level(s). */
                if (enemy.attribute("level"))
                {
                    std::string levels = enemy.attribute("level").value();
                    if (levels.find('-') < levels.length() || levels.find(',') < levels.length())
                    {
                        if (levels.find('-') < levels.length())
                        {
                            curArea.minLv[e] = atoi(levels.substr(0, levels.find('-')).c_str());
                            curArea.maxLv[e] = atoi(levels.substr(levels.find('-') + 1).c_str());
                            curArea.randType[e] = 1;
                        }
                        if (levels.find(',') < levels.length())
                        {
                            curArea.minLv[e] = atoi(levels.substr(0, levels.find(',')).c_str());
                            curArea.maxLv[e] = atoi(levels.substr(levels.find(',') + 1).c_str());
                            curArea.randType[e] = 2;
                        }
                    }
                    else
                        curArea.minLv[e] = curArea.maxLv[e] = atoi(levels.c_str());
                }
                e++;
            }
            encounterMap.insert(std::pair<std::string, encounterData>(area.attribute("name").value(), curArea));
        }
    }
    else
    {
        XML_Error(source, result);
    }
    return result;
}

bool Game::SaveGame()
{
    pugi::xml_document doc;
    pugi::xml_node location = doc.append_child("Location");
    location.text().set(GetLastRoom().c_str());

    if (pMoney)
    {
        pugi::xml_node wallet = doc.append_child("Money");
        wallet.text().set(std::to_string(pMoney).c_str());
    }

    pugi::xml_node varroot = doc.append_child("Variables");
    for(std::map<std::string, std::string>::iterator it = Vars.begin(); it != Vars.end(); ++it)
    {
        if(it->second != "0")
        {
            pugi::xml_node varchild = varroot.append_child("Variable");
            varchild.append_attribute("name") = it->first.c_str();
            varchild.text().set(it->second.c_str());
        }
    }

    pugi::xml_node invroot = doc.append_child("Inventory");
    for(std::map<std::string, int>::iterator it = Inventory.begin(); it != Inventory.end(); ++it)
    {
        if(it->second != 0)
        {
            pugi::xml_node invchild = invroot.append_child("Item");
            invchild.append_attribute("name") = it->first.c_str();
            invchild.text().set(std::to_string(it->second).c_str());
        }
    }

    if (pPartySize >= 1)
    {
        for (auto p = 0; p < pPartySize; p++)
        {
            pugi::xml_node memroot = doc.append_child("PartyMember");
            memroot.append_attribute("id") = party[p]->GetIdNum();

            pugi::xml_node nmchild = memroot.append_child("Name");
            nmchild.text().set(party[p]->GetNickname().substr(15,
                party[p]->GetNickname().length() - 27).c_str());

            pugi::xml_node lvchild = memroot.append_child("Level");
            lvchild.text().set(std::to_string(party[p]->GetLevel()).c_str());

            pugi::xml_node hpchild = memroot.append_child("HP");
            hpchild.text().set(std::to_string(party[p]->GetHP()).c_str());

            pugi::xml_node xpchild = memroot.append_child("Exp");
            xpchild.text().set(std::to_string(party[p]->GetExperience()).c_str());

            pugi::xml_node evroot = memroot.append_child("EV");
            pugi::xml_node ivroot = memroot.append_child("IV");
            for (auto s = 0; s < NUM_STATS; s++)
            {
                std::string n = "Attack"; // Case 1
                switch (s)
                {
                case HEALTH:
                    n = "Health";
                    break;
                case DEFENSE:
                    n = "Defense";
                    break;
                case SATTACK:
                    n = "SpAttack";
                    break;
                case SDEFENSE:
                    n = "SpDefense";
                    break;
                case SPEED:
                    n = "Speed";
                }
                pugi::xml_node evchild = evroot.append_child(n.c_str());
                evchild.text().set(std::to_string(party[p]->GetEV(s)).c_str());
                pugi::xml_node ivchild = ivroot.append_child(n.c_str());
                ivchild.text().set(std::to_string(party[p]->GetIV(s)).c_str());
            }

            if (party[p]->Asleep())
            {
                pugi::xml_node dbchild = memroot.append_child("Asleep");
            }
            if (party[p]->Burned())
            {
                pugi::xml_node dbchild = memroot.append_child("Burned");
            }
            if (party[p]->Frozen())
            {
                pugi::xml_node dbchild = memroot.append_child("Frozen");
            }
            if (party[p]->Paralyzed())
            {
                pugi::xml_node dbchild = memroot.append_child("Paralyzed");
            }
            if (party[p]->Poisoned())
            {
                pugi::xml_node dbchild = memroot.append_child("Poisoned");
            }
            if (party[p]->Seeded())
            {
                pugi::xml_node dbchild = memroot.append_child("Seeded");
            }

            pugi::xml_node mvroot = memroot.append_child("Moves");
            for (auto m = 0; m < MOVE_MEM; m++)
	        {
                if (party[p]->MoveName(m) != "")
                {
                    pugi::xml_node mvchild = mvroot.append_child("Move");
                    mvchild.text().set(party[p]->MoveName(m).c_str());
                    // NYI: mvchild.append_attribute("pp") = 
                }
	        }
        }
    }
    doc.save_file(SAVE_XML);
    return std::filesystem::exists(SAVE_XML);
}

bool Game::LoadGame()
{
    const pugi::char_t* source = SAVE_XML;
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(source);

    if (result)
    {
        /* Clear Old Game*/
        ResetGame();

        if (doc.child("Location"))
        {
            OldRoom = doc.child_value("Location");
            SetRoom(OldRoom);
        }

        if (doc.child("Money"))
        {
            pMoney = atoi(doc.child_value("Money"));
        }

        /* Read all the "variable" tags in the file. */
        if (pugi::xml_node varblock = doc.child("Variables"))
        {
            for (pugi::xml_node svar = varblock.child("Variable"); svar; svar = svar.next_sibling("Variable"))
            {
                if (svar.attribute("name").value())
                {
                    Vars.find(svar.attribute("name").value())->second = svar.text().as_string();
                }
            }
        }

        /* Read all the "item" tags in the file. */
        if (pugi::xml_node invblock = doc.child("Inventory"))
        {
            for (pugi::xml_node item = invblock.child("Item"); item; item = item.next_sibling("Item"))
            {
                if (item.attribute("name").value())
                {
                    AddInventoryItem(item.attribute("name").value(), item.text().as_int());
                }
            }
        }

        /* Read all the Party Members in the file. */
        for (pugi::xml_node member = doc.child("PartyMember"); member; member = member.next_sibling("PartyMember"))
        {
            
            if (member.attribute("id").value())
            {
                PartyMember* pm = new PartyMember(this);
                int id_no = atoi(member.attribute("id").value());

                std::string mname = "";
                if (member.child("Name"))
                {
                    std::string mname = member.child("name").text().as_string();
                }

                int new_lv = 1;
                if (member.child("Level"))
                {
                    new_lv = member.child("Level").text().as_int();
                }

                int new_hp = 10;
                if (member.child("HP"))
                {
                    new_hp = member.child("HP").text().as_int();
                }

                int new_xp = 0;
                if (member.child("Exp"))
                {
                    new_xp = member.child("Exp").text().as_int();
                }

                int new_ev[NUM_STATS] = { 0 };
                if (pugi::xml_node evblock = member.child("EV"))
                {
                    if (pugi::xml_node hlestat = evblock.child("Health"))
                    {
                        new_ev[HEALTH] = hlestat.text().as_int();
                    }

                    if (pugi::xml_node atestat = evblock.child("Attack"))
                    {
                        new_ev[ATTACK_STAT] = atestat.text().as_int();
                    }

                    if (pugi::xml_node dfestat = evblock.child("Defense"))
                    {
                        new_ev[DEFENSE] = dfestat.text().as_int();
                    }

                    if (pugi::xml_node saestat = evblock.child("SpAttack"))
                    {
                        new_ev[SATTACK] = saestat.text().as_int();
                    }

                    if (pugi::xml_node sdestat = evblock.child("SpDefense"))
                    {
                        new_ev[SDEFENSE] = sdestat.text().as_int();
                    }

                    if (pugi::xml_node spestat = evblock.child("Speed"))
                    {
                        new_ev[SPEED] = spestat.text().as_int();
                    }
                }

                int new_iv[NUM_STATS] = { 0 };
                if (pugi::xml_node ivblock = member.child("IV"))
                {
                    if (pugi::xml_node hlistat = ivblock.child("Health"))
                    {
                        new_iv[HEALTH] = hlistat.text().as_int();
                    }

                    if (pugi::xml_node atistat = ivblock.child("Attack"))
                    {
                        new_iv[ATTACK_STAT] = atistat.text().as_int();
                    }

                    if (pugi::xml_node dfistat = ivblock.child("Defense"))
                    {
                        new_iv[DEFENSE] = dfistat.text().as_int();
                    }

                    if (pugi::xml_node saistat = ivblock.child("SpAttack"))
                    {
                        new_iv[SATTACK] = saistat.text().as_int();
                    }

                    if (pugi::xml_node sdistat = ivblock.child("SpDefense"))
                    {
                        new_iv[SDEFENSE] = sdistat.text().as_int();
                    }

                    if (pugi::xml_node spistat = ivblock.child("Speed"))
                    {
                        new_iv[SPEED] = spistat.text().as_int();
                    }
                }

                if (member.child("Asleep"))
                {
                    pm->Sleep();
                }
                if (member.child("Burned"))
                {
                    pm->Burn();
                }
                if (member.child("Frozen"))
                {
                    pm->Freeze();
                }
                if (member.child("Paralyzed"))
                {
                    pm->Paralyze();
                }
                if (member.child("Poisoned"))
                {
                    pm->Poison();
                }
                if (member.child("Seeded"))
                {
                    pm->Seed();
                }

                pm->Create(id_no, new_lv, new_iv[HEALTH], new_iv[ATTACK_STAT],
                    new_iv[DEFENSE], new_iv[SATTACK],
                    new_iv[SDEFENSE], new_iv[SPEED]);

                for (auto eva = 0; eva < NUM_STATS; eva++)
                {
                    pm->AwardEV(new_ev[eva], eva);
                }

                pm->AwardXP(new_xp - pm->GetExperience());

                pm->SetNickname(mname);

                std::string nmoves[MOVE_MEM] = { "" };
                if (pugi::xml_node movblock = member.child("Moves"))
                {
                    auto mc = 0;
                    for (pugi::xml_node xmove = movblock.child("Move"); xmove; xmove = xmove.next_sibling("Move"))
                    {
                        pm->SetMove("", mc);
                        pm->SetMove(xmove.text().as_string(), mc);
                        // NYI: PP
                        mc++;
                    }
                }
                int npm = AddPartyMemberFull(pm, new_hp);
            }
        }
    }
    else
    {
        XML_Error(source, result);
    }
    return result;
}

/* Print XML Errors to the screen. */
void XML_Error(const pugi::char_t* source, pugi::xml_parse_result result)
{
    std::cout << "XML [" << source << "] parsed with errors.\n";
    std::cout << "Error description: " << result.description() << "\n";
    std::cout << "Error offset: " << result.offset << " (error at [..." << (source + result.offset) << "]\n\n";
}

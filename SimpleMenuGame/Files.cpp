#include "PugiXML/pugixml.hpp"
#include "game.h"
#include "battle.h"

constexpr auto GAME_XML = "game.xml";
constexpr auto VARS_XML = "vars.xml";
constexpr auto MONSTERS = "enemies.xml";
constexpr auto ENCO_XML = "encounters.xml";
constexpr auto MOVE_XML = "moves.xml";

constexpr bool XTRA_BANNERS = false;

bool Game::ReadFile(bool firstBoot)
{
    const pugi::char_t* SOURCE = GAME_XML;
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(SOURCE);

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
                    position = desc.find("$");
                }

                /* Add the rooms name and description to room list. */
                AddRoom(room.attribute("name").value(), desc, !room.child("return"));

                /* Check whether the room sets a variable. */
                if (room.child("variable"))
                {
                    if (room.child("variable").attribute("name").value())
                        AddRoomVar(room.attribute("name").value(),
                            room.child("variable").attribute("name").value(),
                            LoadString(room.child_value("variable"), "1"));
                }

                /* Find out if this room gives items. */
                if (room.child("inventory"))
                    AddInventoryItem(room.child("inventory").attribute("name").value(),
                        atoi(LoadString(room.child_value("inventory"), "1").c_str()));

                /* This room heals. */
                if (room.child("heal"))
                {
                    for (auto i = 0; i < PARTYSIZE; i++)
                    {
                        if (party[i])
                        {
                            party[i]->Heal();
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
        AddChoice("Back", GetLastRoom());
        AddChoice("HELP", "Help");
        AddChoice("RESTART", "RESTART");
        AddChoice("GET YE FLASK", "YE FLASK");
        AddChoice("GET FLASK", "FLASK");
    }
    /* Print XML Errors to the screen. */
    else
    {
        std::cout << "XML [" << SOURCE << "] parsed with errors.\n";
        std::cout << "Error description: " << result.description() << "\n";
        std::cout << "Error offset: " << result.offset << " (error at [..." << (SOURCE + result.offset) << "]\n\n";
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
    /* Print XML Errors to the screen. */
    else
    {
        std::cout << "XML [" << source << "] parsed with errors.\n";
        std::cout << "Error description: " << result.description() << "\n";
        std::cout << "Error offset: " << result.offset << " (error at [..." << (source + result.offset) << "]\n\n";
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
    /* Print XML Errors to the screen. */
    else
    {
        std::cout << "XML [" << source << "] parsed with errors.\n";
        std::cout << "Error description: " << result.description() << "\n";
        std::cout << "Error offset: " << result.offset << " (error at [..." << (source + result.offset) << "]\n\n";
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
    /* Print XML Errors to the screen. */
    else
    {
        std::cout << "XML [" << source << "] parsed with errors.\n";
        std::cout << "Error description: " << result.description() << "\n";
        std::cout << "Error offset: " << result.offset << " (error at [..." << (source + result.offset) << "]\n\n";
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
            auto i = 0;
            for (pugi::xml_node enemy = area.child("enemy"); enemy; enemy = enemy.next_sibling("enemy"))
            {
                /* Read each enemy. */
                curArea.enemies[i] = atoi(enemy.child_value());

                /* Read the chance, if present. */
                if (enemy.attribute("chance"))
                    curArea.chance[i] = atoi(enemy.attribute("chance").value());

                /* Read the level(s). */
                if (enemy.attribute("level"))
                {
                    std::string levels = enemy.attribute("level").value();
                    if (levels.find('-') < levels.length() || levels.find(',') < levels.length())
                    {
                        if (levels.find('-') < levels.length())
                        {
                            curArea.minLv[i] = atoi(levels.substr(0, levels.find('-')).c_str());
                            curArea.maxLv[i] = atoi(levels.substr(levels.find('-') + 1).c_str());
                            curArea.randType[i] = 1;
                        }
                        if (levels.find(',') < levels.length())
                        {
                            curArea.minLv[i] = atoi(levels.substr(0, levels.find(',')).c_str());
                            curArea.maxLv[i] = atoi(levels.substr(levels.find(',') + 1).c_str());
                            curArea.randType[i] = 2;
                        }
                    }
                    else
                        curArea.minLv[i] = curArea.maxLv[i] = atoi(levels.c_str());
                }
                i++;
            }
            encounterMap.insert(std::pair<std::string, encounterData>(area.attribute("name").value(), curArea));
        }
    }
    /* Print XML Errors to the screen. */
    else
    {
        std::cout << "XML [" << source << "] parsed with errors.\n";
        std::cout << "Error description: " << result.description() << "\n";
        std::cout << "Error offset: " << result.offset << " (error at [..." << (source + result.offset) << "]\n\n";
    }
    return result;
}

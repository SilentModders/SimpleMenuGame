#include "files.h"
#include "game.h"
#include "PugiXML/pugixml.hpp"

#define GAME_XML "game.xml"

bool ReadFile(bool firstBoot)
{
    const pugi::char_t* source = GAME_XML;
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(source);

    if (result)
    {
        /* Read the script's startup banner, if available. */
        if (firstBoot)
        {
            if (doc.child("banner"))
                std::cout << doc.child_value("banner") << std::endl;
            std::cout << std::endl;

            /* Read all the "varible" tags in the beginning */
            if (pugi::xml_node varblock = doc.child("variables"))
                for (pugi::xml_node gamevar = varblock.child("variable"); gamevar; gamevar = gamevar.next_sibling("variable"))
                    if (gamevar.attribute("name").value())
                        AddGameVar(gamevar.attribute("name").value(), LoadString(gamevar.child_value(), "0"));
        }

        /* Read all the "location" tags in the file. */
        for (pugi::xml_node room = doc.child("location"); room; room = room.next_sibling("location"))
            /* Find the one for the current room. */
            if (room.attribute("name").value() == GetRoom())
            {
                /* Load the description. */
                std::string desc = LoadString(room.child_value("description"), "You are in an empty room.");

                /* Search the description for replaceable words. */
                int position = desc.find("$");
                while (position >= 0)
                {
                    int nlen = desc.find("$", position + 1);
                    std::string word = desc.substr(position, nlen - position + 1);
                    if (word.substr(1, word.length() - 2) != "")
                    {
                        std::string s_var = word.substr(1, word.length() - 2);
                        if (FindGameVar(s_var))
                            desc = ReplaceSubstring(desc, word, LoadGameVar(s_var, true));
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
        std::cout << "XML [" << source << "] parsed with errors.\n";
        std::cout << "Error description: " << result.description() << "\n";
        std::cout << "Error offset: " << result.offset << " (error at [..." << (source + result.offset) << "]\n\n";
    }
    return result;
}

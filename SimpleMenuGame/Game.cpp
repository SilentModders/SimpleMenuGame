#include "game.h"
#include "PugiXML/pugixml.hpp"

#define GAME_XML "game.xml"

/* The Current Room */
std::string Room = "Main";
/* The initial vaule determines the entry point for the script. */

/* The last room is used for rooms that automatically exit. */
std::string OldRoom = Room;

/* The Current Command */
std::string Choice = "";

/* Available Commands */
std::map<std::string, std::string> choiceMap;

/* Available Rooms */
std::map<std::string, std::pair<std::string, bool>> roomMap;

/* Game has just started or restarted. */
bool firstBoot = true;

/* Game Script Variables */
std::map<std::string, std::string> gameVars;

/* Rooms which change variables */
std::map<std::string, std::pair<std::string, std::string>> roomVars;

int toupper(std::string& input)
{
    int ret = EXIT_SUCCESS;
    for (auto& c : input)
        if (!(c = toupper(c)))
            ret = EXIT_FAILURE;
    return ret;
}

/* Returns the string value if it is not empty, otherwise returns a default value. */
std::string LoadString(std::string input, std::string def)
{
    if (input != "")
        return input;
    return def;
}

/* Search "source" for any occurance of "find" and substitute "replace". */
std::string ReplaceSubstring(std::string source, std::string find, std::string replace)
{
    int position = 0;
    int origLength = find.length();
    position = source.find(find, position);

    while (position >= 0)
    {
        source.erase(position, origLength);
        source.insert(position, replace);
        position = source.find(find, position);
    }
    return source;
}

std::string GetChoice()
{
    return Choice;
}

void SetChoice(std::string choice)
{
    Choice = choice;
}

std::string GetRoom()
{
    return Room;
}

/* Set the player's location and clear old data. */
void SetRoom(std::string room)
{
    if (room == "RESTART")
    {
        std::cout << std::endl << std::endl;
        firstBoot = true;
        room = "Main";
        Choice = "";
    }

    OldRoom = Room;
    Room = room;

    /* Reload all commands. */
    roomMap.clear();
    choiceMap.clear();
    Setup();
}

void AddRoom(std::string key, std::string text, bool move=false)
{
    roomMap.insert(std::pair<std::string, std::pair<std::string, bool>>(key, std::make_pair(text, move)));
}

/* The XML file is re-read every time a new command is issued. */
bool ReadFile()
{
    const pugi::char_t* source = GAME_XML;
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(source);

    if (result)
    {
        /* Read the script's startup banner, if available. */
        if (firstBoot)
        {
            firstBoot = false;
            if (doc.child("banner"))
                std::cout << doc.child_value("banner") << std::endl;
            std::cout << std::endl;

            /* Read all the "varible" tags in the beginning */
            if (pugi::xml_node varblock = doc.child("variables"))
                for (pugi::xml_node gamevar = varblock.child("variable"); gamevar; gamevar = gamevar.next_sibling("variable"))
                    if (gamevar.attribute("name").value())
                    {
                        gameVars.insert(std::pair<std::string, std::string>(gamevar.attribute("name").value(),
                            LoadString(gamevar.child_value(), "0")));
                    }
        }

        /* Read all the "location" tags in the file. */
        for (pugi::xml_node room = doc.child("location"); room; room = room.next_sibling("location"))
            /* Find the one for the current room. */
            if (room.attribute("name").value() == Room)
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
                        if (gameVars.find(s_var) != gameVars.end())
                            desc = ReplaceSubstring(desc, word, gameVars.find(s_var)->second);
                    }
                    position = desc.find("$");
                }

                /* Add the rooms name and description to room list. */
                AddRoom(room.attribute("name").value(), desc, !room.child("return"));

                /* Check whether the room sets a variable. */
                if (room.child("variable"))
                {
                    if (room.child("variable").attribute("name").value())
                        roomVars.insert(std::pair<std::string, std::pair<std::string, std::string>>(
                            room.attribute("name").value(),
                            std::make_pair(room.child("variable").attribute("name").value(),
                                LoadString(room.child_value("variable"), "1"))
                            ));
                }


                if (room.child("options"))
                    /* Read all of its options. */
                    for (pugi::xml_node option = room.child("options").child("option"); option; option = option.next_sibling("option"))
                    {
                        bool allowed = true;
                        if (option.attribute("prohibit"))
                        {
                            allowed = false;
                            if (gameVars.find(option.attribute("prohibit").value()) != gameVars.end())
                                allowed = (gameVars.find(option.attribute("prohibit").value())->second == "0");
                        }
                        if (option.attribute("require"))
                        {
                            allowed = false;
                            if (gameVars.find(option.attribute("require").value()) != gameVars.end())
                                allowed = (gameVars.find(option.attribute("require").value())->second == "1");
                        }
                        if (allowed)
                            choiceMap.insert(std::pair<std::string, std::string>(
                                option.child_value(), option.attribute("room").value()));
                    }
            }
            /* These options are always available. */
            choiceMap.insert(std::pair<std::string, std::string>("Back", OldRoom));
            choiceMap.insert(std::pair<std::string, std::string>("HELP", "Help"));
            choiceMap.insert(std::pair<std::string, std::string>("RESTART", "RESTART"));
            choiceMap.insert(std::pair<std::string, std::string>("GET YE FLASK", "YE FLASK"));
            choiceMap.insert(std::pair<std::string, std::string>("GET FLASK", "FLASK"));
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
/* Create the list of available rooms. */
bool Setup()
{
    bool ret = false;

    /* Rooms in this list cannot be overridden. */
    AddRoom("invalid", "Invalid command; try again.");
    AddRoom("RESTART", "");

    /* Read the file to find our room. */
    ret = ReadFile();
    if (!ret)
        AddRoom(Room, "You in a void. No file was loaded. Please QUIT.", true);

    /* Rooms below here can be overridden. */
    AddRoom("YE FLASK", "You can't get YE FLASK!");
    AddRoom("FLASK", "Ye cannot get the FLASK.");
    AddRoom("Help", "You can type HELP for this message, RESTART to restart, and QUIT to quit.");
    
    if (roomMap.find(Room) == roomMap.end())
        AddRoom(Room, "You tried to enter \"" + Room + "\" which doesn't exist.");
    /* Rooms below here will not be read. */

    return ret;
}

/* Find the room that matches what was chosen. */
void ChooseRoom(std::string key)
{
    if (key != "")
        if (choiceMap.find(key) != choiceMap.end())
            SetRoom(choiceMap.find(key)->second);
        else
            SetRoom("invalid");
}

/* Sets a game variable when specified by the room. */
bool RoomFunc(std::string key)
{
    if (roomVars.find(key) != roomVars.end())
    {
        if (gameVars.find(roomVars.find(key)->second.first) != gameVars.end())
        {
            gameVars.find(roomVars.find(key)->second.first)->second =
                roomVars.find(key)->second.second;
            return true;
        }
    }
    return false;
}

/* Get the room's text. */
std::string RoomText(std::string key)
{
    if (roomMap.find(key) != roomMap.end())
    {
        /* Perform any Room action. */
        RoomFunc(key);
        return roomMap.find(key)->second.first;
    }
    return roomMap.find("invalid")->second.first;
}

/* Is this an auto-exit room? */
bool AutoRoom(std::string key)
{
    if (!roomMap.find(key)->second.second)
    {
        Choice = "Back";
        return true;
    }
    return false;
}

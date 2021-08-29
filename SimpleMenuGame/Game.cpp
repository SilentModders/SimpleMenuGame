#include "game.h"
#include "PugiXML/pugixml.hpp"

#define GAME_XML "game.xml"

std::string Room = "Main";
std::string OldRoom = Room;
std::string Choice = "";

std::map<std::string, std::string> choiceMap;
typedef std::pair<std::string, bool> roomData;
std::map<std::string, std::pair<std::string, bool>> roomMap;

bool firstBoot = true;

int toupper(std::string& input)
{
    int ret = EXIT_SUCCESS;
    for (auto& c : input)
        if (!(c = toupper(c)))
            ret = EXIT_FAILURE;
    return ret;
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

    /* Reload all commands */
    roomMap.clear();
    choiceMap.clear();
    Setup();
}

void AddRoom(std::string key, std::string text, bool move=false)
{
    roomMap.insert(std::pair<std::string, roomData>(key, std::make_pair(text, move)));
}

/* The XML file is re-read every time a new command is issued */
bool ReadFile()
{
    const pugi::char_t* source = GAME_XML;
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(source);

    if (result)
    {
        /* Read the scripts startup banner, if available */
        if (firstBoot)
        {
            if (doc.child("banner"))
                std::cout << doc.child_value("banner") << std::endl;
            std::cout << std::endl;
            firstBoot = false;
        }

        /* These choices are always available. */
        choiceMap.insert(std::pair<std::string, std::string>("Back", OldRoom));
        choiceMap.insert(std::pair<std::string, std::string>("HELP", "GameHelp"));
        choiceMap.insert(std::pair<std::string, std::string>("RESTART", "RESTART"));
        choiceMap.insert(std::pair<std::string, std::string>("GET YE FLASK", "YE FLASK"));
        choiceMap.insert(std::pair<std::string, std::string>("GET FLASK", "FLASK"));

        /* Read all the "location" tags in the file */
        for (pugi::xml_node room = doc.child("location"); room; room = room.next_sibling("location"))
        {
            /* Find the one for the current room. */
            if (room.attribute("name").value() == Room)
            {
                /* Add its name and description to room list. */
                AddRoom(room.attribute("name").value(), room.child_value("description"), !room.child("return"));
                if (room.child("options"))
                {
                    /* Read all of its options. */
                    for (pugi::xml_node option = room.child("options").child("option"); option; option = option.next_sibling("option"))
                    {
                        choiceMap.insert(std::pair<std::string, std::string>(option.child_value(), option.attribute("room").value()));
                    }
                }
                return result;
            }
        }
    }
    else
    {
        std::cout << "XML [" << source << "] parsed with errors.\n";
        std::cout << "Error description: " << result.description() << "\n";
        std::cout << "Error offset: " << result.offset << " (error at [..." << (source + result.offset) << "]\n\n";
    }
    return result;
}
/* Create the list of available rooms. */
void Setup()
{
    /* Rooms in this list cannot be overridden. */
    AddRoom("invalid", "Invalid command; try again.");
    AddRoom("RESTART", "");

    /* Read file to find our room. */
    if (!ReadFile())
        AddRoom(Room, "You in a void. No file was loaded. Please QUIT.", true);

    /* Rooms below here can be overridden. */
    AddRoom("YE FLASK", "You can't get YE FLASK!");
    AddRoom("FLASK", "Ye cannot get the FLASK.");
    AddRoom("GameHelp", "You can type HELP for this message, RESTART to restart, and QUIT to quit.");
    
    if (roomMap.find(Room) == roomMap.end())
        AddRoom(Room, "You tried to enter \"" + Room + "\" which doesn't exist.");
    /* Rooms below here will not be read. */
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

/* Get the room's text. */
std::string RoomText(std::string key)
{
    if (roomMap.find(key) != roomMap.end())
        return roomMap.find(key)->second.first;
    return roomMap.find("invalid")->second.first;
}

/* Is this an auto-exit room? */
bool AutoRoom(std::string key)
{
    return !roomMap.find(key)->second.second;
}

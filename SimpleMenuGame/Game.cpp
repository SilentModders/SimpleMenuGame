#include "game.h"
#include "files.h"

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

/* Game Inventory */
std::map<std::string, int> gameInventory;

/* The Current Choice */
std::string GetChoice()
{
    return Choice;
}

/* Set the Choice */
void SetChoice(std::string choice)
{
    Choice = choice;
}

/* The Current Room */
std::string GetRoom()
{
    return Room;
}

/* Previous Location */
std::string GetLastRoom()
{
    return OldRoom;
}

/* Set the player's location and clear old data. */
void SetRoom(std::string room)
{
    firstBoot = false;
    if (room == "RESTART")
    {
        std::cout << std::endl << std::endl;
        gameVars.clear();
        gameInventory.clear();
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

/* Add a Room to the List */
void AddRoom(std::string key, std::string text, bool move)
{
    roomMap.insert(std::pair<std::string, std::pair
        <std::string, bool>>(key, std::make_pair(text, move)));
}

/* Add Option for Player */
void AddChoice(std::string option, std::string room)
{
    if (option != "")
        choiceMap.insert(std::pair<std::string, std::string>(option, LoadString(room, "invalid")));
}

/* Add a variable from the game's script. */
void AddGameVar(std::string var, std::string val)
{
    if (var != "")
        gameVars.insert(std::pair<std::string, std::string>(var, LoadString(val, "0")));
}

/* Create the list of available rooms. */
bool Setup()
{
    bool ret = false;

    /* Rooms in this list cannot be overridden. */
    AddRoom("invalid", "Invalid command; try again.");
    AddRoom("RESTART", "");

    /* Read the file to find our room. */
    ret = ReadFile( firstBoot );
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

/* Set a room to set a variable. */
void AddRoomVar(std::string room, std::string var, std::string val)
{
    if (room != "")
        if (var != "")
            roomVars.insert(std::pair<
                std::string, std::pair<
                std::string, std::string>>(room,
                    std::make_pair(var,
                        LoadString(val, "0"))));
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

/* Check the existance of a game var. */
bool FindGameVar(std::string key)
{
    return gameVars.find(key) != gameVars.end();
}

/* Read a game var. */
std::string LoadGameVar(std::string key, bool second)
{
    if (gameVars.find(key) != gameVars.end())
        if (second)
            return gameVars.find(key)->second;
        else
            return gameVars.find(key)->first;
    return "Unknown";
}

/* Check the Inventory. */
bool FindInventoryItem(std::string key)
{
    return gameInventory.find(key) != gameInventory.end();
}

/* Add an item to the inventory. */
void AddInventoryItem(std::string item, int count)
{
    if (item != "")
        if (gameInventory.find(item) != gameInventory.end())
            gameInventory.find(item)->second += count;
        else
            gameInventory.insert(std::pair<std::string, int>(item, count));
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

#include "game.h"
#include "text.h"
#include "battle.h"

/*
    TODO: Seperate the engine from the UI for easier adaptation
        to any future UI.
//*/

constexpr auto CURRENCY_PFIX = "$";
constexpr auto CURRENCY_SFIX = "";
std::string Money(int money)
{
    return CURRENCY_PFIX + std::to_string(int(money)) + CURRENCY_SFIX;
}

Game::Game()
{
    firstBoot = true;
    Room = "Main";
    OldRoom = Room;
    Choice = "";
    pMoney = 0;
    combatSys = new CombatSys(this);
    for (auto i = 0; i < PARTYSIZE; i++)
        party[i] = nullptr;
}

/* The Current Choice */
std::string Game::GetChoice()
{
    return Choice;
}

/* Set the Choice */
void Game::SetChoice(std::string choice)
{
    Choice = choice;
}

/* The Current Room */
std::string Game::GetRoom()
{
    return Room;
}

/* Previous Location */
std::string Game::GetLastRoom()
{
    return OldRoom;
}

/* Set the player's location and clear old data. */
void Game::SetRoom(std::string room)
{
    firstBoot = false;
    if (room == "RESTART")
    {
        std::cout << std::endl << std::endl;
        Vars.clear();
        Inventory.clear();
        pMoney = 0;
        firstBoot = true;
        room = "Main";
        Choice = "";
    }

    if (Room != "Battle")
        OldRoom = Room;
    Room = room;

    /* Reload all commands. */
    roomMap.clear();
    choiceMap.clear();
    Setup();
}

/* Add a Room to the List */
void Game::AddRoom(std::string key, std::string text, bool move)
{
    roomMap.insert(std::pair<std::string, std::pair
        <std::string, bool>>(key, std::make_pair(text, move)));
}

/* Add Option for Player */
void Game::AddChoice(std::string option, std::string room)
{
    if (option != "")
        choiceMap.insert(std::pair<std::string, std::string>(option, LoadString(room, "invalid")));
}

/* Add a variable from the game's script. */
void Game::AddGameVar(std::string var, std::string val)
{
    if (var != "")
        Vars.insert(std::pair<std::string, std::string>(var, LoadString(val, "0")));
}

/* Refresh Game State */
bool Game::Setup()
{
    bool readFile = false;

    srand(unsigned(time(NULL))); // Seed the RNG

    /* Create the list of available rooms. */

    /* Rooms in this list cannot be overridden. */
    AddRoom("invalid", "Invalid command; try again.");
    AddRoom("RESTART", "");

    /* Read the file to find the room. */
    readFile = ReadFile(firstBoot);
    if (!readFile)
        AddRoom(Room, "You in a void. No file was loaded. Please QUIT.", true);

    /* Rooms below here can be overridden. */
    AddRoom("YE FLASK", "You can't get YE FLASK!");
    AddRoom("FLASK", "Ye cannot get the FLASK.");
    AddRoom("Help", "You can type HELP for this message, RESTART to restart, and QUIT to quit.");
    AddRoom("Battle", "You are in a battle. You can ATTACK, use an item from the BAG, or RUN.", true);
    AddRoom("GameOver", "Sorry, but your adventure has ended. Please RESTART or QUIT.", true);
    /* Rooms below here will not be read. */

    /* Setup the intial player state. */
    if (readFile && firstBoot)
        InitPlayer();

    return readFile;
}

void Game::Pause(int time)
{
    GameSleep(time);
}

/* Setup the intial player state. */
void Game::InitPlayer()
{
    /* Default Party for testing */
    party[0] = new PartyMember(this);
    party[0]->Create(7, 5);
}
PartyMember* Game::GetPartyMember(int index)
{
    return party[std::clamp(index, 0, PARTYSIZE - 1)];
}
int Game::AddPartyMember(int basetype, int level, int Hp)
{
    for (auto i = 0; i < PARTYSIZE; i++)
    {
        if (!party[i])
        {
            party[1] = new PartyMember(this);
            party[1]->Create(basetype, level);
            party[i]->SetHP(Hp);
            return i;
        }
        // TODO: Pokemon Boxes
    }
    return 0;
    /* This is okay, because you can
     * (almost) never have an empty party.
    //*/
}

int Game::AddMoney(int money)
{
    return pMoney += money;
}

CombatSys* Game::GetCombatSys()
{
    return combatSys;
}

encounterData* Game::ReadEncounterZone(std::string key)
{
    if (key != "")
        if (encounterMap.find(key) != encounterMap.end())
            return &encounterMap.find(key)->second;
    return nullptr;
}

/* Set a room to set a variable. */
void Game::AddRoomVar(std::string room, std::string var, std::string val)
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
void Game::ChooseRoom(std::string key)
{
    if (key != "")
        if (choiceMap.find(key) != choiceMap.end())
            SetRoom(choiceMap.find(key)->second);
        else
            if (!combatSys->InBattle())
                SetRoom("invalid");
}

/* Check the existance of a game var. */
bool Game::FindGameVar(std::string key)
{
    return Vars.find(key) != Vars.end();
}

/* Read a game var. */
std::string Game::LoadGameVar(std::string key, bool second)
{
    if (Vars.find(key) != Vars.end())
        if (second)
            return Vars.find(key)->second;
        else
            return Vars.find(key)->first;
    return "Unknown";
}

/* Check the Inventory. */
bool Game::FindInventoryItem(std::string key)
{
    return Inventory.find(key) != Inventory.end();
}

/* Add an item to the inventory. */
bool Game::AddInventoryItem(std::string item, int count)
{
    if (item != "")
    {
        if (Inventory.find(item) != Inventory.end())
            Inventory.find(item)->second += count;
        else
            Inventory.insert(std::pair<std::string, int>(item, count));
        return true;
    }
    return false;
}
bool Game::RemoveInventoryItem(std::string item, int count)
{
    if (item != "")
        if (Inventory.find(item) != Inventory.end())
        {
            Inventory.find(item)->second -= count;
            if (Inventory.find(item)->second <= 0)
                Inventory.extract(item);
            return true;
        }
    return false;
}

/* Sets a game variable when specified by the room. */
bool Game::RoomFunc(std::string key)
{
    if (key == "Battle")
        return !combatSys->BattleTurn();

    if (roomVars.find(key) != roomVars.end())
        if (Vars.find(roomVars.find(key)->second.first) != Vars.end())
            Vars.find(roomVars.find(key)->second.first)->second =
            roomVars.find(key)->second.second;
    return true;
}

/* Get the room's text. */
std::string Game::RoomText(std::string key)
{
    if (roomMap.find(key) != roomMap.end())
    {
        RoomFunc(key);
        return roomMap.find(GetRoom())->second.first;
    }
    return roomMap.find("invalid")->second.first;
}

/* Is this an auto-exit room? */
bool Game::AutoRoom(std::string key)
{
    if (!roomMap.find(key)->second.second)
    {
        Choice = "Back";
        return true;
    }
    return false;
}

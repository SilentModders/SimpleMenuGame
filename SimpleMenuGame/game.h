#pragma once
#include <algorithm>
#include <map>
#include "text.h"
#include "enemies.h"

constexpr auto PARTYSIZE = 6;

class CombatSys;

class Game
{
public:
	Game();

	/* Refresh Game State */
	bool Setup();

	std::string GetChoice();
	void SetChoice(std::string choice);

	std::string GetRoom();

	/* Is this an auto-exit room? */
	bool AutoRoom(std::string key);

	/* Find the room that matches what was chosen. */
	void ChooseRoom(std::string key);

	/* Get the room's text. */
	std::string RoomText(std::string key);

	/* Previous Location */
	std::string GetLastRoom();

	/* Set the player's location and clear old data. */
	void SetRoom(std::string room);

	/* Get Party Members. Empty slots are nullptr. */
	Enemy* PartyMember(int index);

	/* Save a party member's health. Returns false on an empty slot. */
	bool SaveHealth(int index, int health);

	/* Get a party member's health. Returns 0 on an empty slot. */
	int GetHealth(int index);

private:
	/* The Current Room */
	std::string Room;
	/* The initial vaule determines the entry point for the script. */

	/* The last room is used for rooms that automatically exit. */
	std::string OldRoom;

	/* The Current Command */
	std::string Choice;

	/* Available Commands */
	std::map<std::string, std::string> choiceMap;

	/* Available Rooms */
	std::map<std::string, std::pair<std::string, bool>> roomMap;

	/* Game has just started or restarted. */
	bool firstBoot;

	/* Game Script Variables */
	std::map<std::string, std::string> Vars;

	/* Rooms which change variables */
	std::map<std::string, std::pair<std::string, std::string>> roomVars;

	/* Game Inventory */
	std::map<std::string, int> Inventory;

	CombatSys* combatSys;

	/* Player's Party */
	Enemy* party[PARTYSIZE];
	int partyHP[PARTYSIZE];

	/* Setup the intial player state. */
	void InitPlayer();

	/* Add Option for Player */
	void AddChoice(std::string option, std::string room);

	/* Add a Room to the List */
	void AddRoom(std::string key, std::string text, bool move = false);

	/* Add a variable from the game's script. */
	void AddGameVar(std::string var, std::string val);

	/* Set a room to set a variable. */
	void AddRoomVar(std::string room, std::string var, std::string val);

	/* Check the existance of a game var. */
	bool FindGameVar(std::string key);

	/* Read a game var. */
	std::string LoadGameVar(std::string key, bool second = false);

	/* Check the Inventory. */
	bool FindInventoryItem(std::string key);

	/* Add an item to the inventory. */
	void AddInventoryItem(std::string item, int count);

	/* Perform Room actions. */
	bool RoomFunc(std::string key);

	/* The XML file is re-read every time a new command is issued. */
	bool ReadFile(bool firstBoot = false);
};

#pragma once

#include <map>
#include "text.h"

std::string GetChoice();
void SetChoice(std::string choice);
std::string GetRoom();
std::string GetLastRoom();
void SetRoom(std::string room);

void AddChoice(std::string option, std::string room);
void AddRoom(std::string key, std::string text, bool move = false);
void AddGameVar(std::string var, std::string val);

/* Create the list of available rooms. */
bool Setup();

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

/* Find the room that matches what was chosen. */
void ChooseRoom(std::string key);

/* Get the room's text. */
std::string RoomText(std::string key);

/* Is this an auto-exit room? */
bool AutoRoom(std::string key);

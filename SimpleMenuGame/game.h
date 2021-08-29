#pragma once

#include <iostream>
#include <map>
#include <string>

int toupper(std::string& input);

std::string GetChoice();
void SetChoice(std::string choice);
std::string GetRoom();
void SetRoom(std::string room);

// Create the list of available rooms.
void Setup();

// Find the room that matches what was chosen.
void ChooseRoom(std::string key);

// Get the room's text.
std::string RoomText(std::string key);

// Is this an auto-exit room?
bool AutoRoom(std::string key);
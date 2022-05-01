#pragma once
#include <string>

const enum class Types
{
	BUG,
	DARK,
	DRAGON,
	ELECTRIC,
	FAIRY,
	FIGHTING,
	FIRE,
	FLYING,
	GHOST,
	GRASS,
	GROUND,
	ICE,
	NORMAL,
	POISON,
	PSYCHIC,
	ROCK,
	STEEL,
	WATER,
	NUM_TYPES
};

std::string GetTypeName(Types typ = Types::NORMAL);
Types TypeFromNum(int num = 12);
Types TypeFromName(std::string nme = "Normal");

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
	NORMAL, // Type 2 is never Normal.
	POISON,
	PSYCHIC,
	ROCK,
	STEEL,
	WATER,
	NUM_TYPES
};

/*
	Returns the type multiplier when a given
	monster type is hit with a given move type.
*/
float Effective(Types targ = Types::NORMAL, Types atk = Types::NORMAL);

/*
	Returns the type multiplier when both given
	monster types are hit with a given move type.
*/
float TypeEffectiveness(Types targ1 = Types::NORMAL, Types targ2 = Types::NORMAL, Types atk = Types::NORMAL);

/* Use SP.ATK/DEF? */
bool SpecialType(Types mType = Types::NORMAL);

std::string GetTypeName(Types typ = Types::NORMAL);
Types TypeFromName(std::string nme = "Normal");

const float IMMUNE = 0; // Doesn't effect
const float UNEFTV = 0.5f; // Not very effective
const float NORM_D = 1; // Normal effectiveness
const float SPR_EF = 2; // Super effective

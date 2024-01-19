#include <algorithm>
#include "text.h"
#include "types.h"

/* Holds type effectiveness as [Defender][Attacker] */
const float EFTV_MAP[size_t(Types::NUM_TYPES)][size_t(Types::NUM_TYPES)] =
{
	{ NORM_D, NORM_D, NORM_D, NORM_D, NORM_D, UNEFTV, SPR_EF, SPR_EF, NORM_D, UNEFTV, UNEFTV, NORM_D, NORM_D, NORM_D, NORM_D, SPR_EF, NORM_D, NORM_D },
	{ SPR_EF, UNEFTV, NORM_D, NORM_D, SPR_EF, SPR_EF, NORM_D, NORM_D, UNEFTV, NORM_D, NORM_D, NORM_D, NORM_D, NORM_D, IMMUNE, NORM_D, NORM_D, NORM_D },
	{ NORM_D, NORM_D, SPR_EF, UNEFTV, SPR_EF, NORM_D, UNEFTV, NORM_D, NORM_D, UNEFTV, NORM_D, SPR_EF, NORM_D, NORM_D, NORM_D, NORM_D, NORM_D, UNEFTV },
	{ NORM_D, NORM_D, NORM_D, UNEFTV, NORM_D, NORM_D, NORM_D, UNEFTV, NORM_D, NORM_D, SPR_EF, NORM_D, NORM_D, NORM_D, NORM_D, NORM_D, UNEFTV, NORM_D },
	{ UNEFTV, UNEFTV, IMMUNE, NORM_D, NORM_D, UNEFTV, NORM_D, NORM_D, NORM_D, NORM_D, NORM_D, NORM_D, NORM_D, SPR_EF, NORM_D, NORM_D, SPR_EF, NORM_D },
	{ UNEFTV, UNEFTV, NORM_D, NORM_D, SPR_EF, NORM_D, NORM_D, SPR_EF, NORM_D, NORM_D, NORM_D, NORM_D, NORM_D, NORM_D, SPR_EF, UNEFTV, NORM_D, NORM_D },
	{ UNEFTV, NORM_D, NORM_D, NORM_D, UNEFTV, NORM_D, UNEFTV, NORM_D, NORM_D, UNEFTV, SPR_EF, UNEFTV, NORM_D, NORM_D, NORM_D, SPR_EF, UNEFTV, SPR_EF },
	{ UNEFTV, NORM_D, NORM_D, SPR_EF, NORM_D, UNEFTV, NORM_D, NORM_D, NORM_D, UNEFTV, IMMUNE, SPR_EF, NORM_D, NORM_D, NORM_D, SPR_EF, NORM_D, NORM_D },
	{ UNEFTV, SPR_EF, NORM_D, NORM_D, NORM_D, IMMUNE, NORM_D, NORM_D, SPR_EF, NORM_D, NORM_D, NORM_D, IMMUNE, UNEFTV, NORM_D, NORM_D, NORM_D, NORM_D },
	{ SPR_EF, NORM_D, NORM_D, UNEFTV, NORM_D, NORM_D, SPR_EF, SPR_EF, NORM_D, UNEFTV, UNEFTV, SPR_EF, NORM_D, SPR_EF, NORM_D, NORM_D, NORM_D, UNEFTV },
	{ NORM_D, NORM_D, NORM_D, IMMUNE, NORM_D, NORM_D, NORM_D, NORM_D, NORM_D, SPR_EF, NORM_D, SPR_EF, NORM_D, UNEFTV, NORM_D, UNEFTV, NORM_D, SPR_EF },
	{ NORM_D, NORM_D, NORM_D, NORM_D, NORM_D, SPR_EF, SPR_EF, NORM_D, NORM_D, NORM_D, NORM_D, UNEFTV, NORM_D, NORM_D, NORM_D, SPR_EF, SPR_EF, NORM_D },
	{ NORM_D, NORM_D, NORM_D, NORM_D, NORM_D, SPR_EF, NORM_D, NORM_D, IMMUNE, NORM_D, NORM_D, NORM_D, NORM_D, NORM_D, NORM_D, NORM_D, NORM_D, NORM_D },
	{ UNEFTV, NORM_D, NORM_D, NORM_D, UNEFTV, UNEFTV, NORM_D, NORM_D, NORM_D, UNEFTV, SPR_EF, NORM_D, NORM_D, UNEFTV, SPR_EF, NORM_D, NORM_D, NORM_D },
	{ SPR_EF, SPR_EF, NORM_D, NORM_D, NORM_D, UNEFTV, NORM_D, NORM_D, SPR_EF, NORM_D, NORM_D, NORM_D, NORM_D, NORM_D, UNEFTV, NORM_D, NORM_D, NORM_D },
	{ NORM_D, NORM_D, NORM_D, NORM_D, NORM_D, SPR_EF, UNEFTV, UNEFTV, NORM_D, SPR_EF, SPR_EF, NORM_D, UNEFTV, UNEFTV, NORM_D, NORM_D, SPR_EF, SPR_EF },
	{ UNEFTV, NORM_D, UNEFTV, NORM_D, UNEFTV, SPR_EF, SPR_EF, UNEFTV, NORM_D, UNEFTV, SPR_EF, UNEFTV, UNEFTV, IMMUNE, UNEFTV, UNEFTV, UNEFTV, NORM_D },
	{ NORM_D, NORM_D, NORM_D, SPR_EF, NORM_D, NORM_D, UNEFTV, NORM_D, NORM_D, SPR_EF, NORM_D, UNEFTV, NORM_D, NORM_D, NORM_D, NORM_D, UNEFTV, UNEFTV }
};

/* When to use special attack (and special defense) based on move type. */
const bool SP_MOVE[size_t(Types::NUM_TYPES)] =
{
	false,
	true,
	true,
	true,
	true,
	false,
	true,
	false,
	false,
	true,
	false,
	true,
	false,
	false,
	true,
	false,
	false,
	true
};

/*
	Returns the type multiplier when a given
	monster type is hit with a given move type.
*/
float Effective(Types targ, Types atk)
{
	if ((targ < Types(0)) || (atk < Types(0)) ||
		(targ >= Types::NUM_TYPES) || (atk >= Types::NUM_TYPES))
		return NORM_D;
	return EFTV_MAP[size_t(targ)][size_t(atk)];
}

/*
	Returns the type multiplier when both given
	monster types are hit with a given move type.
*/
float TypeEffectiveness(Types targ1, Types targ2, Types atk)
{
	float ret = NORM_D * Effective(targ1, atk);
	if (targ2 != Types::NORMAL)  // T2 = Normal when there's no second type.
		ret *= Effective(targ2, atk);
	return ret;
}

/* Use SP.ATK/DEF? */
bool SpecialType(Types mType)
{
	if ((mType >= Types(0)) && (mType < Types::NUM_TYPES))
	{
		return SP_MOVE[size_t(mType)];
	}
	return false;
}

std::string GetTypeName(Types typ)
{
	std::string myName = "Normal"; // This covers the "Default" case.
	switch (typ)
	{
	case Types::BUG:
		myName = "Bug";
		break;
	case Types::DARK:
		myName = "Dark";
		break;
	case Types::DRAGON:
		myName = "Dragon";
		break;
	case Types::ELECTRIC:
		myName = "Electric";
		break;
	case Types::FAIRY:
		myName = "Fairy";
		break;
	case Types::FIGHTING:
		myName = "Fighting";
		break;
	case Types::FIRE:
		myName = "Fire";
		break;
	case Types::FLYING:
		myName = "Flying";
		break;
	case Types::GHOST:
		myName = "Ghost";
		break;
	case Types::GRASS:
		myName = "Grass";
		break;
	case Types::GROUND:
		myName = "Ground";
		break;
	case Types::ICE:
		myName = "Ice";
		break;
	case Types::POISON:
		myName = "Poison";
		break;
	case Types::PSYCHIC:
		myName = "Psychic";
		break;
	case Types::ROCK:
		myName = "Rock";
		break;
	case Types::STEEL:
		myName = "Steel";
		break;
	case Types::WATER:
		myName = "Water";
	}
	return myName;
}

Types TypeFromName(std::string nme)
{
	const unsigned long long NUM_T = unsigned long long(Types::NUM_TYPES);

	nme = LoadString(nme, "invalid");

	std::string tNames[NUM_T] =
	{
		"Bug",
		"Dark",
		"Dragon",
		"Electric",
		"Fairy",
		"Fighting",
		"Fire",
		"Flying",
		"Ghost",
		"Grass",
		"Ground",
		"Ice",
		"Normal",
		"Poison",
		"Psychic",
		"Rock",
		"Steel",
		"Water"
	};

	/* Convert text to title case. */
	std::transform(nme.begin(), nme.begin() + 1, nme.begin(), std::toupper);

	for (auto i = 0; i < NUM_T; i++)
		if (nme == tNames[i])
			return Types(i);
	return Types::NORMAL;
}

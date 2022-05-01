#include "battle.h"
#include "moves.h"

Move::Move()
{
	name = "None";
	pp = power = accuracy =
		effect = 1;
	mType = (int)Types::NORMAL;
}

std::string Move::GetName()
{
	return name;
}
int Move::GetPP()
{
	return pp;
}
int Move::GetPower()
{
	return power;
}
int Move::GetAccuracy()
{
	return accuracy;
}
int Move::GetType()
{
	return mType;
}
int Move::GetEffect()
{
	return effect;
}

void Move::Setup(std::string nme, std::string typ,
	int p_p, int pwr, int acc, std::string fx)
{
	name = nme;
	power = pwr;
	pp = p_p;
	accuracy = acc;
	mType = (int)TypeFromName(typ);
	effect = LookupEffect(fx);

	/*
		Auto-assign Attack if no effect is given for a move with attack power.
	// */
	if ((effect == NO_EFFECT) && (power > 0))
		effect = ATTACK_EFFECT;
}

int LookupEffect(std::string eff)
{
	eff = LoadString(eff, "unknown");
	toupper(eff);

	std::string eNames[NUM_EFFECTS] =
	{
		"NOTHING",
		"ATTACK",
		"BLIND",
		"LESSATTACK",
		"LESSDEFENSE",
		"DEV",
		"SLOW"
	};

	for (auto i = 0; i < NUM_EFFECTS; i++)
		if (eff == eNames[i])
			return i;
	return NO_EFFECT; // Return the Nothing effect when no match is found.
}
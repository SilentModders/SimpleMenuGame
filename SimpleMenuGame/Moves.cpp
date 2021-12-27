#include "battle.h"
#include "moves.h"

Move::Move()
{
	name = "None";
	pp = power = accuracy = 1;
	mType = 12;
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

void Move::Setup(std::string nme, std::string typ,
	int p_p, int pwr, int acc)
{
	name = nme;
	power = pwr;
	pp = p_p;
	accuracy = acc;
	mType = (int)TypeFromName(typ);
}

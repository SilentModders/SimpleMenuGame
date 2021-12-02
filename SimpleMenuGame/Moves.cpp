#include "moves.h"

Move::Move()
{
	name = "";
	power = pp = accuracy = 1;
}

std::string Move::GetName()
{
	return name;
}
int Move::GetPower()
{
	return power;
}
int Move::GetPP()
{
	return pp;
}
int Move::GetAccuracy()
{
	return accuracy;
}

void Move::Setup(std::string nme, int pwr, int p_p, int acc)
{
	name = nme;
	power = pwr;
	pp = p_p;
	accuracy = acc;
}
#include <algorithm>
#include "text.h"
#include "enemies.h"

/*
	TODO: Create a Stat class to allow other stat systems.
//*/

Enemy::Enemy()
{
	name = "Placeholder";
	bHealth =
	bAttack =
	bDefense =
	bSpAtk =
	bSpDef =
	bSpeed =
	bXpYield = 40;
	bIndex =
	xpCurve = 0;
	catchRate =
	evolve = 255;
}
std::string Enemy::GetName()
{
	return ColoredString(name, Color::COLOR_WHITE);
}
int Enemy::GetIdNum()
{
	return bIndex;
}

int Enemy::GetBaseHealth()
{
	return bHealth;
}
int Enemy::GetBaseAttack()
{
	return bAttack;
}
int Enemy::GetBaseDefense()
{
	return bDefense;
}
int Enemy::GetBaseSpAtk()
{
	return bSpAtk;
}
int Enemy::GetBaseSpDef()
{
	return bSpDef;
}
int Enemy::GetBaseSpeed()
{
	return bSpeed;
}

int Enemy::GetXpCurve()
{
	return xpCurve;
}
int Enemy::GetXpYield()
{
	return bXpYield;
}
int Enemy::GetEvoLevel()
{
	return evolve;
}

void Enemy::Setup(std::string nme, int idx, int hlh,
	int atk, int def,
	int sat, int sdf,
	int spd, int xpc, int xpy,
	int crt, int evl)
{
	name = nme;
	bHealth = hlh;
	bAttack = atk;
	bDefense = def;
	bSpAtk = sat;
	bSpDef = sdf;
	bSpeed = spd;
	xpCurve = xpc;
	bXpYield = xpy;
	catchRate = crt;
	evolve = evl;
}

int Enemy::SetIdNum(int id)
{
	return bIndex = std::clamp(id, 0, MAX_ENEMIES);
}
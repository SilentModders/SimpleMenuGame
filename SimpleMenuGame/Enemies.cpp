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
	bSpeed = 40;
}
std::string Enemy::GetName()
{
	return ColoredString(name, Color::COLOR_WHITE);
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

void Enemy::Setup(std::string nme, int hlh, int atk, int def, int sat, int sdf, int spd)
{
	name = nme;
	bHealth = hlh;
	bAttack = atk;
	bDefense = def;
	bSpAtk = sat;
	bSpDef = sdf;
	bSpeed = spd;
}

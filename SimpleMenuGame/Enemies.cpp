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
	/* Default type is Normal.
	 * Type 2 can never be nomral,
	 * so use Normal to indicate
	 * no second type.
	//*/
	type[0] =
	type[1] = (int)Types::NORMAL;

	for (auto i = 0; i < MOVE_MEM; i++)
		myMoves[i] = "";
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
int Enemy::GetCatchRate()
{
	return catchRate;
}
int Enemy::GetEvoLevel()
{
	return evolve;
}
int Enemy::GetType(bool type1)
{
	if (type1)
		return type[0];
	return type[1];
}

void Enemy::Setup(std::string nme, int idx, int hlh,
	int atk, int def,
	int sat, int sdf,
	int spd, int xpc,
	int xpy, int crt,
	int type1, int type2,
	int evl)
{
	name = nme;
	bIndex = idx;
	bHealth = hlh;
	bAttack = atk;
	bDefense = def;
	bSpAtk = sat;
	bSpDef = sdf;
	bSpeed = spd;
	xpCurve = xpc;
	bXpYield = xpy;
	catchRate = crt;
	type[0] = type1;
	type[1] = type2;
	evolve = evl;
}

bool Enemy::AddMove(std::string nme, int lv)
{
	/* Too early to search the
	 * Game's Move dictionary,
	 * or is it?
	//*/
	if (nme.length() < 2)
		return false;
	lv = std::clamp(lv, 1, MAX_LEVEL);

	moveMap.insert(std::pair<std::string, int>(nme, lv));

	return true;
}

void Enemy::BuildMoveList(int lv)
{
	/* Reads all moves the that
	 * this creature can learn
	 * and overwrites the oldest.
	//*/
	int i = 0;
	for (auto&& item : moveMap)
	{
		if (item.second <= lv)
			myMoves[i % 4] = item.first;
		i++;
	}
}

std::map<std::string, int> Enemy::GetAllMoves()
{
	return moveMap;
}

std::string Enemy::MoveName(int idx)
{
	if (myMoves[0] == "")
		BuildMoveList();
	idx = std::clamp(idx, 0, MOVE_MEM);
	return myMoves[idx];
}

int Enemy::SetIdNum(int id)
{
	return bIndex = std::clamp(id, 0, MAX_ENEMIES);
}

std::string StatName(int st)
{
	std::string names[NUM_STATS] =
	{
		"Health", // Still not HP
		"Attack",
		"Defense",
		"Special Attack",
		"Special Defense",
		"Speed"
	};

	if ((st < 0) || (st >= NUM_STATS))
		return names[2];
	return names[st];
}

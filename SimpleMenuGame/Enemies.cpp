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
	type[1] = Types::NORMAL;

	for (auto h = 0; h < NUM_DEBUFFS; h++)
	{
		debuffs[h] = false;
	}

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
Types Enemy::GetType(bool type1)
{
	return type[!type1];
}

bool Enemy::SetType(Types typ, bool type1)
{
	const Types oldT = type[!type1];
	type[!type1] = typ;
	return oldT != type[!type1];
}

bool Enemy::GetDebuff(int buff)
{
	if (buff > 0 && buff <= NUM_DEBUFFS)
	{
		return debuffs[buff];
	}
	return false;
}
bool Enemy::Asleep()
{
	return GetDebuff(SLEEP);
}
bool Enemy::Burned()
{
	return GetDebuff(BURN);
}
bool Enemy::Frozen()
{
	return GetDebuff(FREEZE);
}
bool Enemy::Paralyzed()
{
	return GetDebuff(PARALYZE);
}
bool Enemy::Poisoned()
{
	return GetDebuff(POISON);
}
bool Enemy::Seeded()
{
	return GetDebuff(SEED);
}

bool Enemy::SetDebuff(int buff, bool state)
{
	if (buff > 0 && buff <= NUM_DEBUFFS)
	{
		debuffs[buff] = state;
		return true;
	}
	return false;
}
void Enemy::ClearDebuffs()
{
	for (int i = 0; i < NUM_DEBUFFS; i++)
	{
		debuffs[i] = false;
	}
}

void Enemy::Burn(bool state)
{
	SetDebuff(BURN, state);
}
void Enemy::Freeze(bool state)
{
	SetDebuff(FREEZE, state);
	if (!state)
	{
		SetDebuff(SLEEP, false);
		SetDebuff(PARALYZE, false);
	}
}
void Enemy::Paralyze(bool state)
{
	SetDebuff(PARALYZE, state);
	if (!state)
	{
		SetDebuff(FREEZE, false);
		SetDebuff(SLEEP, false);
	}
}
void Enemy::Poison(bool state)
{
	SetDebuff(POISON, state);
}
void Enemy::Seed(bool state)
{
	SetDebuff(SEED, state);
}
void Enemy::Sleep(bool state)
{
	SetDebuff(SLEEP, state);
	if (!state)
	{
		SetDebuff(FREEZE, false);
		SetDebuff(PARALYZE, false);
	}
}

void Enemy::Setup(std::string nme, int idx, int hlh,
	int atk, int def,
	int sat, int sdf,
	int spd, int xpc,
	int xpy, int crt,
	int evl,
	Types type1, Types type2)
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
	int m = 0;
	for (auto&& item : moveMap)
	{
		if (item.second <= lv)
		{
			myMoves[m] = item.first;
			m++;
			if (m >= MOVE_MEM)
			{
				m = 0;
			}
		}
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

#include "text.h"
#include <algorithm>
#include "party.h"

PartyMember::PartyMember()
{
	nickname = "";
	health =
	attack =
	defense =
	spAtk =
	spDef =
	speed = 10;
	myLevel = hitP = totalHP = 1;
	for (auto i = 0; i < NUM_STATS; i++)
		iv[i] = ev[i] = 0;
}
std::string PartyMember::GetNickname()
{
	return ColoredString(nickname, Color::COLOR_WHITE);
}
int PartyMember::GetStat(int st)
{
	int ret = attack;
	switch (st)
	{
	case HEALTH:
		ret = health;
		break;
	/* case ATTACK is the default value. */
	case DEFENSE:
		ret = defense;
		break;
	case SATTACK:
		ret = spAtk;
		break;
	case SDEFENSE:
		ret = spDef;
		break;
	case SPEED:
		ret = speed;
	}
	return ret;
}
int PartyMember::GetHealth()
{
	return health;
}
int PartyMember::GetAttack()
{
	return attack;
}
int PartyMember::GetDefense()
{
	return defense;
}
int PartyMember::GetSpAtk()
{
	return spAtk;
}
int PartyMember::GetSpDef()
{
	return spDef;
}
int PartyMember::GetSpeed()
{
	return speed;
}
int PartyMember::GetHP()
{
	return hitP;
}

int PartyMember::GetTotalHP()
{
	return totalHP;
}

bool PartyMember::Create(Enemy* basetype, int level)
{
	for (auto i = 0; i < NUM_STATS; i++)
	{
		iv[i] = rand() % MAX_IV;
	}
	Create(basetype, level, iv[HEALTH], iv[ATTACK], iv[DEFENSE], iv[SATTACK], iv[SDEFENSE], iv[SPEED]);
	return true;
}
bool PartyMember::Create(Enemy* basetype, int level, int healthIV, int attackIV, int defenseIV, int spAttackIV, int spDefenseIV, int speedIV)
{
	if (!basetype)
		return false;

	level = std::clamp(level, 1, MAX_LEVEL);

	Setup(
		basetype->GetName(),
		basetype->GetBaseHealth(),
		basetype->GetBaseAttack(),
		basetype->GetBaseDefense(),
		basetype->GetBaseSpAtk(),
		basetype->GetBaseSpDef(),
		basetype->GetBaseSpeed());
	nickname = GetName();

	// Base Stats
	int base[NUM_STATS] = { 0 };
	base[HEALTH] = GetBaseHealth();
	base[ATTACK] = GetBaseAttack();
	base[DEFENSE] = GetBaseDefense();
	base[SATTACK] = GetBaseSpAtk();
	base[SDEFENSE] = GetBaseSpDef();
	base[SPEED] = GetBaseSpeed();

	int stat[NUM_STATS] = { 0 };

	// Calc Stats
	for (auto i = 0; i < NUM_STATS; i++)
	{
		iv[i] = std::clamp(iv[i], 0, MAX_IV);
		ev[i] = 0; // Starts at 0
		stat[i] = ((2 * base[i] + iv[i] + ev[i] / 4) * level) / 100 + 5;
	}

	health = stat[HEALTH];
	attack = stat[ATTACK];
	defense = stat[DEFENSE];
	spAtk = stat[SATTACK];
	spDef = stat[SDEFENSE];
	speed = stat[SPEED];

	// HP is a stat with a different formula.
	totalHP = ((2 * base[HEALTH] + iv[HEALTH] + ev[HEALTH] / 4 + 100) * level) / 100 + 10;
	hitP = totalHP;

	return true;
}

void PartyMember::SetHP(int hp)
{
	hitP = std::clamp(hp, 0, totalHP);
}

int PartyMember::GetLevel()
{
	return myLevel;
}
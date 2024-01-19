#include "text.h"
#include <algorithm>
#include "party.h"
#include "game.h"
#include "battle.h"

int ipow(int a, int b) { return int(pow(a, b)); }

constexpr int MAX_EV = 65535;

const enum XP_CURVES
{
	XP_SLOW,
	XP_MED_SLOW,
	XP_MED_FAST,
	XP_FAST,
	NUM_XP_TYPES
};

PartyMember::PartyMember(Game* mygame)
{
	created = false;
	myGame = mygame;
	nickname = "";
	health =
	attack =
	defense =
	spAtk =
	spDef =
	speed = 10;
	curXp = 0;
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

int PartyMember::GetLevel()
{
	return myLevel;
}

int PartyMember::XpForLevel(int level, int curve)
{
	if (level <= 1)
		return 0;

	switch (curve)
	{
	case XP_FAST:
		return 4 * ipow(level, 3) / 5;
	case XP_MED_FAST:
		return ipow(level, 3);
	default:
	case XP_MED_SLOW:
		return int(6.0f / 5 * ipow(level, 3) - 15 * ipow(level, 2) + 100 * level - 140);
	case XP_SLOW:
		return 5 * ipow(level, 3) / 4;
	}
}

void PartyMember::AwardEV(int gain, int st)
{
	if (gain <= 0)
		return;
	
	if (st < NUM_STATS)
		ev[st] = std::clamp(ev[st] + gain, 0, MAX_EV);
	CalcStats();
}

bool PartyMember::AwardXP(int xp)
{
	bool leveledUp = false;

	if (xp <= 0)
		return false;

	if (myLevel >= MAX_LEVEL)
	{
		/* EVs can be earned
		 * for a long time.
		//*/
		CalcStats();
		return false;
	}

	curXp += xp;

	while (XpForLevel(myLevel + 1, GetXpCurve()) <= curXp)
	{
		leveledUp = true;
		myLevel++;
		std::cout << nickname << " reached level " << myLevel << "!" << std::endl;
		if (myLevel >= GetEvoLevel())
		{
			std::cout << nickname << " is evolving!" << std::endl;
			std::string oldName = nickname;
			bool nicknamed = (nickname != GetName());
			SetIdNum(GetIdNum() + 1);
			SetBaseType(GetIdNum());
			std::cout << oldName << " evolved into " << GetName() << "!" << std::endl;
			if (!nicknamed)
				nickname = GetName();
		}
		// FIXME: Check for new moves based on level/evolution.
	}

	/* HP is calculated each time
	 * because EVs can raise the HP.
	//*/
	float hpFrac = float(hitP / totalHP);
	CalcStats();
	SetHP(int(hpFrac * totalHP));

	return true;
}

bool PartyMember::Create(int basetype, int level)
{
	for (auto i = 0; i < NUM_STATS; i++)
	{
		iv[i] = random_int(1, MAX_IV);
	}
	Create(basetype, level, iv[HEALTH], iv[ATTACK_STAT], iv[DEFENSE], iv[SATTACK], iv[SDEFENSE], iv[SPEED]);
	return true;
}
bool PartyMember::Create(int basetype, int level, int healthIV, int attackIV, int defenseIV, int spAttackIV, int spDefenseIV, int speedIV)
{
	if (!SetBaseType(basetype))
		return false;
	
	nickname = GetName();

	myLevel = std::clamp(level, 1, MAX_LEVEL);
	curXp = XpForLevel(myLevel, GetXpCurve());

	CalcStats();
	hitP = totalHP;
	
	BuildMoveList(level);

	return true;
}

bool PartyMember::SetBaseType(int idx)
{
	Enemy* basetype = myGame->GetCombatSys()->EnemyFromIndex(idx);

	if (!basetype)
		return false;

	/* BUGBUG: If all stats
	 * reload when evolving,
	 * what if a stat was
	 * raised by a powerup?
	//*/
	Setup(
		basetype->GetName(),
		idx,
		basetype->GetBaseHealth(),
		basetype->GetBaseAttack(),
		basetype->GetBaseDefense(),
		basetype->GetBaseSpAtk(),
		basetype->GetBaseSpDef(),
		basetype->GetBaseSpeed(),
		basetype->GetXpCurve(),
		basetype->GetXpYield(),
		basetype->GetCatchRate(),
		basetype->GetEvoLevel());

	// Get Move List
	if (!created)
	{
		std::map<std::string, int> mMap = basetype->GetAllMoves();
		for (auto&& item : mMap)
		{
			AddMove(item.first, item.second);
		}
		created = true;
	}

	return true;
}

void PartyMember::CalcStats()
{
	// Base Stats
	int base[NUM_STATS] = { 0 };
	base[HEALTH]  = GetBaseHealth();
	base[ATTACK_STAT]  = GetBaseAttack();
	base[DEFENSE] = GetBaseDefense();
	base[SATTACK] = GetBaseSpAtk();
	base[SDEFENSE]= GetBaseSpDef();
	base[SPEED]   = GetBaseSpeed();

	int stat[NUM_STATS] = { 0 };

	// Calc Stats
	for (auto i = 0; i < NUM_STATS; i++)
		stat[i] = ((2 * base[i] + iv[i] + ev[i] / 4) * myLevel) / 100 + 5;

	health = stat[HEALTH];
	attack = stat[ATTACK_STAT];
	defense= stat[DEFENSE];
	spAtk  = stat[SATTACK];
	spDef  = stat[SDEFENSE];
	speed  = stat[SPEED];

	// HP is a stat with a different formula.
	totalHP = ((2 * base[HEALTH] + iv[HEALTH] + ev[HEALTH] / 4 + 100) * myLevel) / 100 + 10;
}

void PartyMember::SetHP(int hp)
{
	hitP = std::clamp(hp, 0, totalHP);
}

void PartyMember::Heal()
{
	hitP = totalHP;
	ClearDebuffs();
}

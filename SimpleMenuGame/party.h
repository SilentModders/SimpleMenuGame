#pragma once
#include "enemies.h"

constexpr int MAX_IV = 15;

class PartyMember: public Enemy
{
public:
	PartyMember();
	std::string GetNickname();
	int GetStat(int st = ATTACK);
	int GetHealth();
	int GetAttack();
	int GetDefense();
	int GetSpAtk();
	int GetSpDef();
	int GetSpeed();
	int GetHP();
	int GetTotalHP();
	int GetLevel();

	// Calculate Stats
	bool Create(Enemy* basetype, int level);
	bool Create(Enemy* basetype, int level,
		int healthIV, int attackIV, int defenseIV, int spAttackIV, int spDefenseIV, int speedIV);

	void SetHP(int hp = 0);

private:
	std::string nickname;

	// IVs and EVs
	int iv[NUM_STATS], ev[NUM_STATS];

	// Calculated Stats
	int health; // Health (Not HP)
	int attack; // Attack
	int defense; // Defense
	int spAtk; // Special Attack
	int spDef; // Special Defense
	int speed; // Speed
	int myLevel; // Level
	int hitP, totalHP; // HP
};
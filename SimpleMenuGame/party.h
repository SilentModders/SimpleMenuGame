#pragma once
#include "enemies.h"

constexpr int MAX_IV = 15;

class Game;

class PartyMember : public Enemy
{
public:
	PartyMember(Game* mygame);
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

	int XpForLevel(int level, int curve);

	void AwardEV(int gain = 1, int st = ATTACK);
	bool AwardXP(int xp = 0);

	bool Create(int basetype, int level);
	bool Create(int basetype, int level,
		int healthIV, int attackIV, int defenseIV, int spAttackIV, int spDefenseIV, int speedIV);

	void SetHP(int hp = 0);

private:
	bool created;

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
	int curXp; // Experience

	std::string nickname;

	Game* myGame; // The Game

	bool SetBaseType(int idx);

	void CalcStats();
};

#pragma once
#include <string>

constexpr auto MAX_ENEMIES = 1000;
constexpr auto MAX_LEVEL = 100;

class Enemy
{
public:
	Enemy();
	std::string GetName();
	int GetBaseHealth();
	int GetBaseAttack();
	int GetBaseDefense();
	int GetBaseSpAtk();
	int GetBaseSpDef();
	int GetBaseSpeed();

	void Setup(std::string name, int hlh = 10, int atk = 5, int def = 5, int sat = 5, int sdf = 5, int spd = 5);
private:
	std::string name;
	// Base Stats
	int bHealth; // Health (Not HP)
	int bAttack; // Attack
	int bDefense; // Defense
	int bSpAtk; // Special Attack
	int bSpDef; // Special Defense
	int bSpeed; // Speed
};

enum stats
{
	HEALTH,
	ATTACK,
	DEFENSE,
	SATTACK,
	SDEFENSE,
	SPEED,
	NUM_STATS
};
#pragma once
#include <string>
#include <map>
#include "moves.h"
#include "types.h"

constexpr auto MAX_ENEMIES = 1000;
constexpr auto MAX_LEVEL = 100;

// A monster can only know this many moves.
constexpr auto MOVE_MEM = 4;

const enum DEBUFFS
{
	BURN,
	FREEZE,
	PARALYZE,
	POISON,
	SEED,
	SLEEP,
	NUM_DEBUFFS
};

class Enemy
{
public:
	Enemy();
	std::string GetName();
	int GetIdNum();
	int GetBaseHealth();
	int GetBaseAttack();
	int GetBaseDefense();
	int GetBaseSpAtk();
	int GetBaseSpDef();
	int GetBaseSpeed();
	int GetXpCurve();
	int GetXpYield();
	int GetCatchRate();
	int GetEvoLevel();
	Types GetType(bool type1 = true);

	// Always sets the type, but returns false if they were already the same. //
	bool SetType(Types typ = Types::NORMAL, bool type1 = true);

	bool Asleep();
	bool Burned();
	bool Frozen();
	bool Paralyzed();
	bool Poisoned();
	bool Seeded();

	void ClearDebuffs();

	void Burn(bool state = true);
	void Freeze(bool state = true);
	void Paralyze(bool state = true);
	void Poison(bool state = true);
	void Seed(bool state = true);
	void Sleep(bool state = true);

	// Load all base stats
	void Setup(std::string name,
		int idx = 0,
		int hlh = 10,
		int atk = 5, int def = 5,
		int sat = 5, int sdf = 5,
		int spd = 5,
		int xpc = 0,
		int xpy = 5,
		int crt = 255,
		int evl = 255,
		Types type1 = Types::NORMAL,
		Types type2 = Types::NORMAL);

	// Add a move the this monster's total list.
	bool AddMove(std::string nme, int lv = 1);

	// Gives this enemy the newest moves for that level
	void BuildMoveList(int lv = 1);

	std::map<std::string, int> GetAllMoves();

	// Get the specified move, out of 4.
	std::string MoveName(int idx = 0);

protected:
	// Don't call this without reloading Stats!
	int SetIdNum(int id);

private:
	bool GetDebuff(int buff = SLEEP);
	bool SetDebuff(int buff = SLEEP, bool state = true);

	std::string name;
	int bIndex; // Index Number
	// Base Stats
	int bHealth; // Health (Not HP)
	int bAttack; // Attack
	int bDefense; // Defense
	int bSpAtk; // Special Attack
	int bSpDef; // Special Defense
	int bSpeed; // Speed
	int xpCurve; // Growth Rate
	int bXpYield; // XP Yield when beaten
	int catchRate; // Catch Rate
	int evolve; // Level to evolve
	Types type[2]; // Monster Types
	bool debuffs[NUM_DEBUFFS]; // Status Ailments

	// All moves
	std::map<std::string, int> moveMap;

	// Known Moves, Not always loaded
	std::string myMoves[MOVE_MEM];
};

const enum STATS
{
	HEALTH, // Not HP
	ATTACK_STAT,
	DEFENSE,
	SATTACK, // Special Attack
	SDEFENSE, // Special Defense
	SPEED,
	NUM_STATS
};
std::string StatName(int st);

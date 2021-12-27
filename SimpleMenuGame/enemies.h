#pragma once
#include <string>
#include <map>
#include "moves.h"

constexpr auto MAX_ENEMIES = 1000;
constexpr auto MAX_LEVEL = 100;

// A monster can only know this many moves.
constexpr auto MOVE_MEM = 4;

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
		int evl = 255);

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

	// All moves
	std::map<std::string, int> moveMap;

	// Known Moves, Not always loaded
	std::string myMoves[4];
};

const enum Stats
{
	HEALTH,
	ATTACK,
	DEFENSE,
	SATTACK,
	SDEFENSE,
	SPEED,
	NUM_STATS
};
#pragma once
#include "game.h"
#include "moves.h"

constexpr auto MAX_MOVES = 1000;

class CombatSys
{
public:
	CombatSys(Game* gameObj);
	bool InBattle();
	bool BattleTurn(); // Runs Every Turn
	bool ReadFile(std::string file);
	bool ReadMoveFile(std::string file);
	Enemy* EnemyFromIndex(int index = 0);
private:
	int eHp; // Enemy HP
	int pHp; // Player HP
	int totalEhp, totalPhp;
	Enemy* opponent; // The Battle combatant
	PartyMember* partyMember; // The Player's guy

	bool bStarted; // Is the battle active?
	Game* theGame;
	/* All Enemy Types */
	Enemy* enemies[MAX_ENEMIES];
	int eCount;

	/* All Moves */
	Move* moveList[MAX_MOVES];
	int mCount;

	// Enemy Individual Values
	int eIv[NUM_STATS];
	// Enemy Stat Values
	int eStat[NUM_STATS];
	// Player Stat Values, with any temporary changes/
	int pStat[NUM_STATS];

	/*
		Find the first available party member.
		Returns false if there is none.
	*/
	bool FindPartyMember();

	// Use Special Attack and Special Defense when applicable.
	int CalcDamage(int level, int power, int attackStat, int defenseStat);

	void CalcStats(int index = 0);
	void PrintHealth();
	void StartBattle();
	void EndBattle();
};

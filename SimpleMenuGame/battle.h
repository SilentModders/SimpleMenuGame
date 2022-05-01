#pragma once
#include "game.h"
#include "moves.h"
#include "types.h"

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
	int eLevel;
	int eHp; // Enemy HP
	int pHp; // Player HP
	int totalEhp, totalPhp;
	int eIndex; // Enemy Index
	Enemy* opponent; // The Battle combatant
	PartyMember* partyMember; // The Player's guy
	Game* theGame;

	bool enemyTurn; // Whose stats are read.
	bool bStarted; // Is the battle active?
	bool trainerBattle; // Is the foe a trainer?
	bool participated[PARTYSIZE]; // Which part memebers fought here?

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

	// Battle Stats: Accuracy and Evasiveness
	int eAcc, eEvas,
		pAcc, pEvas;

	/*
		Find the first available party member.
		Returns false if there is none.
	*/
	bool FindPartyMember();

	// Should the move hit?
	bool HitChance(int mAcc = 95);

	// Use Special Attack and Special Defense when applicable.
	int CalcDamage(Move* mov);

	int CalcExp();

	void CalcStats(int index, int level);
	void PrintHealth();
	void StartBattle();
	void EndBattle();
	void MoveAction(std::string mov);

	Move* MovesByName(std::string mv);
};


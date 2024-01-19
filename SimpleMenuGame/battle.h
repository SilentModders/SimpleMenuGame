#pragma once
#include "game.h"
#include "moves.h"
#include "types.h"

constexpr auto MAX_MOVES = 1000;
constexpr auto MAX_COMBATANTS = 2; // May become 4.

const enum TURNS
{
	ASLEEP,
	LONG_MOVE,
	CONFUSED,
	FLINCHING, // Always just one
	SCREENED, // Buffed S.Def. (Light Screen)
	REFLECTING, // Buffed Def. (Reflect)
	NUM_COUNTDOWNS
};

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
	bool Transform(); // For the Transform move. Currently cannot fail.
	int eLevel;
	int eHp; // Enemy HP
	int pHp; // Player HP
	int totalEhp, totalPhp;
	int eIndex; // Enemy Index
	Enemy* opponent; // The Battle combatant
	PartyMember* partyMember; // The Player's guy
	Game* theGame;

	bool reportCrit; // Kept for the summary print
	bool enemyTurn; // Whose stats are read.
	bool bStarted; // Is the battle active?
	bool trainerBattle; // Is the foe a trainer?
	bool participated[PARTYSIZE]; // Which party members fought here?

	/* All Enemy Types */
	Enemy* enemies[MAX_ENEMIES];
	int eCount;

	/* All Moves */
	Move* moveList[MAX_MOVES];
	int mCount;

	std::string eMoves[MOVE_MEM], pMoves[MOVE_MEM];
	int eMvCount, pMvCount;

	// Enemy Individual Values
	int eIv[NUM_STATS];

	// Enemy Stat Values
	int eStat[NUM_STATS];

	// Player Stat Values, with any temporary changes/
	int pStat[NUM_STATS];

	// Battle Stats: Accuracy, Evasiveness, Crit Chance
	int eAcc, eEvas, eCritC,
		pAcc, pEvas, pCritC;

	/* How many turns will the burn, poison, etc, last?
	 * 0 for player, 1 for enemy.
	//*/
	int debuffTurns[MAX_COMBATANTS][NUM_COUNTDOWNS];

	/* How many times has "Bad Poison" gotten worse? */
	int bPoisonNum[MAX_COMBATANTS];

	/* Prevent Stat Reductions */
	bool statGuarded[MAX_COMBATANTS];

	/* Rage Counter */
	int rageCount[MAX_COMBATANTS];

	/* "Locked" in a multi-turn move. E.g. Fly */
	bool midMove[MAX_COMBATANTS];
	bool hidden[MAX_COMBATANTS];

	/* Pre-substitute HP to put back when it breaks. */
	int pSubstituteHP[MAX_COMBATANTS];
	bool substituted[MAX_COMBATANTS];

	/* What moves were used last turn? */
	std::string lastAttack[MAX_COMBATANTS];
	/* How much damage was taken? */
	int lastDamage[MAX_COMBATANTS];

	int sCoins; // Scattered coins

	/*
		Find the first available party member.
		Returns false if there is none.
	*/
	bool FindPartyMember();

	// Should the move hit?
	bool HitChance(int mAcc = 95);

	// Calculate move damage
	int CalcDamage(Move* mov, bool noCrits = false);

	int CalcExp();

	void CalcStats(int index, int level);
	void PrintHealth();
	void StartBattle();
	void EndBattle();
	void MoveAction(std::string mov);

	Move* MovesByName(std::string mv);
};

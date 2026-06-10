#pragma once
#include "game.h"
#include "moves.h"
#include "types.h"

constexpr auto MAX_MOVES = 1000;
constexpr auto MAX_COMBATANTS = 2; // May become 4.

enum TURNS
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
	int eMember; // Which enemy slot is active?
	int ePartySz; // Enemy party size
	PartyMember* eParty[PARTYSIZE]; // All combatants
	PartyMember* partyMember; // The Player's guy
	Game* theGame;
	encounterData* encZone;

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

	// Enemy Stat Values
	int eStat[NUM_STATS];

	// Player Stat Values
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

	/* Need to remind player what level the opponent is. */
	bool recentReset;

	/* What moves were used last turn? */
	std::string lastAttack[MAX_COMBATANTS];
	/* How much damage was taken? */
	int lastDamage[MAX_COMBATANTS];

	/* Saved Enemy HP (for switching) */
	int savedHP[PARTYSIZE];

	int sCoins; // Scattered coins

	/*
		Find the first available party member.
		Returns false if there is none.
	*/
	bool FindPartyMember();
	bool SwitchPartyMember(int mem = 0, bool stdMsg = true);
	bool EnemySwitch(int mem = 0);

	// Should the move hit?
	bool HitChance(int mAcc = 95);

	// Calculate move damage
	int CalcDamage(Move* mov, bool noCrits = false);

	int CalcExp();

	int GetLevel(int eChoice);

	int SearchEnemies();

	void CalcStats(int index, int level);
	void PrintHealth();
	void StartBattle(bool sameEnc = false); // The flag is for trainer battles with more monsters.
	void EndBattle();
	void MoveAction(std::string mov);
	void ResetEnemy();

	Move* MovesByName(std::string mv);
};

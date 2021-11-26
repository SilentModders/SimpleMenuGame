#pragma once
#include "game.h"

class CombatSys
{
public:
	CombatSys(Game* gameObj);
	bool BattleTurn(); // Runs Every Turn
	bool ReadFile(std::string file);
	Enemy* EnemyFromIndex(int index = 0);
private:
	int member; // Player Party Member
	int pHealth; // Player Health
	int eHealth; // Enemy Health
	bool bStarted; // Is the battle active?
	Game* theGame;
	/* All Enemy Types */
	Enemy* enemies[MAX_ENEMIES];
	int eCount;

	/*
		Find the first available party member.
		Returns false if there is none.
	*/
	bool FindPartyMember();

	void PrintHealth();
	void StartBattle();
	void EndBattle();
};

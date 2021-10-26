#pragma once
#include "game.h"

class CombatSys
{
public:
	CombatSys(Game* gameObj);
	bool BattleTurn(); // Runs Every Turn
	bool ReadFile();
	Enemy* EnemyFromIndex(int index = 0);
private:
	int pHealth; // Player Health
	int eHealth; // Enemy Health
	bool bStarted; // Is the battle active?
	Game* theGame;
	/* All Enemy Types */
	Enemy* enemies[MAX_ENEMIES];
	int eCount;

	void StartBattle();
	void EndBattle();
	void AddEnemy(std::string key, int hp = 1, int dam = 1);
};

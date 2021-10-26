#include "enemies.h"
#include "battle.h"

/* Player heals at the end of each battle */
constexpr bool AUTOHEAL = true;

constexpr int OPPONENT = 1;

std::string ColoredHp(int cur, int max)
{
	Color color = Color::COLOR_GREEN;
	float frac = float(cur / max);
	if (frac >= 33.3f)
		color = Color::COLOR_RED;
	else if (frac > 66.7f)
		color = Color::COLOR_YELLOW;
	return ColoredString(std::to_string(cur), color);
}

CombatSys::CombatSys(Game* gameObj)
{
	eHealth = pHealth = 20;
	bStarted = false;
	theGame = gameObj;
	eCount = 0;
	/* Fill the array with safe data. */
	for (auto i = 0; i < MAX_ENEMIES; i++)
		enemies[i] = new Enemy();
}

Enemy* CombatSys::EnemyFromIndex(int index)
{
	return enemies[std::clamp(index, 0, eCount)];
}

void CombatSys::StartBattle()
{
	std::cout << "A wild " << enemies[OPPONENT]->GetName() << " appeared." << std::endl;
	if (!theGame->PartyMember(0))
	{
		std::cout << "You somehow have no party so you ran." << std::endl;
		EndBattle();
		return;
	}
	std::cout << "You sent out " << theGame->PartyMember(0)->GetName() << "." << std::endl;

	pHealth = theGame->PartyMember(0)->GetHealth();
	eHealth = enemies[OPPONENT]->GetHealth();
	std::cout << "You have " << ColoredHp(pHealth, theGame->PartyMember(0)->GetHealth()) << " hit points." << std::endl;
	std::cout << "Enemy has " << ColoredHp(eHealth, enemies[OPPONENT]->GetHealth()) << " hit points." << std::endl;
}

void CombatSys::EndBattle()
{
	theGame->SetRoom(theGame->GetLastRoom());
	std::cout << std::endl;
	bStarted = false;
}

/* Process one turn in combat. Returns whether the battle is over. */
bool CombatSys::BattleTurn()
{
	if (!bStarted)
	{
		StartBattle();
		bStarted = true;
		return false;
	}

	if (theGame->GetChoice() == "ATTACK")
	{
		std::cout << "You attacked for " << theGame->PartyMember(0)->GetDamage() << " hit points." << std::endl;
		eHealth -= theGame->PartyMember(0)->GetDamage();
	}
	/* Decide to attack */
	if (rand() % 100 <= 50)
    {
        std::cout << "You were attacked for " << enemies[OPPONENT]->GetDamage() << " hit points." << std::endl;
        pHealth -= enemies[OPPONENT]->GetDamage();
    }
    else
        std::cout << "The enemy growled or something." << std::endl;

	std::cout << "You have " << pHealth << " hit points." << std::endl;
	std::cout << "Enemy has " << eHealth << " hit points." << std::endl;
    std::cout << std::endl;

	if (pHealth <= 0)
	{
		std::cout << "You lost!" << std::endl;
		EndBattle();
		return true;
	}
	if (eHealth <= 0)
	{
		std::cout << "You won!" << std::endl;
		EndBattle();
		return true;
	}
	return false;
}

#include "enemies.h"
#include "battle.h"

/* Player heals at the end of each battle */
constexpr bool AUTOHEAL = false;

constexpr int OPPONENT = 1;

std::string ColoredHp(int cur, int max)
{
	Color color = Color::COLOR_RED;
	float frac = 0;
	if (max)
		frac = float(cur / max);
	if (frac > 0.33f)
		color = Color::COLOR_YELLOW;
	if (frac > 0.67f)
		color = Color::COLOR_GREEN;
	return ColoredString(std::to_string(cur), color);
}

void CombatSys::PrintHealth()
{
	std::cout << "You have " << ColoredHp(pHealth, theGame->PartyMember(0)->GetHealth()) << " hit points." << std::endl;
	std::cout << "Enemy has " << ColoredHp(eHealth, enemies[OPPONENT]->GetHealth()) << " hit points." << std::endl;
}

CombatSys::CombatSys(Game* gameObj)
{
	eHealth = pHealth = 20;
	bStarted = false;
	theGame = gameObj;
	eCount = member = 0;
	/* Fill the array with safe data. */
	for (auto i = 0; i < MAX_ENEMIES; i++)
		enemies[i] = new Enemy();
}

Enemy* CombatSys::EnemyFromIndex(int index)
{
	return enemies[std::clamp(index, 0, eCount)];
}

bool CombatSys::FindPartyMember()
{
	/* Search for a party member that isn't down. */
	for (auto i = 0; i < 6; i++)
		/* Is the party slot occupied? */
		if (theGame->PartyMember(i))
			/* Is the party member incapaciated? */
			if (theGame->GetHealth(i) > 0)
			{
				member = i;
				return true;
			}
	return false;
}

void CombatSys::StartBattle()
{
	std::cout << "A wild " << enemies[OPPONENT]->GetName() << " appeared." << std::endl;

	if (!FindPartyMember())
	{
		std::cout << "You have no party so you ran." << std::endl;
		EndBattle();
		return;
	}
	std::cout << "You sent out " << theGame->PartyMember(member)->GetName() << "." << std::endl;

	pHealth = theGame->GetHealth(member);
	eHealth = enemies[OPPONENT]->GetHealth();
	PrintHealth();
}

void CombatSys::EndBattle()
{
	if (!AUTOHEAL)
		theGame->SaveHealth(member, std::max(0, pHealth));
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

	/* The enemy only gets a turn when you enter a valid command like ATTACK or BAG */
	if (theGame->GetChoice() == "ATTACK")
	{
		std::cout << "You attacked for " << theGame->PartyMember(member)->GetDamage() << " hit points." << std::endl;
		eHealth -= theGame->PartyMember(0)->GetDamage();

		/* Decide to attack */
		if (rand() % 100 <= 60)
		{
			std::cout << "You were attacked for " << enemies[OPPONENT]->GetDamage() << " hit points." << std::endl;
			pHealth -= enemies[OPPONENT]->GetDamage();
		}
		else
			std::cout << "The enemy growled or something." << std::endl;
	}

	PrintHealth();
    std::cout << std::endl;

	if (pHealth <= 0)
	{
		std::cout << theGame->PartyMember(member)->GetName() << " can no longer fight!" << std::endl;
		if (!AUTOHEAL)
			theGame->SaveHealth(member, 0);

		if (FindPartyMember())
			std::cout << "You sent out " << theGame->PartyMember(member)->GetName() << "." << std::endl;
		else
		{
			std::cout << "You lost!" << std::endl;
			EndBattle();
			return true;
		}
	}
	if (eHealth <= 0)
	{
		std::cout << "You won!" << std::endl;
		EndBattle();
		return true;
	}
	return false;
}

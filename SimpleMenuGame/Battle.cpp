#include "enemies.h"
#include "battle.h"

/* The player heals at the end of each battle? */
constexpr bool AUTOHEAL = false;

constexpr int OPPONENT = 1;
constexpr int LV = 5;

std::string ColoredHp(int cur, int max)
{
	Color color = Color::COLOR_RED;
	float frac = 0.0f;
	if (max)
		frac = float(cur) / (float)max;
	if (frac > 0.33f)
		color = Color::COLOR_YELLOW;
	if (frac > 0.67f)
		color = Color::COLOR_GREEN;
	return ColoredString(std::to_string(cur)+"/"+ std::to_string(max), color);
}

CombatSys::CombatSys(Game* gameObj)
{
	totalEhp = totalPhp =
		eHp = pHp = 10;
	bStarted = false;
	theGame = gameObj;
	mCount = eCount = 0;
	opponent = nullptr;
	partyMember = nullptr;
	/* Fill the arrays with safe data. */
	for (auto i = 0; i < MAX_ENEMIES; i++)
		enemies[i] = new Enemy();
	for (auto j = 0; j < NUM_STATS; j++)
		eIv[j] = eStat[j] = pStat[j] = 0;
	for (auto k = 0; k < MAX_MOVES; k++)
		moveList[k] = new Move;
}

bool CombatSys::InBattle()
{
	return bStarted;
}

Enemy* CombatSys::EnemyFromIndex(int index)
{
	return enemies[std::clamp(index, 0, eCount)];
}

void CombatSys::PrintHealth()
{
	std::cout << "Your " << partyMember->GetNickname() << " has " << ColoredHp(pHp, totalPhp) << " hit points." << std::endl;
	std::cout << "Enemy " << opponent->GetName() << " has " << ColoredHp(eHp, totalEhp) << " hit points." << std::endl;
}

void CombatSys::CalcStats(int index)
{
	// Base Stats
	int base[NUM_STATS] = { 0 };
	base[HEALTH]  = enemies[index]->GetBaseHealth();
	base[ATTACK]  = enemies[index]->GetBaseAttack();
	base[DEFENSE] = enemies[index]->GetBaseDefense();
	base[SATTACK] = enemies[index]->GetBaseSpAtk();
	base[SDEFENSE]= enemies[index]->GetBaseSpDef();
	base[SPEED]   = enemies[index]->GetBaseSpeed();

	const int EV = 0; // Always 0 for wild

	// Calc Stats
	for (auto i = 0; i < NUM_STATS; i++)
	{
		eIv[i] = rand() % MAX_IV;
		eStat[i] = ((2 * base[i] + eIv[i] + EV / 4) * LV) / 100 + 5;
	}

	// HP is a stat with a different formula.
	totalEhp = ((2 * base[HEALTH] + eIv[HEALTH] + EV / 4 + 100) * LV) / 100 + 10;
}

int CombatSys::CalcDamage(int level, int power, int attackStat, int defenseStat)
{
	float E = 1; // Targets, .75 when move effects multiple targets
	float W = 1; // Weather
	float G = 1; // Badge, Gen 2 only
	float C = 1; // Crit, 2 for a crit
	float R = 0.85f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (1 - 0.85f))); // Random
	float S = 1; // Same-Type Attack Bonus, 1.5 when the move is same type as one of the user's types.
	float T = 1; // Type: 0, (.25 or .5), 1, (2 or 4), depending on both the move's and target's types.
	float U = 1; // Burn, .5 when burned user is hit with physical move.
	float O = 1; // Other, for special cases
	return (((2 * level / 5 + 2) * power * attackStat / defenseStat) / 50 + 2)
		* E * W * G * C * R * S * T * U * O;
}

bool CombatSys::FindPartyMember()
{
	/* Search for a party member that isn't down. */
	for (auto i = 0; i < PARTYSIZE; i++)
		/* Is the party slot occupied? */
		if (theGame->GetPartyMember(i))
			/* Is the party member incapaciated? */
			if (theGame->GetPartyMember(i)->GetHP() > 0)
			{
				partyMember = theGame->GetPartyMember(i);
				pHp = partyMember->GetHP();
				totalPhp = partyMember->GetTotalHP();
				for (auto i = 0; i < NUM_STATS; i++)
					pStat[i] = partyMember->GetStat(i);
				return true;
			}
	return false;
}

void CombatSys::StartBattle()
{
	opponent = enemies[OPPONENT];
	if (!opponent)
	{
		std::cout << "INVALID BAD GUY!" << std::endl;
		EndBattle();
		return;
	}

	CalcStats(OPPONENT);
	std::cout << "A wild " << opponent->GetName() << " appeared." << std::endl;

	if (!FindPartyMember())
	{
		std::cout << "You have no party so you ran." << std::endl;
		EndBattle();
		return;
	}

	std::cout << "You sent out " << partyMember->GetNickname() << "." << std::endl;

	eHp = totalEhp;
	PrintHealth();
}

void CombatSys::EndBattle()
{
	if (!AUTOHEAL)
		if (partyMember)
			partyMember->SetHP(pHp);
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
		Move* theMove = moveList[0]; // Tackle
		std::string choice = "0";

		std::cout << partyMember->GetNickname() << " knows these moves:" << std::endl;
		for (auto i = 0; i < mCount; i++)
			std::cout << moveList[i]->GetName() << std::endl;
		std::cout << "Attack with which move?" << std::endl;
		std::getline(std::cin, choice);
		toupper(choice);

		if (choice != "TAIL WHIP")
		{
			int pDmg = CalcDamage(partyMember->GetLevel(), theMove->GetPower(), pStat[ATTACK], eStat[DEFENSE]);
			std::cout << "Your " << partyMember->GetNickname() << " attacked with " << theMove->GetName() << ", dealing " << pDmg << " points of damage." << std::endl;
			eHp -= pDmg;
		}
		else
		{
			theMove = moveList[1]; // Tail Whip
			std::cout << "You " << partyMember->GetNickname() << " used " << theMove->GetName() << ", lowering " << opponent->GetName() << "'s defense." << std::endl;
			eStat[DEFENSE]--;
		}

		/* Decide to attack */
		if (rand() % 100 <= 60)
		{
			int eDmg = CalcDamage(LV, moveList[0]->GetPower(), eStat[ATTACK], pStat[DEFENSE]);
			std::cout << "Enemy " << opponent->GetName() << " attacked with " << moveList[0]->GetName() << ", dealing " << eDmg << " points of damage." << std::endl;
			pHp -= eDmg;
		}
		else
		{
			std::cout << "Enemy " << opponent->GetName() << " used " << moveList[1]->GetName() << ", lowering " << partyMember->GetNickname() << "'s defense." << std::endl;
			pStat[DEFENSE]--;
		}
	}

	PrintHealth();
    std::cout << std::endl;

	if (pHp <= 0)
	{
		std::cout << partyMember->GetName() << " can no longer fight!" << std::endl;
		if (!AUTOHEAL)
			partyMember->SetHP(0);

		if (FindPartyMember())
			std::cout << "You sent out " << partyMember->GetName() << "." << std::endl;
		else
		{
			std::cout << "You lost!" << std::endl;
			EndBattle();
			return true;
		}
	}
	if (eHp <= 0)
	{
		std::cout << "You won!" << std::endl;
		EndBattle();
		return true;
	}
	return false;
}

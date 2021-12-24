#include "enemies.h"
#include "battle.h"

/* TODO: Split the combat system from the battle itself. */

/* The player heals at the end of each battle? */
constexpr bool AUTOHEAL = false;

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
	theGame = gameObj;
	mCount = eCount = eLevel = 0;
	totalEhp = totalPhp =
		eHp = pHp = 10;
	eIndex = 19; // Rattata - They are everywhere
	bStarted = trainerBattle = false;
	opponent = partyMember = nullptr;
	/* Fill the arrays with safe data. */
	for (auto h = 0; h < PARTYSIZE; h++)
		participated[h] = false;
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

void CombatSys::CalcStats(int index, int level)
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
		eStat[i] = ((2 * base[i] + eIv[i] + EV / 4) * level) / 100 + 5;
	}

	// HP is a stat with a different formula.
	totalEhp = ((2 * base[HEALTH] + eIv[HEALTH] + EV / 4 + 100) * level) / 100 + 10;
}

int CombatSys::CalcDamage(int level, int power, int attackStat, int defenseStat)
{
	float E = 1; // Targets, .75 when move effects multiple targets
	float W = 1; // Weather
	float C = 1; // Crit, 2 for a crit
	float R = 0.85f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (1 - 0.85f))); // Random
	float S = 1; // Same-Type Attack Bonus, 1.5 when the move is same type as one of the user's types.
	float T = 1; // Type: 0, (.25 or .5), 1, (2 or 4), depending on both the move's and target's types.
	float U = 1; // Burn, .5 when burned user is hit with physical move.
	float O = 1; // Other, for special cases
	return (((2 * level / 5 + 2) * power * attackStat / defenseStat) / 50 + 2)
		* E * W * C * R * S * T * U * O;
}

int CombatSys::CalcExp()
{
	float trainType = 1;
	if (trainerBattle)
		trainType += 0.5;

	float tradeExp = 1;
	if (false) // Was this pokemon traded to you?
		tradeExp += 0.5;

	int share = 0; // EXP. All greatly changes this calculation.
	for (auto i = 0; i < PARTYSIZE; i++)
		if (theGame->GetPartyMember(i))
			if ((participated[i]) &&
				(theGame->GetPartyMember(i)->GetHP() > 0))
				share++;

	int bXp = opponent->GetXpYield();

	return (trainType * tradeExp * bXp * eLevel) / 7 * share;
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
				participated[i] = true;
				partyMember = theGame->GetPartyMember(i);
				pHp = partyMember->GetHP();
				totalPhp = partyMember->GetTotalHP();
				for (auto i = 0; i < NUM_STATS; i++)
					pStat[i] = partyMember->GetStat(i);
				std::cout << "You sent out " << partyMember->GetNickname() << "." << std::endl;
				return true;
			}
	return false;
}

void CombatSys::StartBattle()
{
	eIndex = 19;
	if (rand() % 2 == 0)
		eIndex = 16;

	opponent = enemies[eIndex];
	if (!opponent)
	{
		std::cout << "INVALID BAD GUY!" << std::endl;
		EndBattle();
		return;
	}

	int minLv = 2;
	int maxLv = 5;
	eLevel = rand() % (maxLv - minLv + 1) + minLv;

	CalcStats(eIndex, eLevel);
	std::cout << "A wild " << opponent->GetName() << " appeared!" << std::endl;
	std::cout << opponent->GetName() << " is level " << eLevel << "." << std::endl;
	eHp = totalEhp;

	if (!FindPartyMember())
	{
		std::cout << "You have no party so you ran." << std::endl;
		EndBattle();
		return;
	}

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
	for (auto i = 0; i < PARTYSIZE; i++)
		participated[i] = false;
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
			int eDmg = CalcDamage(eLevel, moveList[0]->GetPower(), eStat[ATTACK], pStat[DEFENSE]);
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

		if (!FindPartyMember())
		{
			std::cout << "You lost!" << std::endl;
			EndBattle();
			return true;
		}
		PrintHealth();
	}
	if (eHp <= 0)
	{
		std::cout << "Enemy " << opponent->GetName() << " fainted!" << std::endl;
		partyMember->AwardEV(opponent->GetBaseHealth(), HEALTH);
		partyMember->AwardEV(opponent->GetBaseAttack(), ATTACK);
		partyMember->AwardEV(opponent->GetBaseDefense(),DEFENSE);
		partyMember->AwardEV(opponent->GetBaseSpAtk(),  SATTACK);
		partyMember->AwardEV(opponent->GetBaseSpDef(),  SDEFENSE);
		partyMember->AwardEV(opponent->GetBaseSpeed(),  SPEED);

		std::cout << "You won!" << std::endl;
		int xp = CalcExp();
		for (auto i = 0; i < PARTYSIZE; i++)
			if (theGame->GetPartyMember(i))
				if ((participated[i]) &&
					(theGame->GetPartyMember(i)->GetHP() > 0) &&
					(theGame->GetPartyMember(i)->GetLevel() < MAX_LEVEL))
				{
					std::cout << theGame->GetPartyMember(i)->GetNickname() << " gained " << xp << " experience!" << std::endl;
					theGame->GetPartyMember(i)->AwardXP(xp);
				}

		if (trainerBattle)
		{
			int mReward = eLevel * 10;
			std::cout << "You should get " << Money(mReward) << " for winning." << std::endl;
		}
		EndBattle();
		return true;
	}
	return false;
}

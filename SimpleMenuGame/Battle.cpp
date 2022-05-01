#include <algorithm>
#include "enemies.h"
#include "moves.h"
#include "battle.h"

/* TODO: Split the combat system from the battle itself. */

/* The player heals at the end of each battle? */
constexpr bool AUTOHEAL = false;

constexpr auto BSTAT = 0;

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
	return ColoredString(std::to_string(cur) + "/" + std::to_string(max), color);
}

CombatSys::CombatSys(Game* gameObj)
{
	theGame = gameObj;
	mCount = eCount = eLevel = 0;
	totalEhp = totalPhp =
		eHp = pHp = 10;
	eIndex = 19; // Rattata - They are everywhere
	bStarted = trainerBattle = enemyTurn = false;
	opponent = partyMember = nullptr;
	eAcc = eEvas = pAcc = pEvas = BSTAT;

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

int CombatSys::CalcDamage(Move* mov)
{
	int power = mov->GetPower();

	int level = partyMember->GetLevel();
	int attackStat = pStat[ATTACK];
	int defenseStat = eStat[DEFENSE];
	int t1 = partyMember->GetType();
	int t2 = partyMember->GetType(false);
	if (enemyTurn)
	{
		level = eLevel;
		attackStat = eStat[ATTACK];
		defenseStat = pStat[DEFENSE];
		t1 = opponent->GetType();
		t2 = opponent->GetType(false);
	}
	
	float R = 0.85f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (1 - 0.85f))); // Random
	
	float S = 1; // Same-Type Attack Bonus, 1.5 when the move is same type as one of the user's types.
	int mType = mov->GetType();
	if (mType == t1)
		S += 0.5f;
	else
		if (mType == t2)
			if (t2 != (int)Types::NORMAL) // T2 = Normal when there's no second type.
				S += 0.5f;

	const float T = 1; // Type: 0, (.25 or .5), 1, (2 or 4), depending on both the move's and target's types.
	const float C = 1; // Crit, 2 for a crit
	const float U = 1; // Burn, .5 if burned attacker using physical move
	const float E = 1; // Targets, .75 when move effects multiple targets
	const float W = 1; // Weather
	const float O = 1; // Other, for special cases
	return int((((2 * level / 5 + 2) * power * attackStat / defenseStat) / 50 + 2)
		* R * S * T * C * U * E * W * O);
}

bool CombatSys::HitChance(int mAcc)
{
	int acc = mAcc; // Move Accuracy
	int ot = 1; // Other effects, serially applied.

	/*
		StageMultiplier is the accuracy stage multiplier
		after the evasion stage is subtracted from the 
		accuracy stage, both possibly modified by
		Ability or move effects (to > -6 and < 6)
	// */
	int aas = pAcc; // Attacker's Accuracy Stage
	int des = eEvas; // Defender's Evasion Stage
	if (enemyTurn)
	{
		aas = eAcc;
		des = pEvas;
	}
	int cs = std::clamp(aas - des, -6, 6); // Combined Stage
	float sv[13] = { 33, 36, 43, 50, 60, 75, 100, 133, 166, 200, 250, 266, 300 };
	float sm = sv[cs + 6] / 100;

	float ch = acc * sm * ot; // Calculation to test against RNG
	return (rand() % 100) <= ch;
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

	return int((trainType * tradeExp * bXp * eLevel) / 7 * share);
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
				pAcc = pEvas = BSTAT;
				for (auto i = 0; i < NUM_STATS; i++)
					pStat[i] = partyMember->GetStat(i);
				std::cout << "You sent out " << partyMember->GetNickname() << "." << std::endl;
				return true;
			}
	return false;
}

void CombatSys::StartBattle()
{
	encounterData* encZone = theGame->ReadEncounterZone(theGame->GetLastRoom());
	if (!encZone)
	{
		std::cout << "There's no one to fight here." << std::endl;
		EndBattle();
		return;
	}

	trainerBattle = encZone->trainer;

	auto eChoice = 0; // The baddie that we spawn
	auto slots = 0; // How many slots did we try?
	for (auto i = 0; i < MAX_WILD; i++)
	{
		if (encZone->enemies[i])
		{
			eIndex = 0;
			int chnce = *encZone->chance;
			if (chnce)
			{
				if ((rand() % 100) <= chnce)
					eIndex = encZone->enemies[i];
			}
			else
				eIndex = encZone->enemies[i];
			slots++;
		}
		// Stop rolling if one won.
		if (eIndex)
		{
			eChoice = i;
			break;
		}
	}
	// Dice never rolled hign enough, so choose a random *used* slot.
	if (!eIndex)
	{
		eChoice = rand() % slots;
		eIndex = encZone->enemies[eChoice];
	}
	opponent = enemies[eIndex];

	int minLv = encZone->minLv[eChoice];
	int maxLv = encZone->maxLv[eChoice];

	eLevel = maxLv;
	if (encZone->randType[eChoice] == 1)
		eLevel = rand() % (maxLv - minLv + 1) + minLv;
	if (encZone->randType[eChoice] == 2)
	{
		eLevel = minLv;
		if ((rand() % 2) == 0)
			eLevel = maxLv;
	}
	opponent->BuildMoveList(eLevel);

	CalcStats(eIndex, eLevel);
	if (trainerBattle)
		std::cout << "Your challenger sent out " << opponent->GetName() << "." << std::endl;
	else
		std::cout << "A wild " << opponent->GetName() << " appeared!" << std::endl;
	std::cout << opponent->GetName() << " is level " << eLevel << "." << std::endl;
	eHp = totalEhp;

	if (!FindPartyMember())
	{
		std::cout << "You have no party so you ran." << std::endl;
		EndBattle();
		return;
	}
	else
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
	if ((theGame->GetChoice() == "ATTACK") || (theGame->GetChoice() == "BAG"))
	{
		if (theGame->GetChoice() == "BAG")
		{
			std::string choice = "";
			std::cout << "Use Which item?" << std::endl;
			std::getline(std::cin, choice);
			toupper(choice);
			if (choice != "POKEBALL")
				return false;
			if (theGame->RemoveInventoryItem("Pokeball"))
			{
				if (trainerBattle)
				{
					std::cout << "You can't use that right now!" << std::endl;
					return false;
				}
				std::cout << "You threw a pokeball." << std::endl;
				std::cout << "..." << std::endl;
				theGame->Pause();
				if (rand() % 100 > 95) // TODO: Catch Rate?
				{
					std::cout << "It broke free!" << std::endl;
					return false;
				}
				std::cout << "You caught " << opponent->GetName() << "!" << std::endl;
				theGame->AddPartyMember(eIndex, eLevel, eHp); // BUGBUG: This will re-roll the IVs.
				EndBattle();
				return true;
			}
			else
			{
				std::cout << "You don't have any!" << std::endl;
				return false;
			}
		}
		if (theGame->GetChoice() == "ATTACK")
		{
			std::string choice = "";

			std::cout << partyMember->GetNickname() << " knows these moves:" << std::endl;
			for (auto i = 0; i < MOVE_MEM; i++)
				if (partyMember->MoveName(i) != "")
					std::cout << partyMember->MoveName(i) << std::endl;

			std::cout << "Attack with which move?" << std::endl;
			std::getline(std::cin, choice);
			if (choice == "")
				return false;
			toupper(choice);

			bool chosen = false;
			for (auto i = 0; i < MOVE_MEM; i++)
			{
				std::string pMove = partyMember->MoveName(i);
				toupper(pMove);
				if (pMove == choice)
				{
					chosen = true;
					MoveAction(partyMember->MoveName(i));
				}
			}
			if (!chosen)
				return false;
		}

		enemyTurn = true;
		/* Pick a random enemy move. */
		// TODO: Favor attacking moves?
		std::string eMoves[MOVE_MEM] = { "" };
		auto i = 0;
		for (; i < MOVE_MEM; i++)
			if (opponent->MoveName(i) != "")
				eMoves[i] = opponent->MoveName(i);
		MoveAction(eMoves[rand() % i]);
		enemyTurn = false;
	}
	if ((theGame->GetChoice() == "RUN") || (theGame->GetChoice() == "QUIT"))
	{
		if (trainerBattle)
		{
			std::cout << "You can't run from a trainer!" << std::endl;
			return false;
		}
		std::cout << "You ran." << std::endl;
		EndBattle();
		return true;
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
			theGame->SetRoom("GameOver");
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
				if ((participated[i]) && (theGame->GetPartyMember(i)->GetHP() > 0))
				{
					if (theGame->GetPartyMember(i)->GetLevel() < MAX_LEVEL)
						std::cout << theGame->GetPartyMember(i)->GetNickname() << " gained " << xp << " experience!" << std::endl;
					// This allows new EVs to be calculated into stats.
					theGame->GetPartyMember(i)->AwardXP(xp);
				}

		if (trainerBattle)
		{
			int reward = eLevel * 10;
			theGame->AddMoney(reward);
			std::cout << "You got " << Money(reward) << " for winning." << std::endl;
		}
		EndBattle();
		return true;
	}
	return false;
}

Move* CombatSys::MovesByName(std::string mv)
{
	for (auto i = 0; i < mCount; i++)
		if (moveList[i]->GetName() == mv)
			return moveList[i];
	return moveList[33];
}

void CombatSys::MoveAction(std::string mov)
{
	Move* move = MovesByName(mov);
	std::string summ = "";
	std::string pnam = opponent->GetName();
	std::string oppo = partyMember->GetNickname();

	if (enemyTurn)
		summ += "Enemy";
	else
	{
		oppo = opponent->GetName();
		pnam = partyMember->GetNickname();
		summ += "Your";
	}
	summ += " " + pnam + " used " + move->GetName() + ", ";

	if (move->GetEffect() == NO_EFFECT)
		summ += "and nothing happened";

	if (move->GetEffect() == DEV_EFFECT)
	{
		for (auto i = 0; i < 34; i++)
			summ += "developers, ";
		summ += "developers";
	}

	if (move->GetEffect() == ATTACK_EFFECT)
	{
		if (HitChance(move->GetAccuracy()))
		{
			int dmg = CalcDamage(move);
			if (enemyTurn)
				pHp -= dmg;
			else
				eHp -= dmg;
			summ += "dealing " + std::to_string(dmg) + " points of damage";
		}
		else
			summ += "but it missed";
	}
	else
	{
		std::string sstat = "Stats";

		if (move->GetEffect() == BLIND)
		{
			sstat = "Accuracy";
			if (enemyTurn)
				pAcc = std::clamp(pAcc-1, -6, 6);
			else
				eAcc = std::clamp(eAcc-1, -6, 6);
		}
		else
		{
			// LESS_DEFENSE
			int aStat = DEFENSE;
			if (move->GetEffect() == LESS_ATTACK)
				aStat = ATTACK;
			if (move->GetEffect() == SLOW)
				aStat = SPEED;

			if (enemyTurn)
				pStat[aStat]--;
			else
				eStat[aStat]--;
			sstat = StatName(aStat);
		}
		tolower(sstat);
		summ += "lowering " + oppo + "'s " + sstat;
	}
	std::cout << summ << "." << std::endl;
}

std::string GetTypeName(Types typ)
{
	std::string myName = "Normal"; // This covers the "Default" case.
	switch (typ)
	{
	case Types::BUG:
		myName = "Bug";
		break;
	case Types::DARK:
		myName = "Dark";
		break;
	case Types::DRAGON:
		myName = "Dragon";
		break;
	case Types::ELECTRIC:
		myName = "Electric";
		break;
	case Types::FAIRY:
		myName = "Fairy";
		break;
	case Types::FIGHTING:
		myName = "Fighting";
		break;
	case Types::FIRE:
		myName = "Fire";
		break;
	case Types::FLYING:
		myName = "Flying";
		break;
	case Types::GHOST:
		myName = "Ghost";
		break;
	case Types::GRASS:
		myName = "Grass";
		break;
	case Types::GROUND:
		myName = "Ground";
		break;
	case Types::ICE:
		myName = "Ice";
		break;
	case Types::POISON:
		myName = "Poison";
		break;
	case Types::PSYCHIC:
		myName = "Psychic";
		break;
	case Types::ROCK:
		myName = "Rock";
		break;
	case Types::STEEL:
		myName = "Steel";
		break;
	case Types::WATER:
		myName = "Water";
	}
	return myName;
}

Types TypeFromNum(int num)
{
	Types retType = Types::NORMAL; // This covers the "Default" case.

	switch (num)
	{
	case 0:
		retType = Types::BUG;
		break;
	case 1:
		retType = Types::DARK;
		break;
	case 2:
		retType = Types::DRAGON;
		break;
	case 3:
		retType = Types::ELECTRIC;
		break;
	case 4:
		retType = Types::FAIRY;
		break;
	case 5:
		retType = Types::FIGHTING;
		break;
	case 6:
		retType = Types::FIRE;
		break;
	case 7:
		retType = Types::FLYING;
		break;
	case 8:
		retType = Types::GHOST;
		break;
	case 9:
		retType = Types::GRASS;
		break;
	case 10:
		retType = Types::GROUND;
		break;
	case 11:
		retType = Types::ICE;
		break;
	case 13:
		retType = Types::POISON;
		break;
	case 14:
		retType = Types::PSYCHIC;
		break;
	case 15:
		retType = Types::ROCK;
		break;
	case 16:
		retType = Types::STEEL;
		break;
	case 17:
		retType = Types::WATER;
	};

	return retType;
}

Types TypeFromName(std::string nme)
{
	const unsigned long long NUM_T = unsigned long long(Types::NUM_TYPES);

	nme = LoadString(nme, "invalid");
	
	std::string tNames[NUM_T] =
	{
		"Bug",
		"Dark",
		"Dragon",
		"Electric",
		"Fairy",
		"Fighting",
		"Fire",
		"Flying",
		"Ghost",
		"Grass",
		"Ground",
		"Ice",
		"Normal",
		"Poison",
		"Psychic",
		"Rock",
		"Steel",
		"Water"
	};

	/* Convert test to title case. */
	transform(nme.begin(), nme.begin() + 1, nme.begin(), std::toupper);

	for (auto i = 0; i < NUM_T; i++)
		if (nme == tNames[i])
			return TypeFromNum(i);
	return Types::NORMAL;
}

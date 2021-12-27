#include <algorithm>
#include "enemies.h"
#include "moves.h"
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
	_ASSERT(gameObj);
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

	int minLv = 2;
	int maxLv = 5;
	eLevel = rand() % (maxLv - minLv + 1) + minLv;
	opponent->BuildMoveList(eLevel);

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
				std::cout << "You threw a pokeball." << std::endl;
				std::cout << "You caught " << opponent->GetName() << "!" << std::endl;
				theGame->AddPartyMember(eIndex, eLevel, eHp);
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
			toupper(choice);

			bool chosen = false;
			for (auto i = 0; i < MOVE_MEM; i++)
			{
				std::string pMove = partyMember->MoveName(i);
				toupper(pMove);
				if (pMove == choice)
				{
					chosen = true;
					MoveAction(partyMember->MoveName(i), true);
				}
			}
			if (!chosen)
				return false;
		}
		/* Pick a random enemy move. */
		// TODO: Favor attacking moves?
		std::string eMoves[MOVE_MEM] = { "" };
		auto i = 0;
		for (; i < MOVE_MEM; i++)
			if (opponent->MoveName(i) != "")
				eMoves[i] = opponent->MoveName(i);
		MoveAction(eMoves[rand() % i]);
	}
	if ((theGame->GetChoice() == "RUN") || (theGame->GetChoice() == "QUIT"))
	{
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

Move* CombatSys::MovesByName(std::string mv)
{
	for (auto i = 0; i < mCount; i++)
		if (moveList[i]->GetName() == mv)
			return moveList[i];
	return moveList[33];
}

void CombatSys::MoveAction(std::string mov, bool plr)
{
	Move* move = MovesByName(mov);
	std::string summ = "";
	std::string pnam = opponent->GetName();
	std::string oppo = partyMember->GetNickname();

	if (plr)
	{
		oppo = opponent->GetName();
		pnam = partyMember->GetNickname();
		summ += "Your";
	}
	else
		summ += "Enemy";
	summ += " " + pnam + " used " + move->GetName();
	if (move->GetPower())
	{
		int level = eLevel;
		int attack = eStat[ATTACK];
		int def = pStat[DEFENSE];
		if (plr)
		{
			level = partyMember->GetLevel();
			attack = pStat[ATTACK];
			def = eStat[DEFENSE];
		}
		int dmg = CalcDamage(level, move->GetPower(), attack, def);
		if (plr)
			eHp -= dmg;
		else
			pHp -= dmg;
		summ += ", dealing " + std::to_string(dmg) + " points of damage.";
	}
	else
	{
		if (plr)
			eStat[DEFENSE]--;
		else
			pStat[DEFENSE]--;
		summ += ", lowering " + oppo + "'s defense.";
	}
	std::cout << summ << std::endl;
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
	nme = LoadString(nme, "invalid");
	
	std::string tNames[18] =
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

	for (auto i = 0; i < 18; i++)
		if (nme == tNames[i])
			return TypeFromNum(i);
	return Types::NORMAL;
}

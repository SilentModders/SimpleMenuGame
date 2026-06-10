#include "enemies.h"
#include "moves.h"
#include "battle.h"

/* The player heals at the end of each battle? */
constexpr bool AUTOHEAL = false;

/* Temp stats (Accuracy, Evasion) begin at 0. */
constexpr auto BSTAT = 0;

/* Critical damage is multiplied by this. */
constexpr float CRIT_BONUS = 2;

/* Chance to apply a debuff (like poison) in moves like Poison Sting */
constexpr auto DB_CHANCE = 30;

/* Debuffs last up to this many turns. */
constexpr auto DB_TURNS = 7;

/* Damage from poison, burn... */
int poisonDamage(int maxHP)
{
	int dmg = maxHP / 16;
	if (dmg > 1)
	{
		return dmg;
	}
	return 1;
}

std::string ColoredHp(int cur, int max)
{
	Color color = Color::COLOR_RED;
	float frac = 0.0f;
	if (max)
	{
		frac = float(cur) / (float)max;
	}
	if (frac > 0.33f)
	{
		color = Color::COLOR_YELLOW;
	}
	if (frac > 0.67f)
	{
		color = Color::COLOR_GREEN;
	}
	return ColoredString(std::to_string(cur) + "/" + std::to_string(max), color);
}

CombatSys::CombatSys(Game* gameObj)
{
	theGame = gameObj;
	mCount =
		sCoins =
		eMember =
		eCount =
		eMvCount =
		pMvCount =
		eLevel =
		ePartySz = 0;
	totalEhp = totalPhp =
		eHp = pHp = 10;
	eIndex = 19; // Rattata - They are everywhere
	bStarted =
		trainerBattle =
		reportCrit =
		enemyTurn =
		recentReset = false;
	partyMember = nullptr;
	encZone = nullptr;
	eAcc = eEvas = eCritC =
		pAcc = pEvas = pCritC = BSTAT;

	/* Fill the arrays with safe data. */
	for (auto r = 0; r < PARTYSIZE; r++)
	{
		participated[r] = false;
		savedHP[r] = 0;
		eParty[r] = nullptr;
	}
	for (auto n = 0; n < MAX_ENEMIES; n++)
	{
		enemies[n] = new Enemy();
	}
	for (auto s = 0; s < NUM_STATS; s++)
	{
		eStat[s] = pStat[s] = 0;
	}
	for (auto v = 0; v < MAX_MOVES; v++)
	{
		moveList[v] = new Move;
	}
	for (auto m = 0; m < MAX_COMBATANTS; m++)
	{
		bPoisonNum[m] =
		rageCount[m] =
		pSubstituteHP[m] = 0;
		midMove[m] =
		hidden[m] =
		statGuarded[m] =
		substituted[m] = false;
		for (auto n = 0; n < NUM_COUNTDOWNS; n++)
		{
			debuffTurns[m][n] = 0;
		}
		lastAttack[m] = "";
	}
	for (auto p = 0; p < MOVE_MEM; p++)
	{
		eMoves[p] = pMoves[p] = "";
	}
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
	if(recentReset)
	{
		std::cout << eParty[eMember]->GetName() << " is level " << eLevel << "." << std::endl;
		recentReset = false;
	}
	std::cout << "Your " << partyMember->GetNickname() << " has " << ColoredHp(pHp, totalPhp) << " hit points." << std::endl;
	std::cout << "Enemy " << eParty[eMember]->GetName() << " has " << ColoredHp(eHp, totalEhp) << " hit points." << std::endl;
}

void CombatSys::CalcStats(int index, int level)
{
	// Store Stats
	for (auto s = 0; s < NUM_STATS; s++)
	{
		eStat[s] = eParty[eMember]->GetStat(s);
	}
	totalEhp = eParty[eMember]->GetTotalHP();
}

int CombatSys::CalcDamage(Move* mov, bool noCrits)
{
	int power = mov->GetPower();

	int level = partyMember->GetLevel();

	int attackStat = pStat[ATTACK_STAT];
	int defenseStat = eStat[DEFENSE];

	int critSt = pCritC; // Crit Chance stage
	Types t1 = partyMember->GetType();
	Types t2 = partyMember->GetType(false);

	Types mType = mov->GetType();
	if (SpecialType(mType))
	{
		attackStat = pStat[SATTACK];
		defenseStat = eStat[SDEFENSE];
	}

	if (enemyTurn)
	{
		level = eLevel;
		attackStat = eStat[ATTACK_STAT];
		defenseStat = pStat[DEFENSE];
		if (SpecialType(mType))
		{
			attackStat = eStat[SATTACK];
			defenseStat = pStat[SDEFENSE];
		}
		critSt = eCritC;
		t1 = eParty[eMember]->GetType();
		t2 = eParty[eMember]->GetType(false);
	}

	/* Rage makes you stronger as you get hit. */
	attackStat += rageCount[enemyTurn];

	/* Explosion and self-destruct read half the defense for some reason. */
	if (mov->GetEffect() == FAINT)
	{
		defenseStat /= 2;
	}

	/* Reflect */
	if ((debuffTurns[enemyTurn][REFLECTING]) && (!SpecialType(mType)))
	{
		defenseStat *= 2;
	}
	/* Light Screen */
	if ((debuffTurns[enemyTurn][SCREENED]) && (SpecialType(mType)))
	{
		defenseStat *= 2;
	}

	float rnd = 0.85f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (1 - 0.85f))); // Random

	float stab = 1; // Same-Type Attack Bonus, 1.5 when the move is same type as one of the user's types.
	if (mType == t1)
	{
		stab += 0.5f;
	}
	else
	{
		if (mType == t2)
		{
			if (t2 != Types::NORMAL) // T2 = Normal when there's no second type.
			{
				stab += 0.5f;
			}
		}
	}

	float typ = NORM_D; // Type: 0, (.25 or .5), 1, (2 or 4), depending on both the move's and target's types.
	typ *= TypeEffectiveness(t1, t2, mType);
	// BUGBUG: Some moves, like Struggle, are excluded from type calculation.

	float crit = 1; // Critical hit
	if (!noCrits) // No crit calculation when ignored.
	{
		reportCrit = false; // In case it was true last time.
		int crChance = 16;
		// Each crit chance stage makes crits twice as likely.
		if (mov->GetEffect() == HI_CRIT)
		{
			critSt++;
		}
		for (int c = 0; c < critSt; c++)
		{
			crChance /= 2;
		}
		if (random_int(0, crChance) == crChance)
		{
			crit *= CRIT_BONUS;
			reportCrit = true;
		}
	}

	float burned = 1; // Burn, .5 if burned attacker using physical move
	if ((enemyTurn && eParty[eMember]->Burned()) || (!enemyTurn && partyMember->Burned()))
	{
		if (!SpecialType(mType))
		{
			burned = .5;
		}
	}

	const float E = 1; // Targets, .75 when move effects multiple targets
	const float O = 1; // Other, for special cases
	return int((((2 * level / 5 + 2) * power * attackStat / defenseStat) / 50 + 2)
		* rnd * stab * typ * crit * burned * E * O);
}

bool CombatSys::HitChance(int mAcc)
{
	/*
		Combatants become "hidden"
		during moves such as Dig and Fly.
		They cannot be hit.
	// */
	if (hidden[!enemyTurn])
	{
		return false;
	}

	const int acc = mAcc; // Move Accuracy
	const int ot = 1; // Other effects

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
	const int cs = std::clamp(aas - des, -6, 6); // Combined Stage
	const float sv[13] = { 33, 36, 43, 50, 60, 75, 100, 133, 166, 200, 250, 266, 300 };
	const float sm = sv[cs + 6] / 100;

	const float ch = acc * sm * ot; // Calculation to test against RNG
	return random_int(1, 100) <= ch;
}

int CombatSys::CalcExp()
{
	float trainType = 1;
	if (trainerBattle)
	{
		trainType += 0.5;
	}

	float tradeExp = 1;
	if (false) // Was this Pokémon traded to you?
	{
		tradeExp += 0.5;
	}

	int share = 0; // EXP. All greatly changes this calculation.
	for (auto m = 0; m < PARTYSIZE; m++)
	{
		if (theGame->GetPartyMember(m))
		{
			if ((participated[m]) &&
				(theGame->GetPartyMember(m)->GetHP() > 0))
			{
				share++;
			}
		}
	}

	int bXp = eParty[eMember]->GetXpYield();

	return int((trainType * tradeExp * bXp * eLevel) / 7 * share);
}

bool CombatSys::FindPartyMember()
{
	/* Search for a party member that isn't down. */
	for (auto m = 0; m < PARTYSIZE; m++)
	{
		/* Is the party slot occupied? */
		if (theGame->GetPartyMember(m))
		{
			if (SwitchPartyMember(m))
			{
				return true;
			}
		}
	}
	return false;
}

/* BUGBUG: The benched monster's accuracy, evasion, and poisoning will reset. */
bool CombatSys::SwitchPartyMember(int mem, bool stdMsg)
{
	mem = std::clamp(mem, 0, PARTYSIZE - 1);

	/* Is the party slot occupied? */
	if (theGame->GetPartyMember(mem))
	{
		/* Is the party member incapacitated? */
		if (theGame->GetPartyMember(mem)->GetHP() > 0)
		{
			participated[mem] = true;
			partyMember = theGame->GetPartyMember(mem);
			pHp = partyMember->GetHP();
			totalPhp = partyMember->GetTotalHP();
			pAcc = pEvas = pCritC = BSTAT;
			for (auto s = 0; s < NUM_STATS; s++)
			{
				pStat[s] = partyMember->GetStat(s);
			}
			for (pMvCount = 0; pMvCount < MOVE_MEM; pMvCount++)
			{
				if (partyMember->MoveName(pMvCount) != "")
				{
					pMoves[pMvCount] = partyMember->MoveName(pMvCount);
				}
				else
				{
					break;
				}
			}
			/* Reset Battle Conditions */
			bPoisonNum[0] =
				midMove[0] =
				hidden[0] =
				rageCount[0] =
				statGuarded[0] =
				substituted[0] = false;
			for (auto c = 0; c < NUM_COUNTDOWNS; c++)
			{
				debuffTurns[0][c] = 0;
			}
			if (stdMsg)
			{
				std::cout << "You sent out " << partyMember->GetNickname() << "!" << std::endl;
			}
			return true;
		}
	}
	return false;
}

bool CombatSys::Transform()
{
	Enemy* user = partyMember;
	Enemy* target = eParty[eMember];
	std::string* theirM = eMoves;
	std::string* myM = pMoves;
	int atk = eStat[ATTACK_STAT];
	int def = eStat[DEFENSE];
	int sat = eStat[SATTACK];
	int sdf = eStat[SDEFENSE];
	int spd = eStat[SPEED];
	if (enemyTurn)
	{
		atk = pStat[ATTACK_STAT];
		def = pStat[DEFENSE];
		sat = pStat[SATTACK];
		sdf = pStat[SDEFENSE];
		spd = pStat[SPEED];
		user = eParty[eMember];
		target = partyMember;
		theirM = pMoves;
		myM = eMoves;
	}

	/* Species and Main Stats */
	user->Setup(
		user->GetName(),
		target->GetIdNum(),
		user->GetBaseHealth(),
		atk,
		def,
		sat,
		sdf,
		spd,
		user->GetXpCurve(),
		user->GetXpYield(),
		user->GetCatchRate(),
		user->GetEvoLevel(),
		target->GetType(),
		target->GetType(false));

	/* Battle Stats */
	if (enemyTurn)
	{
		eStat[ATTACK_STAT] = atk;
		eStat[DEFENSE] = def;
		eStat[SATTACK] = sat;
		eStat[SDEFENSE]= sdf;
		eStat[SPEED]   = spd;
	}
	else
	{
		pStat[ATTACK_STAT] = atk;
		pStat[DEFENSE] = def;
		pStat[SATTACK] = sat;
		pStat[SDEFENSE]= sdf;
		pStat[SPEED]   = spd;
	}

	/* Stat stages */
	if (enemyTurn)
	{
		eAcc  = pAcc;
		eEvas = pEvas;
		eCritC= pCritC;
	}
	else
	{
		pAcc  = eAcc;
		pEvas = eEvas;
		pCritC= eCritC;
	}

	/* Moves */
	if (enemyTurn)
	{
		eMvCount = pMvCount;
	}
	else
	{
		pMvCount = eMvCount;
	}
	for (auto v = 0; v < MOVE_MEM; v++)
	{
		myM[v] = theirM[v];
	}
	return true;
}

bool CombatSys::EnemySwitch(int mem)
{
	if (mem < 0)
	{
		return false;
	}
	if (mem >= ePartySz)
	{
		return false;
	}

	/* Is the party slot occupied? */
	if (encZone->enemies[mem])
	{
		/* Is the party member incapacitated? */
		if (savedHP[mem] > 0)
		{
			return true;
		}
	}
	return false;
}

int CombatSys::SearchEnemies()
{
	int newnum = -1;

	if (ePartySz > 1)
	{
		// Try backwards, if we're not the first one.
		if (eMember > 0)
		{
			for (auto p = eMember - 1; p >= 0; p--)
			{
				if (EnemySwitch(p))
				{
					newnum = p;
				}
			}
		}
		// First scan forward, if we're not already last.
		if (eMember < ePartySz - 1)
		{
			for (auto n = eMember + 1; n < ePartySz; n++)
			{
				if (EnemySwitch(n))
				{
					newnum = n;
				}
			}
		}
	}
	return newnum;
}

/* BUGBUG: The benched monster's accuracy, evasion, and poisoning will reset. */
void CombatSys::ResetEnemy()
{
	eIndex = encZone->enemies[eMember];
	for (eMvCount = 0; eMvCount < MOVE_MEM; eMvCount++)
	{
		/* Clear old moves */
		eMoves[eMvCount] = "";

		if (eParty[eMember]->MoveName(eMvCount) != "")
		{
			eMoves[eMvCount] = eParty[eMember]->MoveName(eMvCount);
		}
		else
		{
			break;
		}
	}
	bPoisonNum[1] =
		midMove[1] =
		hidden[1] =
		rageCount[1] =
		statGuarded[1] =
		substituted[1] = false;
	for (auto c = 0; c < NUM_COUNTDOWNS; c++)
	{
		debuffTurns[1][c] = 0;
	}
	CalcStats(eIndex, eLevel);
	eAcc = eEvas = eCritC = BSTAT;
	if (savedHP[eMember] > 0)
	{
		eHp = savedHP[eMember];
	}
	else
	{
		eHp = totalEhp;
	}
	recentReset = true;
}

int CombatSys::GetLevel(int eChoice)
{
	int minLv = encZone->minLv[eChoice];
	int maxLv = encZone->maxLv[eChoice];

	eLevel = maxLv;
	// Random level type 1: Range
	if (encZone->randType[eChoice] == 1)
	{
		eLevel = random_int(minLv, maxLv);
	}
	// Random level type 2: Coinflip
	if (encZone->randType[eChoice] == 2)
	{
		if (random_int(0, 1) == 0)
		{
			eLevel = minLv;
		}
	}
	return eLevel;
}

void CombatSys::StartBattle(bool sameEnc)
{
	encZone = theGame->ReadEncounterZone(theGame->GetLastRoom());
	if (!encZone)
	{
		std::cout << "There's no one to fight here." << std::endl;
		EndBattle();
		return;
	}
	trainerBattle = encZone->trainer;

	if (trainerBattle && !sameEnc)
	{
		/* The other trainer's party */

		// Store all members' HP
		for (auto p = 0; p < PARTYSIZE; p++)
		{
			if (encZone->enemies[p])
			{
				GetLevel(p);
				eParty[p] = new PartyMember(theGame);
				eParty[p]->Create(encZone->enemies[p], eLevel);
				savedHP[p] = eParty[p]->GetTotalHP();
				ePartySz++;
			}
		}
	}

	auto eChoice = 0; // The baddie that we spawn
	auto slots = 0; // How many slots did we try?
	for (auto s = eMember; s < MAX_WILD; s++)
	{
		if (encZone->enemies[s])
		{
			eIndex = 0;
			int chnce = *encZone->chance;
			if (chnce)
			{
				if (random_int(1, 100) <= chnce)
				{
					eIndex = encZone->enemies[s];
				}
			}
			else
			{
				eIndex = encZone->enemies[s];
			}
			slots++;
		}
		// Stop rolling if one won.
		if (eIndex)
		{
			eChoice = s;
			break;
		}
	}
	// Dice never rolled high enough, so choose a random *used* slot.
	if (!eIndex)
	{
		eChoice = random_int(0, slots);
		eIndex = encZone->enemies[eChoice];
	}
	// Sets eLevel internally
	GetLevel(eChoice);

	if (!trainerBattle)
	{
		eParty[0] = new PartyMember(theGame);
		eParty[0]->Create(eIndex, eLevel);
	}
	CalcStats(eIndex, eLevel);

	/* Save up to 4 enemy moves. */
	for (eMvCount = 0; eMvCount < MOVE_MEM; eMvCount++)
	{
		if (eParty[eMember]->MoveName(eMvCount) != "")
		{
			eMoves[eMvCount] = eParty[eMember]->MoveName(eMvCount);
		}
		else
		{
			break;
		}
	}

	/* Clear any flags from an old battle. */
	for (auto m = 0; m < MAX_COMBATANTS; m++)
	{
		if ((sameEnc) && (m == 0))
		{
			/*
				Preserve player debuffs when
				it's the enemy switching.
			// */
			m = 1;
		}
		bPoisonNum[m] = 0;
		midMove[m] = false;
		for (auto n = 0; n < NUM_COUNTDOWNS; n++)
		{
			debuffTurns[m][n] = 0;
		}
	}

	if (trainerBattle)
	{
		std::cout << "Your challenger sent out " << eParty[eMember]->GetName();
	}
	else
	{
		std::cout << "A wild " << eParty[eMember]->GetName() << " appeared";
	}
	std::cout << "!" << std::endl;
	std::cout << eParty[eMember]->GetName() << " is level " << eLevel << "." << std::endl;

	// Check for saved eHP
	if (savedHP[eMember] > 0)
	{
		eHp = savedHP[eMember];
	}
	else
	{
		eHp = totalEhp;
	}

	// TODO: Give you a chance to switch.
	if (!sameEnc)
	{
		if (!FindPartyMember())
		{
			/* This shouldn't happen. */
			std::cout << "You have no party so you ran!" << std::endl;
			EndBattle();
			return;
		}
	}
	PrintHealth();
}

void CombatSys::EndBattle()
{
	if (partyMember)
	{
		/*
			After the battle, if we're not autohealing,
			save the HP, if we are, clear the debuffs.
		// */
		if (!AUTOHEAL)
		{
			partyMember->SetHP(pHp);
		}
		else
		{
			partyMember->ClearDebuffs();
		}
	}
	theGame->SetRoom(theGame->GetLastRoom());
	std::cout << std::endl;
	bStarted = false;
	for (auto r = 0; r < PARTYSIZE; r++)
	{
		participated[r] = false;
	}
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

	bool combatTurn = false; /* A valid choice such as ATTACK or BAG was used. */
	bool pAttack = false; /* Did the player attack this turn? */
	int pChoice = 0; /* Which move did the player pick? */

	/* The enemy only gets a turn when you enter a valid command like ATTACK or BAG */
	if ((theGame->GetChoice() == "ATTACK") || (theGame->GetChoice() == "BAG") || (theGame->GetChoice() == "SWITCH"))
	{
		if (theGame->GetChoice() == "SWITCH")
		{
			std::string choice = "";
			std::cout << "Your party contains the following members:" << std::endl;
			for (auto p = 0; p < PARTYSIZE; p++)
			{
				if (theGame->GetPartyMember(p))
				{
					/*
						Print a summary of each party member.
						E.g. 1) Squirtle, Lv. 5, 20/20 HP
					// */
					std::cout << " " << p + 1 << ") " << theGame->GetPartyMember(p)->GetNickname() <<
						", Lv. " << theGame->GetPartyMember(p)->GetLevel() << ", " <<
						ColoredHp(theGame->GetPartyMember(p)->GetHP(), theGame->GetPartyMember(p)->GetTotalHP()) << " HP" << std::endl;
				}
			}
			std::cout << "Select a new pokémon by number." << std::endl;
			std::getline(std::cin, choice);
			std::cout << std::endl;

			/* Convert choice to int. */
			int sChoice = atoi(choice.c_str());
			/* Words convert to a 0, and the only 1-6 are valid.*/

			if ((sChoice <= 0) || (sChoice > 6))
			{
				return false;
			}

			/* Their current party member is also an invalid choice. */
			if (theGame->GetPartyMember(sChoice - 1) == partyMember)
			{
				return false;
			}

			/* If the selected party member cannot fight, act like the choice was invalid. */
			if (!SwitchPartyMember(sChoice - 1))
			{
				return false;
			}
		}
		combatTurn = true;
		if ((theGame->GetChoice() == "BAG") && (!midMove[0]))
		{
			std::string choice = "";
			std::cout << "Use Which item?" << std::endl;
			// TODO: LIST
			std::getline(std::cin, choice);
			toupper(choice);
			// TODO: POTION
			if (choice != "POKEBALL")
			{
				return false;
			}
			if (theGame->RemoveInventoryItem("Pokeball"))
			{
				if (trainerBattle)
				{
					std::cout << "You can't use that right now!" << std::endl;
					return false;
				}
				std::cout << "You threw a pokéball." << std::endl;
				std::cout << "..." << std::endl;
				theGame->Pause();
				if (random_int(1, 100) < eParty[0]->GetCatchRate())
				{
					std::cout << "It broke free!" << std::endl;
					return false;
				}
				std::cout << "You caught " << eParty[0]->GetName() << "!" << std::endl;
				if (AUTOHEAL)
				{
					eHp = totalEhp;
				}
				theGame->AddPartyMemberFull(eParty[0], eHp);
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

			if (!midMove[0])
			{
				std::cout << partyMember->GetNickname() << " knows these moves:" << std::endl;
				for (auto v = 0; v < MOVE_MEM; v++)
				{
					if (pMoves[v] != "")
					{
						std::cout << pMoves[v] << std::endl;
					}
				}

				std::cout << "Attack with which move?" << std::endl;
				std::getline(std::cin, choice);
				if (choice == "")
				{
					return false;
				}
				toupper(choice);
				if (choice == "DISABLED")
				{
					return false;
				}

				for (auto m = 0; m < MOVE_MEM; m++)
				{
					std::string pMove = pMoves[m];
					toupper(pMove);
					if (pMove == choice)
					{
						pChoice = m;
						pAttack = true;
					}
				}
				if (!pAttack)
				{
					return false;
				}
			}
		}

		/* Pick a random enemy move. */
		// TODO: Favor attacking moves?
		int eChoice = random_int(0, eMvCount);

		/* Is the player going first? */
		int pSpeed = pStat[SPEED];
		int eSpeed = eStat[SPEED];
		if (partyMember->Paralyzed())
		{
			pSpeed /= 2;
		}
		if (eParty[eMember]->Paralyzed())
		{
			eSpeed /= 2;
		}
		bool pFirst = pSpeed > eSpeed;

		/* Equal speeds should select the who goes first randomly. */
		if (pSpeed == eSpeed)
		{
			pFirst = random_int(0, 2);
		}

		/*
			If only one monster is using Quick Attack,
			pFirst will equal whether that monster was the player's.
		// */
		if ((MovesByName(pMoves[pChoice])->GetEffect() == QUICK) !=
			(MovesByName(eMoves[eChoice])->GetEffect() == QUICK))
		{
			pFirst = MovesByName(pMoves[pChoice])->GetEffect() == QUICK;
		}

		/* Player's move when they go first. */
		if (pAttack && pFirst)
		{
			if (midMove[0])
			{
				/* Repeat when "locked" in moves like Fly. */
				MoveAction(lastAttack[0]);
			}
			else
			{
				MoveAction(pMoves[pChoice]);
			}
		}

		/* Enemy's move. */
		enemyTurn = true;
		if (midMove[1])
		{
			/* Repeat when "locked" in moves like Fly. */
			MoveAction(lastAttack[1]);
		}
		else
		{
			MoveAction(eMoves[eChoice]);
		}
		enemyTurn = false;

		/* Player's move when they go second. */
		if (pAttack && !pFirst)
		{
			if (midMove[0])
			{
				/* Repeat when "locked" in moves like Fly. */
				MoveAction(lastAttack[0]);
			}
			else
			{
				MoveAction(pMoves[pChoice]);
			}
		}

	}

	/* Battle ended by move (e.g. Teleport) */
	if (!bStarted)
	{
		EndBattle();
		return true;
	}

	if (theGame->GetChoice() == "RUN")
	{
		// Player's monster is still attacking.
		if (midMove[0])
		{
			std::cout << "Your " << partyMember->GetNickname() << " is still attacking!" << std::endl;
			return false;
		}
		if (trainerBattle)
		{
			std::cout << "You can't run from a trainer!" << std::endl;
			return false;
		}
		std::cout << "You go away safely!" << std::endl;
		EndBattle();
		return true;
	}

	/* Post-Turn Effects */
	if (combatTurn)
	{
		if (eParty[eMember]->Burned())
		{
			eHp -= poisonDamage(totalEhp);
			std::cout << "Enemy " << eParty[eMember]->GetName() << " was hurt by their burn!" << std::endl;
		}
		if (partyMember->Burned())
		{
			pHp -= poisonDamage(totalPhp);
			std::cout << "Your " << partyMember->GetNickname() << " was hurt by their burn!" << std::endl;
		}
		if (eParty[eMember]->Poisoned())
		{
			int e_dm = poisonDamage(totalEhp);
			if (bPoisonNum[1])
			{
				e_dm *= bPoisonNum[1];
				bPoisonNum[1]++;
			}
			eHp -= e_dm;
			std::cout << "Enemy " << eParty[eMember]->GetName() << " was hurt by poison!" << std::endl;
		}
		if (partyMember->Poisoned())
		{
			int p_dm = poisonDamage(totalPhp);
			if (bPoisonNum[0])
			{
				p_dm *= bPoisonNum[0];
				bPoisonNum[0]++;
			}
			pHp -= p_dm;
			std::cout << "Your " << partyMember->GetNickname() << " was hurt by poison!" << std::endl;
		}
		if (eParty[eMember]->Seeded())
		{
			int e_dm = poisonDamage(totalEhp);
			if (e_dm >= eHp)
			{
				e_dm = eHp - 1;
			}
			if (e_dm)
			{
				eHp -= e_dm;
				pHp += e_dm;
				if (pHp > totalPhp)
				{
					pHp = totalPhp;
				}
				std::cout << "Seed on enemy " << eParty[eMember]->GetName() <<
					" transferred " << std::to_string(e_dm) << " hit points!" << std::endl;
			}
		}
		if (partyMember->Seeded())
		{
			int p_dm = poisonDamage(totalPhp);
			if (p_dm >= pHp)
			{
				p_dm = pHp - 1;
			}
			if (p_dm)
			{
				pHp -= p_dm;
				eHp += p_dm;
				if (eHp > totalEhp)
				{
					eHp = totalEhp;
				}
				std::cout << "Seed on your " << partyMember->GetNickname() <<
					" transferred " << std::to_string(p_dm) << " hit points!" << std::endl;
			}
		}
	}

	PrintHealth();
	std::cout << std::endl;

	if (pHp <= 0)
	{
		if (substituted[0])
		{
			std::cout << "Your " << partyMember->GetName() << "'s substitute has broken!" << std::endl;
			substituted[0] = false;
			pHp = pSubstituteHP[0];
		}
		else
		{
			std::cout << partyMember->GetName() << " can no longer fight!" << std::endl;
			if (!AUTOHEAL)
			{
				partyMember->SetHP(0);
			}
			partyMember->ClearDebuffs();
			bPoisonNum[0] = 0;

			if (!FindPartyMember())
			{
				std::cout << "You lost!" << std::endl;
				EndBattle();
				// TODO: Blackout
				theGame->SetRoom("GameOver");
				return true;
			}
		}
		PrintHealth();
	}
	if (eHp <= 0)
	{
		if (substituted[1])
		{
			std::cout << "Enemy " << partyMember->GetName() << "'s substitute has broken!" << std::endl;
			substituted[1] = false;
			eHp = pSubstituteHP[1];
			PrintHealth();
			return false;
		}

		std::cout << "Enemy " << eParty[eMember]->GetName() << " fainted!" << std::endl;
		savedHP[eMember] = 0; // In case switching set this earlier.

		partyMember->AwardEV(eParty[eMember]->GetBaseHealth(), HEALTH);
		partyMember->AwardEV(eParty[eMember]->GetBaseAttack(), ATTACK_STAT);
		partyMember->AwardEV(eParty[eMember]->GetBaseDefense(),DEFENSE);
		partyMember->AwardEV(eParty[eMember]->GetBaseSpAtk(),  SATTACK);
		partyMember->AwardEV(eParty[eMember]->GetBaseSpDef(),  SDEFENSE);
		partyMember->AwardEV(eParty[eMember]->GetBaseSpeed(),  SPEED);

		std::cout << "You won!" << std::endl;
		int xp = CalcExp();
		for (auto r = 0; r < PARTYSIZE; r++)
		{
			if (theGame->GetPartyMember(r))
			{
				if ((participated[r]) && (theGame->GetPartyMember(r)->GetHP() > 0))
				{
					if (theGame->GetPartyMember(r)->GetLevel() < MAX_LEVEL)
					{
						std::cout << theGame->GetPartyMember(r)->GetNickname() << " gained " << xp << " experience!" << std::endl;
					}
					theGame->GetPartyMember(r)->AwardXP(xp);
				}
			}
		}

		if (trainerBattle)
		{
			if (ePartySz > 1)
			{
				int nextE = SearchEnemies();
				if (nextE >= 0)
				{
					eMember = nextE;
					/* Start another battle while preserving the encounter data. */
					StartBattle(true);
					return true;
				}
			}
			int reward = eLevel * 10;
			theGame->AddMoney(reward);
			std::cout << "You got " << Money(reward) << " for winning." << std::endl;
		}
		if (sCoins)
		{
			theGame->AddMoney(sCoins);
			std::cout << "You gathered " << Money(sCoins) << " from the ground." << std::endl;
		}
		EndBattle();
		return true;
	}
	return false;
}

Move* CombatSys::MovesByName(std::string mv)
{
	for (auto m = 0; m < mCount; m++)
	{
		if (moveList[m]->GetName() == mv)
		{
			return moveList[m];
		}
	}
	/* Return tackle in case no move was be found. */
	return moveList[33];
}

void CombatSys::MoveAction(std::string mov)
{
	/* Don't process moves if the battle never started. */
	if (!bStarted)
	{
		return;
	}

	Move* move = MovesByName(mov);
	int mv_eff = move->GetEffect();
	std::string summ = "";
	std::string sUser = eParty[eMember]->GetName();
	std::string sTarg = partyMember->GetNickname();
	int level = partyMember->GetLevel();
	Types t1 = eParty[eMember]->GetType();
	Types t2 = eParty[eMember]->GetType(false);
	bool missed = false;
	lastAttack[enemyTurn] = "";
	/*
		Set this as the last move after making
		sure that the monster can actually attack.
	// */

	if (enemyTurn)
		summ += "Enemy";
	else
	{
		sTarg = eParty[eMember]->GetName();
		sUser = partyMember->GetNickname();
		level = eLevel;
		Types t1 = partyMember->GetType();
		Types t2 = partyMember->GetType(false);
		summ += "Your";
	}
	summ += " " + sUser + " ";

	/* Make sure the monster can attack. */
	Enemy* checkMon = partyMember;
	if (enemyTurn)
	{
		checkMon = eParty[eMember];
	}
	if (debuffTurns[enemyTurn][FLINCHING] || checkMon->Asleep() || checkMon->Frozen() || checkMon->Paralyzed())
	{
		bool noAttack = true;

		/* Nested if statements prevent the reporting of multiple conditions. (Just in case.) */
		if (checkMon->Frozen())
		{
			/* Chance to thaw. */
			if (random_int(1, 100) <= DB_CHANCE)
			{
				noAttack = false;
				checkMon->Freeze(false);
				summ += "thawed out";
			}
			else
			{
				summ += "is frozen solid";
			}
		}
		else
		{
			if (checkMon->Paralyzed())
			{
				/* Chance to attack anyway. */
				if (random_int(1, 100) <= DB_CHANCE)
				{
					noAttack = false;
				}
				summ += "is paralyzed";
			}
			else
			{
				if (checkMon->Asleep())
				{
					if (debuffTurns[enemyTurn][ASLEEP] <= 0)
					{
						noAttack = false;
						checkMon->Sleep(false);
						summ += "woke up";
					}
					else
					{
						--debuffTurns[enemyTurn][ASLEEP];
						summ += "is fast asleep";
					}
				}
				else
				{
					if (debuffTurns[enemyTurn][FLINCHING])
					{
						summ += "flinched";
					}
				}
			}
		}
		/* We'll just use this to interrupt the attack. */
		if (noAttack)
		{
			mv_eff = NUM_EFFECTS;
		}
		else
		{
			summ += "! They used " + move->GetName() + ", ";
		}

		/*
			Clearing flinch here will make sure a paralyzed monster
			doesn't "bank" the need to flinch.
		// */
		debuffTurns[enemyTurn][FLINCHING] = 0;
	}
	else
	{
		if (debuffTurns[enemyTurn][CONFUSED])
		{
			--debuffTurns[enemyTurn][CONFUSED];
			if (debuffTurns[enemyTurn][CONFUSED] <= 0)
			{
				summ += "is no longer confused! They used " + move->GetName() + ", ";
			}
			else
			{
				summ += "is confused, ";
				if (random_int(1, 100) <= DB_CHANCE)
				{
					mv_eff = SELF_DMG;
				}
			}
		}
		else
		{
			summ += "used " + move->GetName() + ", ";
		}
	}

	/* Some moves cannot be copied or disabled. */
	if (mov != "STRUGGLE")
	{
		lastAttack[enemyTurn] = mov;
	}

	if (debuffTurns[enemyTurn][REFLECTING])
	{
		--debuffTurns[enemyTurn][REFLECTING];
	}
	if (debuffTurns[enemyTurn][SCREENED])
	{
		--debuffTurns[enemyTurn][SCREENED];
	}

	/* Unless the opponent is using a repetitive move like Bide, reset damage counter from last time. */
	if (debuffTurns[!enemyTurn][LONG_MOVE])
	{
		/*
			Moves like Counter use this.
			Bide tracks it over multiple turns.
		// */
		lastDamage[enemyTurn] = 0;
	}

	/* Check Metronome first as it could become any move. */
	if (mv_eff == METRONOME)
	{
		Move* newM = moveList[random_int(1, mCount - 1)];
		/* If Metronome picked itself */
		if (newM->GetEffect() == move->GetEffect())
		{
			/* Use random damage */
			newM = MovesByName("PSYWAVE");
		}
		move = newM;
		mv_eff = move->GetEffect();
		summ += "which used " + move->GetName() + ", ";
		// Copied move becomes the last move
		lastAttack[enemyTurn] = newM->GetName();
	}

	/*
		Mirror Move cannot become Metronome as Metronome
		resets the "last move" to the move that it picked.
	// */
	if (mv_eff == MIRROR)
	{
		if (lastAttack[!enemyTurn] == "")
		{
			summ += "but it failed";
			std::cout << summ << "!" << std::endl;
			return;
		}
		move = MovesByName(lastAttack[!enemyTurn]);
		mv_eff = move->GetEffect();
		summ += "which used " + move->GetName() + ", ";
		// Copied move becomes the last move
		lastAttack[enemyTurn] = lastAttack[!enemyTurn];
	}

	if (mv_eff == NO_EFFECT)
	{
		summ += "and nothing happened";
	}

	if (mv_eff == SPLASH)
	{
		summ += "and they just splashed around";
	}

	if (mv_eff == DEV_EFFECT)
	{
		for (auto dev = 0; dev < 34; dev++)
			summ += "developers, ";
		summ += "developers";
	}

	if (mv_eff == BIDE)
	{
		if (debuffTurns[enemyTurn][LONG_MOVE] == 0)
		{
			if (midMove[enemyTurn])
			{
				midMove[enemyTurn] = false;
				summ += "finished biding their time, ";
				mv_eff = COUNTER; /* They work the same after this. */
			}
			else
			{
				lastDamage[enemyTurn] = 0; /* Moves like Counter want this. */
				summ += "and began biding their time";
				debuffTurns[enemyTurn][LONG_MOVE] = 2 + random_int(0, 1); /* 2 or 3 turns */
				midMove[enemyTurn] = true;
			}
		}
		else
		{
			summ += "and continued biding their time";
			--debuffTurns[enemyTurn][LONG_MOVE];
		}
	}

	if (mv_eff == THRASH)
	{
		if (debuffTurns[enemyTurn][LONG_MOVE] == 0)
		{
			if (midMove[enemyTurn])
			{
				midMove[enemyTurn] = false;
				summ += "stopped thrashing";
				if (random_int(1, 100) <= DB_CHANCE)
				{
					summ += " and became confused";
					debuffTurns[enemyTurn][CONFUSED] = random_int(1, DB_TURNS);
				}
				summ += ", ";
			}
			else
			{
				summ += "and began thrashing";
				debuffTurns[enemyTurn][LONG_MOVE] = 2 + random_int(0, 1); /* 2 or 3 turns */
				midMove[enemyTurn] = true;
			}
		}
		else
		{
			summ += "and continued thrashing";
			--debuffTurns[enemyTurn][LONG_MOVE];
		}
	}

	if (mv_eff == RAGE)
	{
		if (!rageCount[enemyTurn])
		{
			rageCount[enemyTurn] = 1;
		}
	}
	else
	{
		rageCount[enemyTurn] = 0;
	}

	/*
		Bind should prevent the target from switching out.
	// */
	if (mv_eff == BIND)
	{
		if (debuffTurns[enemyTurn][LONG_MOVE] == 0)
		{
			if (midMove[enemyTurn])
			{
				midMove[enemyTurn] = false;
				// Bind Ended
			}
			else
			{
				// Bind Begins
				debuffTurns[enemyTurn][LONG_MOVE] = 2 + random_int(0, 3); /* 2-5 turns */
				midMove[enemyTurn] = true;
			}
		}
		else
		{
			// Bind Continues
			--debuffTurns[enemyTurn][LONG_MOVE];
		}
	}

	/*
		In current generations, Whirlwind pulls out another party member -
		In trainer and wild battles.
	// */
	if (mv_eff == WHIRLWIND)
	{
		if (enemyTurn)
		{
			int totalMon = theGame->GetPartySize();
			if (totalMon > 1)
			{
				for (auto p = 0; p < theGame->GetPartySize(); p++)
				{
					if (theGame->GetPartyMember(p))
					{
						if (theGame->GetPartyMember(p)->GetHP() <= 0)
						{
							--totalMon;
						}
					}
				}
			}
			if (totalMon > 1)
			{
				/* Don't pick the number that == partyMember */
				int newMon = random_int(0, totalMon - 1);
				if (theGame->GetPartyMember(newMon) == partyMember)
				{
					// If we're at the end of the party go back.
					if (newMon == totalMon - 1)
					{
						--newMon;
					}
					else
					{
						// Otherwise, go forward.
						newMon++;
					}
				}
				SwitchPartyMember(newMon, false);
				summ += "and " + partyMember->GetNickname() + " was dragged out";
			}
			else
			{
				summ += "but it failed";
			}
		}
		else
		{
			if (trainerBattle)
			{
				// Save HP to prep for switching.
				savedHP[eMember] = eHp;

				int newMem = SearchEnemies();
				if (newMem >= 0)
				{
					eMember = newMem;
					summ += "and " + enemies[encZone->enemies[newMem]]->GetName() + " was dragged out";
					ResetEnemy();
				}
				else
				{
					// No one to switch with.
					summ += "but it failed";
				}
			}
			else
			{
				// Always fail against wild pokemon.
				summ += "but it failed";
			}
		}
	}

	if (mv_eff == TELEPORT)
	{
		if (trainerBattle)
		{
			summ += "but it failed";
		}
		else
		{
			summ += "and teleported to safety";
			bStarted = false;
		}
	}

	if (mv_eff == CONVERSION)
	{
		summ += "and is now ";
		if (enemyTurn)
		{
			eParty[eMember]->SetType(MovesByName(eMoves[0])->GetType());
			summ += GetTypeName(eParty[eMember]->GetType());
		}
		else
		{
			partyMember->SetType(MovesByName(pMoves[0])->GetType());
			summ += GetTypeName(partyMember->GetType());
		}
		summ += " type";
	}

	if (mv_eff == TRANSFORM)
	{
		if (Transform())
		{
			summ += "and transformed into ";
			if (enemyTurn)
			{
				summ += sTarg;
			}
			else
			{
				/* Species name, not nickname */
				summ += eParty[eMember]->GetName();
			}
		}
		else
		{
			summ += "but it failed";
		}
	}

	if (mv_eff == REFLECT)
	{
		debuffTurns[enemyTurn][REFLECTING] = 5;
		summ += "protecting themself from physical attacks";
	}
	if (mv_eff == SCREEN)
	{
		debuffTurns[enemyTurn][SCREENED] = 5;
		summ += "protecting themself from special attacks";
	}

	if (mv_eff == GUARD)
	{
		if (!statGuarded[enemyTurn])
		{
			statGuarded[enemyTurn] = true;
			summ += "preventing stat changes";
		}
		else
		{
			summ += "but it failed";
		}
	}

	if (mv_eff == RECOVER)
	{
		int dmg = totalPhp / 2;
		if (enemyTurn)
		{
			dmg = totalEhp / 2;
			eHp += dmg;
			if (eHp > totalEhp)
			{
				eHp = totalEhp;
			}
		}
		else
		{
			pHp += dmg;
			if (pHp > totalPhp)
			{
				pHp = totalPhp;
			}
		}
		summ += "healing " + std::to_string(dmg) + " hit points";
	}

	/*
		TODO: Should the substitute protect the user from
		stat changes (like sand-attack) and debuffs like poison?
	//*/
	if (mv_eff == SUBSTITUTE)
	{
		bool fail = false;

		if (enemyTurn)
		{
			int need = totalEhp / 4;
			bool fail = pHp <= need;
			if (!fail)
			{
				eHp -= need;
				pSubstituteHP[1] = eHp;
				eHp = need;
			}
		}
		else
		{
			int need = totalPhp / 4;
			bool fail = pHp <= need;
			if (!fail)
			{
				pHp -= need;
				pSubstituteHP[0] = pHp;
				pHp = need;
			}
		}

		if (fail)
		{
			summ += "but it failed";
		}
		else
		{
			substituted[enemyTurn] = true;
			summ += "and created a protective decoy";
		}
	}

	if (mv_eff == DIG)
	{
		if (!midMove[enemyTurn])
		{
			midMove[enemyTurn] = true;
			hidden[enemyTurn] = true;
			summ += "and dug a hole";
			std::cout << summ << "!" << std::endl;
			return;
		}
		else
		{
			hidden[enemyTurn] = false;
			midMove[enemyTurn] = false;
		}
	}
	if (mv_eff == CHARGEUP)
	{
		if (!midMove[enemyTurn])
		{
			midMove[enemyTurn] = true;
			summ += "and started taking in sunlight";
			std::cout << summ << "!" << std::endl;
			return;
		}
		else
		{
			midMove[enemyTurn] = false;
		}
	}
	if (mv_eff == FLY)
	{
		if (!midMove[enemyTurn])
		{
			midMove[enemyTurn] = true;
			hidden[enemyTurn] = true;
			summ += "and flew up high";
			std::cout << summ << "!" << std::endl;
			return;
		}
		else
		{
			hidden[enemyTurn] = false;
			midMove[enemyTurn] = false;
		}
	}
	if (mv_eff == RAZORWIND)
	{
		if (!midMove[enemyTurn])
		{
			midMove[enemyTurn] = true;
			summ += "and made a whirlwind";
			std::cout << summ << "!" << std::endl;
			return;
		}
		else
		{
			midMove[enemyTurn] = false;
		}
	}
	if (mv_eff == RECHARGE)
	{
		if (midMove[enemyTurn])
		{
			midMove[enemyTurn] = false;
			summ += "and must recharge";
			std::cout << summ << "!" << std::endl;
			return;
		}
		else
		{
			midMove[enemyTurn] = true;
		}
	}
	if (mv_eff == SKULLBASH)
	{
		if (!midMove[enemyTurn])
		{
			midMove[enemyTurn] = true;
			summ += "and lowered their head";
			std::cout << summ << "!" << std::endl;
			return;
		}
		else
		{
			midMove[enemyTurn] = false;
		}
	}
	if (mv_eff == SKYATTACK)
	{
		if (!midMove[enemyTurn])
		{
			midMove[enemyTurn] = true;
			summ += "and started glowing";
			std::cout << summ << "!" << std::endl;
			return;
		}
		else
		{
			midMove[enemyTurn] = false;
		}
	}

	if (mv_eff == RAND_DMG)
	{
		/*
			Random damage between 1 and 1.5x the user's level.
			Resistance is ignored, but not immunity.
		// */
		int dmg = 1;
		if (TypeEffectiveness(t1, t2, move->GetType()) != IMMUNE)
		{
			level *= int(1.5);
			dmg += random_int(0, level);
		}

		if (enemyTurn)
		{
			pHp -= dmg;
		}
		else
		{
			eHp -= dmg;
		}
		if (dmg)
		{
			summ += "dealing " + std::to_string(dmg) + " point";
			if (dmg != 1)
			{
				summ += "s";
			}
			summ += " of damage";
			rageCount[!enemyTurn]++;
		}
		lastDamage[enemyTurn] += dmg;
	}

	if (mv_eff == SELF_DMG)
	{
		/* Confusion damage is like an untyped move with a power of 40. */
		int dmg = 1;

		int attackStat  = pStat[ATTACK_STAT];
		int defenseStat = pStat[DEFENSE];
		if (enemyTurn)
		{
			attackStat  = eStat[ATTACK_STAT];
			defenseStat = eStat[DEFENSE];
		}
		float rnd = 0.85f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (1 - 0.85f))); // Random
		dmg += int((((2 * level / 5 + 2) * 40 * attackStat / defenseStat) / 50 + 2) * rnd);

		if (enemyTurn)
		{
			eHp -= dmg;
		}
		else
		{
			pHp -= dmg;
		}
		if (dmg)
		{
			summ += "hurting themself for " + std::to_string(dmg) + " point";
			if (dmg != 1)
			{
				summ += "s";
			}
			summ += " of damage";
		}
	}

	if (mv_eff == DISABLE)
	{
		bool fail = true;
		if (lastAttack[!enemyTurn] != "")
		{
			if (enemyTurn) // Disable a player move.
			{
				/*
					Since we're trying to disable a move
					and can't disable their last move,
					make sure that there is more than
					one available move.
				// */
				auto dMove = -1; // This will be the move to disable.
				auto pMcount = 0;
				for (auto m = 0; m < MOVE_MEM; m++)
				{
					if ((pMoves[m] != "") && (pMoves[m] != "DISABLED"))
					{
						pMcount++;
						if (pMoves[m] == lastAttack[!enemyTurn])
						{
							dMove = m; // This is the move to disable.
						}
					}
				}
				if (pMcount > 1)
				{
					if (dMove > -1)
					{
						pMoves[dMove] = "DISABLED";
						fail = false;
					}
				}
			}
			else // Disable an enemy move.
			{
				if (eMvCount > 1) // Don't disable their last move!
				{
					// Build a new move list.
					for (auto i = 0; i < MOVE_MEM;)
					{
						eMvCount = 0;
						if (eParty[eMember]->MoveName(i) != "")
						{
							if (eParty[eMember]->MoveName(i) != lastAttack[!enemyTurn])
							{
								eMoves[eMvCount] = eParty[eMember]->MoveName(i);
								eMvCount++;
							}
							else // This would mean the move was found.
							{
								fail = false;
							}
						}
						else
						{
							break;
						}
					}
				}
			}
			if (!fail)
			{
				summ += "and disabled " + lastAttack[!enemyTurn];
			}
		}
		if (fail)
		{
			summ += "but it failed";
		}
	}

	if (mv_eff == MIMIC)
	{
		int replace = 0;

		if (lastAttack[!enemyTurn] == "")
		{
			summ += "but it failed";
			std::cout << summ << "!" << std::endl;
			return;
		}

		if (enemyTurn)
		{
			for (auto em = 0; em < eMvCount; em++)
			{
				if (MovesByName(eMoves[em])->GetEffect() == MIMIC)
				{
					replace = em;
				}
			}
			eMoves[replace] = lastAttack[!enemyTurn];
		}
		else
		{
			for (auto pm = 0; pm < MOVE_MEM; pm++)
			{
				if (MovesByName(pMoves[pm])->GetEffect() == MIMIC)
				{
					replace = pm;
				}
			}
			pMoves[replace] = lastAttack[!enemyTurn];
		}
		summ += "and copied " + lastAttack[!enemyTurn];
	}

	/*
		Haze resets all stats/modifiers.
		This implementation will likely reset the
		STATS from Transform, but not the SPECIES.
	// */
	if (mv_eff == RESET)
	{
		// Recalc Party Member Stats
		for (auto ps = 0; ps < NUM_STATS; ps++)
		{
			pStat[ps] = partyMember->GetStat(ps);
		}

		// Recalc Enemy Stats
		for (auto es = 0; es < NUM_STATS; es++)
		{
			eStat[es] = eParty[eMember]->GetStat(es);
		}

		/* Accuracy stages and the like start at 0. */
		eAcc = eEvas = eCritC =
			pAcc = pEvas = pCritC = BSTAT;
	}

	if ((mv_eff == ABSORB)  ||
		(mv_eff == ATTACK_EFFECT) ||
		(mv_eff == BIND) ||
		(mv_eff == BURN_EF) ||
		(mv_eff == CHARGEUP)||
		(mv_eff == CONFUSE) ||
		(mv_eff == COUNTER) ||
		(mv_eff == CRASH)   ||
		(mv_eff == DOUBLE_EF) ||
		(mv_eff == DOUBLE_POISON)||
		(mv_eff == DRAGON_RAGE)  ||
		(mv_eff == DREAMEATER)||
		(mv_eff == FAINT) ||
		(mv_eff == FLINCH)||
		(mv_eff == FREEZE_EF)||
		(mv_eff == HALFHP)   ||
		(mv_eff == HI_CRIT)  ||
		(mv_eff == KILL_EFFECT) ||
		(mv_eff == LEVEL)   ||
		(mv_eff == MONEY)   ||
		(mv_eff == MULTIPLE)||
		(mv_eff == PARALYZE_EF)||
		(mv_eff == POISON_EF)  ||
		(mv_eff == QUICK)||
		(mv_eff == RAGE) ||
		(mv_eff == RAZORWIND)  ||
		(mv_eff == RECOIL)  ||
		(mv_eff == SKULLBASH) ||
		(mv_eff == SKYATTACK) ||
		(mv_eff == SONICBOOM) ||
		(mv_eff == THRASH) ||
		(mv_eff == TRIATTACK))
	{
		/*
			BUGBUG: CRASH should "miss" when the target is immune.
		*/
		missed = !HitChance(move->GetAccuracy());

		/* Bypass accuracy checks. */
		if (mv_eff == COUNTER)
		{
			missed = false;
		}

		if (!missed)
		{
			int numHits = 1;
			int dmg = 65535; /* The KILL_EFFECT is always a TKO. */
			if ((mv_eff == ABSORB)||
				(mv_eff == ATTACK_EFFECT) ||
				(mv_eff == BIND) ||
				(mv_eff == BURN_EF)  ||
				(mv_eff == CHARGEUP) ||
				(mv_eff == CONFUSE)  ||
				(mv_eff == CRASH) ||
				(mv_eff == DOUBLE_EF)||
				(mv_eff == DOUBLE_POISON) ||
				(mv_eff == DREAMEATER) ||
				(mv_eff == FAINT) ||
				(mv_eff == FLINCH)||
				(mv_eff == FREEZE_EF)||
				(mv_eff == HI_CRIT)  ||
				(mv_eff == MONEY) ||
				(mv_eff == MULTIPLE) ||
				(mv_eff == PARALYZE_EF)||
				(mv_eff == POISON_EF)  ||
				(mv_eff == QUICK)||
				(mv_eff == RAGE) ||
				(mv_eff == RAZORWIND)||
				(mv_eff == RECOIL)||
				(mv_eff == SKULLBASH)||
				(mv_eff == SKYATTACK)||
				(mv_eff == THRASH)||
				(mv_eff == TRIATTACK))
			{
				dmg = CalcDamage(move);

				if ((mv_eff == DOUBLE_EF) || (mv_eff == DOUBLE_POISON) || (mv_eff == MULTIPLE))
				{
					numHits = 2;
					if (mv_eff == MULTIPLE)
					{
						/* Add 0-3 more */
						numHits += random_int(0, 3);
					}
					for (auto h = 1; h < numHits; h++)
					{
						dmg += CalcDamage(move, false);
					}
				}
				if (mv_eff == DREAMEATER)
				{
					if ((enemyTurn && !partyMember->Asleep()) || (!enemyTurn && !eParty[eMember]->Asleep()))
					{
						/* No effect unless the target is asleep. */
						dmg = 0;
					}
					mv_eff = ABSORB; /* They're the same at this point. */
				}
			}

			if (mv_eff == COUNTER)
			{
				dmg = lastDamage[!enemyTurn] * 2;
			}

			if ((mv_eff == DRAGON_RAGE) ||
				(mv_eff == HALFHP)||
				(mv_eff == LEVEL) ||
				(mv_eff == SONICBOOM))
			{
				if (mv_eff == HALFHP)
				{
					if (enemyTurn)
					{
						dmg = pHp;
					}
					else
					{
						dmg = eHp;
					}
					dmg = std::max(dmg / 2, 1);
				}
				if (mv_eff == LEVEL)
				{
					dmg = level;
				}
				if (mv_eff == DRAGON_RAGE)
				{
					dmg = 40;
				}
				if (mv_eff == SONICBOOM)
				{
					dmg = 20;
				}
				dmg *= int(TypeEffectiveness(t1, t2, move->GetType()));
			}
			if (enemyTurn)
			{
				if (mv_eff == FAINT)
				{
					eHp = 0;
				}
				pHp -= dmg;
			}
			else
			{
				if (mv_eff == FAINT)
				{
					pHp = 0;
				}
				eHp -= dmg;
			}
			lastDamage[enemyTurn] += dmg;
			if ((mv_eff == ABSORB)   ||
				(mv_eff == ATTACK_EFFECT) ||
				(mv_eff == BIND) ||
				(mv_eff == BURN_EF)  ||
				(mv_eff == CHARGEUP) ||
				(mv_eff == CONFUSE)  ||
				(mv_eff == COUNTER)  ||
				(mv_eff == CRASH)    ||
				(mv_eff == DOUBLE_EF)||
				(mv_eff == DOUBLE_POISON) ||
				(mv_eff == DRAGON_RAGE) ||
				(mv_eff == FAINT)    ||
				(mv_eff == FLINCH)   ||
				(mv_eff == FREEZE_EF)||
				(mv_eff == HALFHP)   ||
				(mv_eff == HI_CRIT)  ||
				(mv_eff == LEVEL)    ||
				(mv_eff == MONEY)    ||
				(mv_eff == MULTIPLE) ||
				(mv_eff == PARALYZE_EF) ||
				(mv_eff == POISON_EF)||
				(mv_eff == QUICK)    ||
				(mv_eff == RAGE) ||
				(mv_eff == RAZORWIND)||
				(mv_eff == RECOIL)   ||
				(mv_eff == SKULLBASH)||
				(mv_eff == SKYATTACK)||
				(mv_eff == THRASH)   ||
				(mv_eff == TRIATTACK)||
				(mv_eff == SONICBOOM))
				if (dmg)
				{
					if (reportCrit)
					{
						summ += "- it was a critical hit, ";
					}
					if (TypeEffectiveness(t1, t2, move->GetType()) < NORM_D)
					{
						summ += "but it's not very effective - ";
					}
					if (TypeEffectiveness(t1, t2, move->GetType()) > NORM_D)
					{
						summ += "and it's super effective - ";
					}
					summ += "dealing " + std::to_string(dmg) + " point";
					if (dmg != 1)
					{
						summ += "s";
					}
					summ += " of damage";
					if ((mv_eff == DOUBLE_EF) || (mv_eff == DOUBLE_POISON) || (mv_eff == MULTIPLE))
					{
						summ += " in " + std::to_string(numHits) + " hit";
						if (numHits != 1)
						{
							summ += "s";
						}
					}
					if (mv_eff == MONEY)
					{
						sCoins += random_int(1, level * 2);
						summ += ", and scattering coins everywhere";
					}
				}
				else // Zero damage
				{
					summ += ", but it had no effect";
				}
			if (mv_eff == KILL_EFFECT)
			{
				summ += "and it was a total knock-out";
			}
			if (mv_eff == RECOIL)
			{
				dmg /= 4;
				if (enemyTurn)
				{
					eHp -= dmg;
				}
				else
				{
					pHp -= dmg;
				}
				summ += ", but was hit with recoil";
			}
			if (mv_eff == ABSORB)
			{
				int heal = std::max(dmg / 2, 1);
				if (enemyTurn)
				{
					eHp += heal;
					if (eHp > totalEhp)
					{
						eHp = totalEhp;
					}
				}
				else
				{
					pHp += heal;
					if (pHp > totalPhp)
					{
						pHp = totalPhp;
					}
				}
				summ += " and healing " + std::to_string(heal);
			}
		}
		else
		{
			summ += "but they missed";
			if (move->GetEffect() == CRASH)
			{
				int dmg = CalcDamage(move) / 2;
				if (enemyTurn)
				{
					pHp -= dmg;
				}
				else
				{
					eHp -= dmg;
				}
				summ += " and " + sUser + " crashed, taking " + std::to_string(dmg) + "point";
				if (dmg != 1)
				{
					summ += "s";
				}
				summ += " of damage";
				rageCount[!enemyTurn]++;
			}
			if (mv_eff == RECHARGE)
			{
				/* Hyper Beam does not recharge if it misses. */
				midMove[enemyTurn] = false;
			}
		}
	}

	/* Can't inflict status conditions if the attack missed. */
	if (!missed)
	{
		if (mv_eff == TRIATTACK)
		{
			if (random_int(1, 100) <= DB_CHANCE)
			{
				mv_eff = BURN100;
			}
			else
			{
				if (random_int(1, 100) <= DB_CHANCE)
				{
					mv_eff = FREEZE100;
				}
				else
				{
					if (random_int(1, 100) <= DB_CHANCE)
					{
						mv_eff = PARALYZE100;
					}
				}
			}
		}

		if (mv_eff == BURN_EF)
		{
			if (random_int(1, 100) <= DB_CHANCE)
			{
				mv_eff = BURN100;
			}
		}
		if (mv_eff == FLINCH)
		{
			if (random_int(1, 100) <= DB_CHANCE)
			{
				debuffTurns[!enemyTurn][FLINCHING] = 1;
			}
		}
		if (mv_eff == FREEZE_EF)
		{
			if (random_int(1, 100) <= DB_CHANCE)
			{
				mv_eff = FREEZE100;
			}
		}
		if (mv_eff == PARALYZE_EF)
		{
			if (random_int(1, 100) <= DB_CHANCE)
			{
				mv_eff = PARALYZE100;
			}
		}
		if ((mv_eff == POISON_EF) || (mv_eff == DOUBLE_POISON))
		{
			if (random_int(1, 100) <= DB_CHANCE)
			{
				mv_eff = POISON100;
			}
			/* Second chance for second attack */
			if (mv_eff == DOUBLE_POISON)
			{
				if (random_int(1, 100) <= DB_CHANCE)
				{
					mv_eff = POISON100;
				}
			}
		}
		if (mv_eff == SLEEP_EF)
		{
			if (random_int(1, 100) <= DB_CHANCE)
			{
				mv_eff = SLEEP100;
			}
		}

		if (mv_eff == CONFUSE)
		{
			summ += " and ";
			if (enemyTurn)
			{
				summ += "Your";
			}
			else
			{
				summ += "Enemy";
			}
			debuffTurns[!enemyTurn][CONFUSED] = random_int(1, DB_TURNS);
			summ += " " + sTarg + " became confused";
		}
	}

	bool debuff_fail = false; /* Fail if immune (e.g. by type) */

	if (mv_eff == BURN100)
	{
		if (enemyTurn)
		{
			debuff_fail =
				(partyMember->GetType() == Types::FIRE) ||
				(partyMember->GetType(false) == Types::FIRE);
		}
		else
		{
			debuff_fail =
				(eParty[eMember]->GetType() == Types::FIRE) ||
				(eParty[eMember]->GetType(false) == Types::FIRE);
		}
		summ += " and ";
		if (enemyTurn)
		{
			summ += "Your";
			partyMember->Burn(!debuff_fail);
		}
		else
		{
			summ += "Enemy";
			eParty[eMember]->Burn(!debuff_fail);
		}
		summ += " " + sTarg + " ";
		if (!debuff_fail)
		{
			summ += "was burned";
		}
		else
		{
			summ += "resisted burning";
		}
	}
	if (mv_eff == FREEZE100)
	{
		if (enemyTurn)
		{
			debuff_fail =
				(partyMember->GetType() == Types::ICE) ||
				(partyMember->GetType(false) == Types::ICE);
		}
		else
		{
			debuff_fail =
				(eParty[eMember]->GetType() == Types::ICE) ||
				(eParty[eMember]->GetType(false) == Types::ICE);
		}
		summ += " and ";
		if (enemyTurn)
		{
			summ += "Your";
			partyMember->Freeze(!debuff_fail);
		}
		else
		{
			summ += "Enemy";
			eParty[eMember]->Freeze(!debuff_fail);
		}
		summ += " " + sTarg + " ";
		if (!debuff_fail)
		{
			summ += "was frozen";
		}
		else
		{
			summ += "resisted freezing";
		}
	}
	if (mv_eff == PARALYZE100)
	{
		if (enemyTurn)
		{
			debuff_fail =
				(partyMember->GetType() == Types::ELECTRIC) ||
				(partyMember->GetType(false) == Types::ELECTRIC);
		}
		else
		{
			debuff_fail =
				(eParty[eMember]->GetType() == Types::ELECTRIC) ||
				(eParty[eMember]->GetType(false) == Types::ELECTRIC);
		}
		summ += " and ";
		if (enemyTurn)
		{
			summ += "Your";
			partyMember->Paralyze(!debuff_fail);
		}
		else
		{
			summ += "Enemy";
			eParty[eMember]->Paralyze(!debuff_fail);
		}
		summ += " " + sTarg + " ";
		if (!debuff_fail)
		{
			summ += "was paralyzed";
		}
		else
		{
			summ += "resisted paralysis";
		}
	}
	if ((mv_eff == POISON100) || (mv_eff == BADPOISON))
	{
		if (mv_eff == POISON100)
		{
			if (enemyTurn)
			{
				debuff_fail = partyMember->Poisoned();
			}
			else
			{
				debuff_fail = eParty[eMember]->Poisoned();
			}
		}
		if (mv_eff == BADPOISON)
		{
			debuff_fail = bPoisonNum[!enemyTurn] > 0;
		}
		if (!debuff_fail)
		{
			summ += " and ";
			if (enemyTurn)
			{
				summ += "Your";
				partyMember->Poison();
			}
			else
			{
				summ += "Enemy";
				eParty[eMember]->Poison();
			}
			summ += " " + sTarg + " was ";
			if (mv_eff == BADPOISON)
			{
				summ += "badly ";
				bPoisonNum[!enemyTurn] = 1;
			}
			summ += "poisoned";
		}
		else
		{
			summ += " and ";
			if (enemyTurn)
			{
				summ += "Your";
			}
			else
			{
				summ += "Enemy";
			}
			summ += " " + sTarg + " is already poisoned";
		}
	}
	if (mv_eff == SEED_EF)
	{
		if (enemyTurn)
		{
			debuff_fail = partyMember->Seeded();
		}
		else
		{
			debuff_fail = eParty[eMember]->Seeded();
		}

		if (!debuff_fail)
		{
			summ += " and ";
			if (enemyTurn)
			{
				summ += "Your";
				partyMember->Seed();
			}
			else
			{
				summ += "Enemy";
				eParty[eMember]->Seed();
			}
			summ += " " + sTarg + " was seeded";
		}
		else
		{
			bool sameT = false;

			summ += " and ";
			if (enemyTurn)
			{
				summ += "Your";
			}
			else
			{
				summ += "Enemy";
			}
			summ += " " + sTarg + " ";

			if (enemyTurn)
			{
				sameT =
					(partyMember->GetType() == Types::GRASS) ||
					(partyMember->GetType(false) == Types::GRASS);
			}
			else
			{
				sameT =
					(eParty[eMember]->GetType() == Types::GRASS) ||
					(eParty[eMember]->GetType(false) == Types::GRASS);
			}
			if (sameT)
			{
				summ += "resisted seeding";
			}
			else
			{
				summ += "is already seeded";
			}
		}
	}
	if (mv_eff == SLEEP100)
	{
		if (enemyTurn)
		{
			debuff_fail = partyMember->Asleep();
		}
		else
		{
			debuff_fail = eParty[eMember]->Asleep();
		}
		if (!debuff_fail)
		{
			summ += " and ";
			if (enemyTurn)
			{
				summ += "Your";
				partyMember->Sleep();
			}
			else
			{
				summ += "Enemy";
				eParty[eMember]->Sleep();
			}
			debuffTurns[!enemyTurn][ASLEEP] = random_int(1, DB_TURNS);
			summ += " " + sTarg + " fell asleep";
		}
		else
		{
			summ += " but ";
			if (enemyTurn)
			{
				summ += "Your";
			}
			else
			{
				summ += "Enemy";
			}
			summ += " " + sTarg + " is already asleep";
		}
	}

	if (mv_eff == REST)
	{
		if ((enemyTurn && (eHp == totalEhp)) || (!enemyTurn && (pHp == totalPhp)))
		{
			/* already at full health */
			summ += ", but it failed";
		}
		else
		{
			if (enemyTurn)
			{
				eParty[eMember]->ClearDebuffs();
				eParty[eMember]->Sleep();
				eHp = totalEhp;
			}
			else
			{
				partyMember->ClearDebuffs();
				partyMember->Sleep();
				pHp = totalPhp;
			}
			bPoisonNum[enemyTurn] = 0;
			debuffTurns[enemyTurn][CONFUSED] = 0;
			debuffTurns[enemyTurn][FLINCHING] = 0;
			debuffTurns[enemyTurn][ASLEEP] = 2;
			summ += ", and fell asleep to heal";
		}
	}

	/* Stat-changing moves */
	if ((move->GetEffect() == BLIND) ||
		(move->GetEffect() == LOWER_ATTACK) ||
		(move->GetEffect() == LOWER_DEFENSE) ||
		(move->GetEffect() == LOWER_DEFENSE2) ||
		(move->GetEffect() == LOWER_SPDEFENSE) ||
		(move->GetEffect() == LOWER_SPDEFENSE2) ||
		(move->GetEffect() == RAISE_ATTACK)  ||
		(move->GetEffect() == RAISE_ATTACK2) ||
		(move->GetEffect() == RAISE_CRITS)   ||
		(move->GetEffect() == RAISE_DEFENSE) ||
		(move->GetEffect() == RAISE_DEFENSE2)||
		(move->GetEffect() == RAISE_EVASION) ||
		(move->GetEffect() == RAISE_SPATTACK)||
		(move->GetEffect() == RAISE_SPEED)   ||
		(move->GetEffect() == RAISE_SPEED2)  ||
		(move->GetEffect() == SLOW))
	{
		/* Positive Effects */
		std::string sstat = "Stats";
		std::string dir = "rais"; // Raising or lowering
		std::string target = sUser;
		int amt = 1;

		if (move->GetEffect() == RAISE_EVASION)
		{
			sstat = "Evasion";
			if (enemyTurn)
			{
				eEvas = std::clamp(eAcc + amt, -6, 6);
			}
			else
			{
				pEvas = std::clamp(pAcc + amt, -6, 6);
			}
		}
		if (move->GetEffect() == RAISE_CRITS)
		{
			sstat = "critical chance";
			if (enemyTurn)
			{
				eCritC++;
			}
			else
			{
				pCritC++;
			}
		}
		/* TODO: Can't go any higher? */
		if ((move->GetEffect() == RAISE_ATTACK)  ||
			(move->GetEffect() == RAISE_ATTACK2) ||
			(move->GetEffect() == RAISE_DEFENSE) ||
			(move->GetEffect() == RAISE_DEFENSE2)||
			(move->GetEffect() == RAISE_SPATTACK)||
			(move->GetEffect() == RAISE_SPEED)   ||
			(move->GetEffect() == RAISE_SPEED2))
		{
			int aStat = ATTACK_STAT; // RAISE_ATTACK(2)
			if ((move->GetEffect() == RAISE_DEFENSE)||
				(move->GetEffect() == RAISE_DEFENSE2))
			{
				aStat = DEFENSE;
			}
			if (move->GetEffect() == RAISE_SPATTACK)
			{
				aStat = SATTACK;
			}
			if ((move->GetEffect() == RAISE_SPEED) ||
				(move->GetEffect() == RAISE_SPEED2))
			{
				aStat = SPEED;
			}
			sstat = StatName(aStat);

			if ((move->GetEffect() == RAISE_ATTACK2) ||
				(move->GetEffect() == RAISE_DEFENSE2)||
				(move->GetEffect() == RAISE_SPEED2))
			{
				amt *= 2;
			}

			if (enemyTurn)
			{
				eStat[aStat] += amt;
			}
			else
			{
				pStat[aStat] += amt;
			}
		}

		/* Negative Effects */
		if ((move->GetEffect() == BLIND) ||
			(move->GetEffect() == LOWER_ATTACK) ||
			(move->GetEffect() == LOWER_DEFENSE)||
			(move->GetEffect() == LOWER_DEFENSE2)  ||
			(move->GetEffect() == LOWER_SPDEFENSE) ||
			(move->GetEffect() == LOWER_SPDEFENSE2)||
			(move->GetEffect() == SLOW))
		{
			dir = "lower";
			amt = -1;
			target = sTarg;
		}

		if (move->GetEffect() == BLIND)
		{
			sstat = "Accuracy";
			if (enemyTurn)
			{
				pAcc = std::clamp(pAcc + amt, -6, 6);
			}
			else
			{
				eAcc = std::clamp(eAcc + amt, -6, 6);
			}
		}
		/* TODO: Can't go any lower? */
		if ((move->GetEffect() == LOWER_ATTACK) ||
			(move->GetEffect() == LOWER_DEFENSE)||
			(move->GetEffect() == LOWER_DEFENSE2)  ||
			(move->GetEffect() == LOWER_SPDEFENSE) ||
			(move->GetEffect() == LOWER_SPDEFENSE2)||
			(move->GetEffect() == SLOW))
		{

			int aStat = DEFENSE; // LOWER_DEFENSE
			if (move->GetEffect() == LOWER_ATTACK)
			{
				aStat = ATTACK_STAT;
			}
			if ((move->GetEffect() == LOWER_SPDEFENSE)||
				(move->GetEffect() == LOWER_SPDEFENSE2))
			{
				aStat = SDEFENSE;
			}
			if (move->GetEffect() == SLOW)
			{
				aStat = SPEED;
			}
			sstat = StatName(aStat);

			if ((move->GetEffect() == LOWER_DEFENSE2) ||
				(move->GetEffect() == LOWER_SPDEFENSE2))
			{
				amt *= 2;
			}
			if (!statGuarded[!enemyTurn])
			{
				if (enemyTurn)
				{
					pStat[aStat] += amt;
				}
				else
				{
					eStat[aStat] += amt;
				}
			}
		}
		tolower(sstat);

		if ((dir == "lower") && (statGuarded[!enemyTurn]))
		{
			summ += target + " resisted stat changes";
		}
		else
		{
			summ += dir + "ing " + target + "'s " + sstat;
		}
	}
	std::cout << summ << "!" << std::endl;
}

#pragma once
#include "types.h"

class Move
{
public:
	Move();
	std::string GetName();
	int GetPP(), GetPower(),
		GetAccuracy(),
		GetEffect();
	Types GetType();
	void Setup(
		std::string name = "Unknown",
		std::string typ = "Normal",
		int pp = 0, int power = 0,
		int accuracy = 100,
		std::string fx = "Nothing");
private:
	std::string name;
	int pp, power,
		accuracy,
		effect;
	Types mType;
};

int LookupEffect(std::string eff);

const enum EFFECTS
{
	NO_EFFECT,
	DEV_EFFECT,
	ABSORB,
	ATTACK_EFFECT,
	BADPOISON,
	BIDE,
	BIND,
	BLIND,
	BURN_EF,
	BURN100,
	CHARGEUP,
	CONFUSE,
	CONVERSION,
	COUNTER,
	CRASH,
	DIG,
	DISABLE,
	DOUBLE_EF,
	DOUBLE_POISON,
	DRAGON_RAGE,
	DREAMEATER,
	FAINT,
	FLINCH,
	FLY,
	FREEZE_EF,
	FREEZE100,
	GUARD,
	HALFHP,
	HI_CRIT,
	KILL_EFFECT,
	LEVEL,
	LOWER_ATTACK,
	LOWER_DEFENSE,
	LOWER_DEFENSE2,
	LOWER_SPDEFENSE,
	LOWER_SPDEFENSE2,
	METRONOME,
	MIMIC,
	MIRROR,
	MONEY,
	MULTIPLE,
	PARALYZE_EF,
	PARALYZE100,
	POISON_EF,
	POISON100,
	QUICK,
	RAISE_ATTACK,
	RAISE_ATTACK2,
	RAISE_CRITS,
	RAISE_DEFENSE,
	RAISE_DEFENSE2,
	RAISE_EVASION,
	RAISE_SPATTACK,
	RAISE_SPEED,
	RAISE_SPEED2,
	RAND_DMG,
	RAGE,
	RAZORWIND,
	RECHARGE,
	RECOIL,
	RECOVER,
	REFLECT,
	RESET,
	REST,
	SCREEN,
	SEED_EF,
	SELF_DMG, // Used when confused
	SKULLBASH,
	SKYATTACK,
	SPLASH,
	SLEEP_EF,
	SLEEP100,
	SLOW,
	SONICBOOM,
	SUBSTITUTE,
	TELEPORT,
	THRASH,
	TRANSFORM,
	TRIATTACK,
	WHIRLWIND,
	NUM_EFFECTS
};

const std::string EFFECT_NAMES[NUM_EFFECTS] =
{
	"NOTHING",
	"DEV",
	"ABSORB",
	"ATTACK",
	"BADPOISON",
	"BIDE",
	"BIND",
	"BLIND",
	"BURN",
	"BURN100",
	"CHARGEUP",
	"CONFUSE",
	"CONVERSION",
	"COUNTER",
	"CRASH",
	"DIG",
	"DISABLE",
	"DOUBLE",
	"DOUBLEPOISON",
	"DRAGONRAGE",
	"DREAMEATER",
	"FAINT",
	"FLINCH",
	"FLY",
	"FREEZE",
	"FREEZE100",
	"GUARD",
	"HALFHP",
	"HICRIT",
	"KILL",
	"LEVEL",
	"LOWERATTACK",
	"LOWERDEFENSE",
	"LOWERDEFENSE2",
	"LOWERSPDEFENSE",
	"LOWERSPDEFENSE2",
	"METRONOME",
	"MIMIC",
	"MIRROR",
	"MONEY",
	"MULTIPLE",
	"PARALYZE",
	"PARALYZE100",
	"POISON",
	"POISON100",
	"QUICK",
	"RAISEATTACK",
	"RAISEATTACK2",
	"RAISECRITS",
	"RAISEDEFENSE",
	"RAISEDEFENSE2",
	"RAISEEVASION",
	"RAISESPATTACK",
	"RAISESPEED",
	"RAISESPEED2",
	"RANDDMG",
	"RAGE",
	"RAZORWIND",
	"RECHARGE",
	"RECOIL",
	"RECOVER",
	"REFLECT",
	"RESET",
	"REST",
	"SCREEN",
	"SEED",
	"SELF_DMG"
	"SKULLBASH",
	"SKYATTACK",
	"SPLASH",
	"SLEEP",
	"SLEEP100",
	"SLOW",
	"SONICBOOM",
	"SUBSTITUTE",
	"TELEPORT",
	"THRASH",
	"TRANSFORM",
	"TRIATTACK",
	"WHIRLWIND"
};

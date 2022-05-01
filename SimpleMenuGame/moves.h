#pragma once

class Move
{
public:
	Move();
	std::string GetName();
	int GetPP(), GetPower(),
		GetAccuracy(), GetType(),
		GetEffect();
	void Setup(
		std::string name = "Unknown",
		std::string typ = "Normal",
		int pp = 0, int power = 0,
		int accuracy = 100,
		std::string fx = "Nothing");
private:
	std::string name;
	int pp, power, accuracy,
		mType,
		effect;
};

const enum Effects
{
	NO_EFFECT,
	ATTACK_EFFECT,
	BLIND,
	LESS_ATTACK,
	LESS_DEFENSE,
	DEV_EFFECT,
	SLOW,
	NUM_EFFECTS
};
int LookupEffect(std::string eff);

#pragma once

class Move
{
public:
	Move();
	std::string GetName();
	int GetPP(), GetPower(), GetAccuracy();
	void Setup(
		std::string name = "Unknown",
		std::string typ = "Normal",
		int pp = 0, int power = 0,
		int accuracy = 100);
private:
	std::string name;
	int pp, power, accuracy;
	int mType;
};

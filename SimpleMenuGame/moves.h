#pragma once
#include <string>

class Move
{
public:
	Move();
	std::string GetName();
	int GetPower();
	int GetPP();
	int GetAccuracy();
	void Setup(std::string name, int power, int pp, int accuracy);
private:
	std::string name;
	int power, pp, accuracy;
};
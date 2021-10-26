#pragma once
#include <string>

constexpr auto MAX_ENEMIES = 1000;

class Enemy
{
public:
	Enemy();
	std::string GetName();
	int GetHealth();
	int GetDamage();
	void Setup(std::string name, int hp=1, int dam=1);
private:
	std::string name;
	int health; // Base HP
	int damage; // Attack strength
};
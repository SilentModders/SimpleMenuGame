#include "text.h"
#include "enemies.h"

Enemy::Enemy()
{
	name = "Placeholder";
	health = 10;
	damage = 5;
}
std::string Enemy::GetName()
{
	return ColoredString(name, Color::COLOR_WHITE);
}
int Enemy::GetHealth()
{
	return health;
}
int Enemy::GetDamage()
{
	return damage;
}
void Enemy::Setup(std::string nme, int hp, int dam)
{
	name = nme;
	health = hp;
	damage = dam;
}

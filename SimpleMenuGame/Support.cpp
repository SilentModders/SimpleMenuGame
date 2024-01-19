#include "support.h"

#if defined _WIN32
	#include <Windows.h>
	#define sleep(x) Sleep(1000 * (x))
#else
	#include <unistd.h>
#endif

void GameSleep(int sec)
{
	sleep(sec);
}

int random_int(int min, int max)
{
	return min + rand() % (max + 1 - min);
}

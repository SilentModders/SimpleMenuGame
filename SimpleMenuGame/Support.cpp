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
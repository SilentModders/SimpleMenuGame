/*
    Simple Menu Game
    Uses a given script to drive a simple, menu-based game in the style of "Zork".
    Written by Chris Roxby.
//*/
#include "game.h"

#if defined _WIN32
    #include <Windows.h>
    #define sleep(x) Sleep(1000 * (x))
    #define PAUSECMD "echo Press any key to quit. && pause > nul"
#else
    #include <unistd.h>
    #define PAUSECMD "read -n1 -rs -p 'Press any key to quit.'"
#endif

#define BANNER "Simple Menu Game, Version 0.1"

int Quit(int code = EXIT_FAILURE)
{
#if _DEBUG
    //system(PAUSECMD);
#endif
    return code;
}

bool GameLoop()
{
    /* Load up the previous choice. */
    std::string choice = GetChoice();
    ChooseRoom(choice);
    
    /* Don't print an empty line
    if the last input was blank. */
    if (GetChoice()!="")
    {
        std::cout << std::endl;
    }
    /* Print the room's text. */
    std::cout << RoomText(GetRoom()) << std::endl;

    /* Some rooms put you back you came from
    without the opportunity for input. */
    if (AutoRoom(GetRoom()))
    {
        SetChoice("Back");
        return true;
    }
    
    std::getline(std::cin, choice);
    toupper(choice);
    SetChoice(choice);
    return (choice != "QUIT");
}

int main()
{
    std::cout << BANNER << std::endl;
    Setup();
    while (GameLoop())
    {
        sleep(0);
    }
    return Quit(EXIT_SUCCESS);
}

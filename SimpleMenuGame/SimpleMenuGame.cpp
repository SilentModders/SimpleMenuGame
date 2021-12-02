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
    #define PAUSECMD "bash -c \"read -n1 -s -p 'Press any key to quit.'\""
#endif

#define BANNER "Simple Menu Game, Version 0.3"

int Quit(int code = EXIT_FAILURE)
{
    if (code)
    {
        system(PAUSECMD);
        std::cout << std::endl;
    }
    return code;
}

bool GameLoop(Game* gameObj)
{
    /* Load up the previous choice. */
    std::string choice = gameObj->GetChoice();
    gameObj->ChooseRoom(choice);
    
    /* Don't print an empty line
    if the last input was blank. */
    if (gameObj->GetChoice() != "")
        std::cout << std::endl;
    /* Print the room's text. */
    std::cout << gameObj->RoomText(gameObj->GetRoom()) << std::endl;

    /* Some rooms put you back you came from
    without the opportunity for input. */
    if (gameObj->AutoRoom(gameObj->GetRoom()))
        return true;
    
    std::getline(std::cin, choice);
    toupper(choice);
    gameObj->SetChoice(choice);
    return (choice != "QUIT");
}

int main()
{
    /* Allow unicode chars. */
    std::locale::global(std::locale("en_US.utf8"));
#if defined _WIN32
    SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif

    Game* theGame = new Game;
    if (!theGame) /* You just lost! */
    {
        std::cout << "The game could not be started." << std::endl;
        return Quit();
    }

    std::cout << BANNER << std::endl;
    if (theGame->Setup())
    {
        while (GameLoop(theGame))
            sleep(0);
        return Quit(EXIT_SUCCESS);
    }
    std::cout << "A file load error occured." << std::endl;
    return Quit();
}

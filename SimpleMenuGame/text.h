#pragma once
#include <iostream>
#include <string>

const enum class Color
{
	COLOR_BLACK = 30,
	COLOR_DARK_RED,
	COLOR_DARK_GREEN,
	COLOR_ORANGE,
	COLOR_DARK_BLUE,
	COLOR_MAGENTA,
	COLOR_LIGHT_BLUE,
	COLOR_LIGHT_GRAY,
	COLOR_GRAY = 90,
	COLOR_RED,
	COLOR_GREEN,
	COLOR_YELLOW,
	COLOR_BLUE,
	COLOR_PINK,
	COLOR_CYAN,
	COLOR_WHITE
};

bool toupper(std::string& input);

/* Returns the string value if it is not empty, otherwise returns a default value. */
std::string LoadString(std::string input, std::string def);

/* Search "source" for any occurance of "find" and substitute "replace". */
std::string ReplaceSubstring(std::string source, std::string find, std::string replace);

/* Returns the colorized string value. */
std::string ColoredString(std::string input, Color color = Color::COLOR_LIGHT_GRAY);

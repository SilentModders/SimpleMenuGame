#pragma once

#include <iostream>
#include <string>

bool toupper(std::string& input);

/* Returns the string value if it is not empty, otherwise returns a default value. */
std::string LoadString(std::string input, std::string def);

/* Search "source" for any occurance of "find" and substitute "replace". */
std::string ReplaceSubstring(std::string source, std::string find, std::string replace);

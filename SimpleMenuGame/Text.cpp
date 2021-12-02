#include "text.h"

bool toupper(std::string& input)
{
    int ret = EXIT_SUCCESS;
    for (auto& c : input)
        if (!(c = toupper(c)))
            ret = EXIT_FAILURE;
    return ret;
}

/* Returns the string value if it is not empty, otherwise returns a default value. */
std::string LoadString(std::string input, std::string def)
{
    if (input != "")
        return input;
    return def;
}

/* Search "source" for any occurance of "find" and substitute "replace". */
std::string ReplaceSubstring(std::string source, std::string find, std::string replace)
{
    size_t position = 0;
    size_t origLength = find.length();
    position = source.find(find, position);

    while (position <= source.length())
    {
        source.erase(position, origLength);
        source.insert(position, replace);
        position = source.find(find, position);
    }
    return source;
}

std::string ColoredString(std::string input, Color color)
{
    return "\033[" + std::to_string(int(color)) + "m" + input + "\033[0m";
}

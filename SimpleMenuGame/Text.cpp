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
    int position = 0;
    int origLength = find.length();
    position = source.find(find, position);

    while (position >= 0)
    {
        source.erase(position, origLength);
        source.insert(position, replace);
        position = source.find(find, position);
    }
    return source;
}

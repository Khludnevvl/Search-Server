#include "string_processing.h"
#include "log_duration.h"
#include <string>
#include <vector>
using namespace std;

vector<string_view> SplitIntoWordsView(string_view str)
{
    std::vector<std::string_view> result;
    auto space = str.find(' ');

    while (space != str.npos)
    {

        if (str.substr(0, space) != " ")
            result.push_back(str.substr(0, space));
        str.remove_prefix(space + 1);
        space = str.find(' ');
    }
    result.push_back(str.substr(0, space));
    return result;
}

vector<string> SplitIntoWords(string_view text)
{
    vector<string> words;
    string word;
    for (const char c : text)
    {
        if (c == ' ')
        {
            if (word != "")
                words.push_back(word);
            word = "";
        }
        else
        {
            word += c;
        }
    }

    if (word != "")
        words.push_back(word);

    return words;
}

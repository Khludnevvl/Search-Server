#include "duplicates_remove.h"
#include "search_server.h"

#include <iostream>
#include <map>
#include <set>
#include <vector>

using namespace std;

void RemoveDuplicates(SearchServer &search_server)
{
    set<int> id_for_del;
    set<set<string_view, std::less<>>> all_sets;
    for (const int id : search_server)// Guarantees ascending order
    {
        const auto &current_map = search_server.GetWordFrequencies(id);
        set<string_view, less<>> tmp;
        for (const auto &[word, freq] : current_map)
        {
            tmp.insert(word);
        }
        if (all_sets.count(tmp))
        {
            id_for_del.insert(id);
        }
        else
            all_sets.insert(tmp);
    }

    for (const int id : id_for_del)
    {
        search_server.RemoveDocument(id);
        cout << "Found duplicate document id " << id << endl;
    }
}
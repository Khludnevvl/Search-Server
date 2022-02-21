#include "process_queries.h"
#include "document.h"
#include "search_server.h"

#include <algorithm>
#include <execution>
#include <string>
#include <vector>

std::vector<std::vector<Document>> ProcessQueries(const SearchServer &search_server,
                                                  const std::vector<std::string> &queries)
{

    std::vector<std::vector<Document>> result(queries.size());

    std::transform(std::execution::par, queries.begin(), queries.end(), result.begin(),
                   [&search_server](const std::string &vec) { return search_server.FindTopDocuments(vec); });

    return result;
}

std::vector<Document> ProcessQueriesJoined(const SearchServer &search_server,
                                           const std::vector<std::string> &queries)
{
    std::vector<std::vector<Document>> tmp_vec = ProcessQueries(search_server, queries);

    std::vector<Document> result;
    result.reserve(queries.size());
    for (auto it : tmp_vec)
    {
        result.insert(result.end(), it.begin(), it.end());
    }

    return result;
}
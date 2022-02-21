#include "search_server.h"

#include <iostream>
#include <string>
#include <vector>
#include <cassert>

int main() {

    std::vector<std::string> stop_words {"и", "в", "нa"};

    SearchServer search_server(stop_words);

    search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, {7, 2, 7});
    search_server.AddDocument(2, "пушистый пёс и модный ошейник"s, DocumentStatus::ACTUAL, {1, 2, 3});

    search_server.AddDocument(3, "большой кот модный ошейник "s, DocumentStatus::ACTUAL, {1, 2, 8});
    search_server.AddDocument(4, "большой пёс скворец евгений"s, DocumentStatus::ACTUAL, {1, 3, 2});
    search_server.AddDocument(5, "большой пёс скворец василий"s, DocumentStatus::ACTUAL, {1, 1, 1});

    search_server.AddDocument(6, "большой кот модный ошейник "s, DocumentStatus::BANNED, {1, 2, 8});
    search_server.AddDocument(7, "большой пёс скворец евгений"s, DocumentStatus::BANNED, {1, 2, 3});
    search_server.AddDocument(8, "большой пёс скворец василий"s, DocumentStatus::BANNED, {1, 1, 1});

    {
        auto result = search_server.FindTopDocuments("большой -кот"); // Найдет документы со словом большой, но без слова кот(минус-слово).
                                                                      // По умолчанию ищет документы только со статусом ACTUAL.

        assert(result.size() == 2); // Результат поиска состоит из 2-х документов с id = 4 и id = 5. id = 3 не подходит ,т.к есть минус-слово.
                                    // 6 и 7 ,т.к имеют статус BANNED

        assert(result[0].id == 4); // Документ 4 находится первым, т.к документы 4 и 5 одинаково релевантны, но документ 4 имеет больший рейтинг 
        assert(result[1].id == 5);
    }

    {
        auto result = search_server.FindTopDocuments("большой -кот", DocumentStatus::BANNED); //Документы только со статусом BANNED

        assert(result.size() == 2); 

        assert(result[0].id == 7); 
        assert(result[1].id == 8);
    }
}

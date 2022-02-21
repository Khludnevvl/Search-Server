#pragma once

#include <string>
#include <vector>
#include <deque>

#include "document.h"
#include "search_server.h"

class RequestQueue {
public:
    explicit RequestQueue(const SearchServer& search_server) :
        server_(search_server)
    {
    }
    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {
        std::vector<Document> tmp = server_.FindTopDocuments(raw_query, document_predicate);
        const bool is_current_empty = tmp.empty();
        if (is_current_empty) {
            count_of_empty_++;
        }

        if (requests_.size() >= sec_in_day_) {
            if (requests_.front().is_empty)
                count_of_empty_--;
            requests_.pop_front();
        }
        requests_.push_back({is_current_empty});
        return tmp;
    }

    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status);

    std::vector<Document> AddFindRequest(const std::string& raw_query);

    int GetNoResultRequests() const;


private:
    struct QueryResult {
        bool is_empty;
    };
    const SearchServer& server_;
    std::deque<QueryResult> requests_;
    const static int sec_in_day_ = 1440;
    int count_of_empty_ = 0;

};

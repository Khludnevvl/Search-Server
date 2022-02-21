#pragma once
#include <algorithm>
#include <execution>
#include <future>
#include <map>
#include <mutex>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "concurrent_containers.h"
#include "document.h"
#include "string_processing.h"

const int MAX_RESULT_DOCUMENT_COUNT = 5;
using namespace std;

class SearchServer
{
public:
    SearchServer() = default;

    explicit SearchServer(const std::string_view stop_words_text)
        : SearchServer(SplitIntoWords(stop_words_text))
    {
    }

    explicit SearchServer(const std::string &stop_words_text) : SearchServer(SplitIntoWords(stop_words_text))
    {
    }
    template <typename StringContainer>
    explicit SearchServer(const StringContainer &stop_words)
        : stop_words_(MakeUniqueNonEmptyStrings(stop_words))
    {
        if (!all_of(stop_words_.begin(), stop_words_.end(), IsValidWord))
        {
            throw std::invalid_argument("Some of stop words are invalid");
        }
    }

    void AddDocument(int document_id, const std::string_view document, DocumentStatus status,
                     const std::vector<int> &ratings);

    // main
    template <typename Execution, typename Key_mapper>
    std::vector<Document> FindTopDocuments(Execution &&exec_policy, const std::string_view raw_query,
                                           Key_mapper key) const
    {

        std::vector<Document> matched_documents;

        const Query query = ParseQuery(exec_policy, raw_query);
        matched_documents = FindAllDocuments(exec_policy, query, key);

        std::sort(exec_policy, matched_documents.begin(), matched_documents.end(),
                  [](const Document &lhs, const Document &rhs) {
                      if (std::abs(lhs.relevance - rhs.relevance) < 1e-6)
                      {
                          return lhs.rating > rhs.rating;
                      }
                      else
                      {
                          return lhs.relevance > rhs.relevance;
                      }
                  });

        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT)
        {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }
    template <typename Key_mapper>
    std::vector<Document> FindTopDocuments(const std::string_view raw_query, Key_mapper key) const
    {
        return FindTopDocuments(std::execution::seq, raw_query, key);
    }

    template <typename Execution>
    std::vector<Document> FindTopDocuments(Execution &&exec_policy, const std::string_view raw_query) const
    {
        return FindTopDocuments(exec_policy, raw_query, DocumentStatus::ACTUAL);
    }

    std::vector<Document> FindTopDocuments(const std::string_view raw_query) const
    {
        return FindTopDocuments(std::execution::seq, raw_query, DocumentStatus::ACTUAL);
    }

    template <typename Execution>
    std::vector<Document> FindTopDocuments(Execution &&exec_policy, const std::string_view raw_query,
                                           DocumentStatus raw_status) const
    {
        return FindTopDocuments(exec_policy, raw_query,
                                [raw_status](int document_id, DocumentStatus status, int rating) {
                                    return status == raw_status;
                                });
    }

    std::vector<Document> FindTopDocuments(const std::string_view raw_query, DocumentStatus raw_status) const
    {
        return FindTopDocuments(std::execution::seq, raw_query,
                                [raw_status](int document_id, DocumentStatus status, int rating) {
                                    return status == raw_status;
                                });
    }

    size_t GetDocumentCount() const;

    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(const std::string_view raw_query,
                                                                            int document_id) const
    {
        return MatchDocument(std::execution::seq, raw_query, document_id);
    }

    template <typename ExecutionPolicy>
    std::tuple<std::vector<std::string_view>, DocumentStatus>
    MatchDocument(ExecutionPolicy &exec_policy, const std::string_view raw_query, int document_id) const
    {

        const Query query = ParseQuery(exec_policy, raw_query);
        std::vector<std::string_view> matched_words;
        matched_words.reserve(query.plus_words.size());
        std::mutex mut;
        for_each(exec_policy, query.plus_words.begin(), query.plus_words.end(), [&](std::string_view word) {
            if (word_to_document_freqs_.count(word) != 0)
            {
                if (word_to_document_freqs_.at(word).count(document_id))
                {
                    std::lock_guard<std::mutex> g(mut);
                    matched_words.push_back(word);
                }
            }
        });

        for (std::string_view word : query.minus_words)
        {
            if (word_to_document_freqs_.count(word) == 0)
            {
                continue;
            }
            if (word_to_document_freqs_.at(word).count(document_id))
            {
                matched_words.clear();
                break;
            }
        }

        return {matched_words, documents_.at(document_id).status};
    }

    const auto begin() const
    {
        return doc_ids_.begin();
    }

    const auto end() const
    {
        return doc_ids_.end();
    }

    const std::map<std::string_view, double> &GetWordFrequencies(int document_id) const;

    void RemoveDocument(int document_id)
    {
        return RemoveDocument(std::execution::seq, document_id);
    }

    template <typename ExecutionPolicy>
    void RemoveDocument(ExecutionPolicy exec_policy, int document_id)
    {
        auto iter = doc_ids_.find(document_id);
        if (iter == doc_ids_.end())
            throw std::invalid_argument("Document id doesn't exist");
        doc_ids_.erase(iter);
        documents_.erase(document_id);

        std::for_each(exec_policy, documents_to_word_freqs_.at(document_id).begin(),
                      documents_to_word_freqs_.at(document_id).end(), [&](auto &word_to_freqs) {
                          word_to_document_freqs_.at(word_to_freqs.first).erase(document_id);
                      });

        documents_to_word_freqs_.erase(document_id);
    }

    const std::set<int> &GetAllDocumentsId() const;

private:
    std::set<int> doc_ids_;
    std::set<std::string> stop_words_;
    std::map<std::string_view, std::map<int, double>> word_to_document_freqs_;
    std::map<int, std::map<std::string_view, double>> documents_to_word_freqs_;
    std::set<std::string, std::less<>> all_words_;
    std::map<int, DocumentData> documents_;

    struct QueryWord
    {
        std::string_view data;
        bool is_minus;
        bool is_stop;
    };

    struct Query
    {
        std::set<std::string_view> plus_words;
        std::set<std::string_view> minus_words;
    };

    QueryWord ParseQueryWord(std::string_view word) const;

    Query ParseQuery(const std::string_view text) const
    {
        return ParseQuery(std::execution::seq, text);
    }

    template <typename ExecutionPolicy>
    Query ParseQuery(ExecutionPolicy &&exec_policy, const std::string_view text) const
    {
        if (!IsCorrectString(text))
            throw std::invalid_argument("The minus signs are entered incorrectly");

        auto words = SplitIntoWordsView(text);

        if constexpr (std::is_same_v<ExecutionPolicy, std::execution::parallel_policy>)
        {
            ConcurrentSet<std::string_view> plus_words(16);
            ConcurrentSet<std::string_view> minus_words(16);

            std::for_each(exec_policy, words.begin(), words.end(), [&](std::string_view str) {
                const QueryWord query_word = ParseQueryWord(str);
                if (!query_word.is_stop)
                {
                    if (query_word.is_minus)
                    {

                        minus_words.insert(query_word.data);
                    }
                    else
                    {

                        plus_words.insert(query_word.data);
                    }
                }
            });
            Query result;

            result.minus_words = minus_words.BuildOrdinaryset();
            result.plus_words = plus_words.BuildOrdinaryset();

            return result;
        }
        else
        {
            Query query;
            std::for_each(words.begin(), words.end(), [&](std::string_view str) {
                const QueryWord query_word = ParseQueryWord(str);

                if (!query_word.is_stop)
                {
                    if (query_word.is_minus)
                    {
                        query.minus_words.insert(query_word.data);
                    }
                    else
                    {
                        query.plus_words.insert(query_word.data);
                    }
                }
            });
            return query;
        }
    }

    bool IsStopWord(const std::string_view word) const;

    static bool IsCorrectString(const std::string_view str);

    static bool IsValidWord(const std::string_view word);

    std::vector<std::string> SplitIntoWordsNoStop(const std::string &text) const;

    static int ComputeAverageRating(const std::vector<int> &ratings);

    double ComputeWordInverseDocumentFreq(const std::string_view word) const;

    template <typename ExecutionPolicy, typename Key_mapper>
    std::vector<Document> FindAllDocuments(ExecutionPolicy &&exec_policy, const Query &query,
                                           Key_mapper key) const
    {

        ConcurrentMap<int, double> document_to_relevance(16);

        std::for_each(
            exec_policy, query.plus_words.begin(), query.plus_words.end(), [&](const std::string_view word) {
                if (word_to_document_freqs_.count(word) != 0)
                {

                    const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
                    for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word))
                    {
                        document_to_relevance[document_id].ref_to_value += term_freq * inverse_document_freq;
                    }
                }
            });

        std::for_each(exec_policy, query.minus_words.begin(), query.minus_words.end(),
                      [&](const std::string_view word) {
                          if (word_to_document_freqs_.count(word) != 0)
                          {
                              for (const auto [document_id, _] : word_to_document_freqs_.at(word))
                              {
                                  document_to_relevance.erase(document_id);
                              }
                          }
                      });

        std::vector<Document> matched_documents;

        for (const auto [document_id, relevance] : document_to_relevance.BuildOrdinaryMap())
        {
            if (key(document_id, documents_.at(document_id).status, documents_.at(document_id).rating))
            {
                matched_documents.push_back({document_id, relevance, documents_.at(document_id).rating});
            }
        }
        return matched_documents;
    }

    template <typename Key_mapper>
    std::vector<Document> FindAllDocuments(const Query &query, Key_mapper key) const
    {
        return FindAllDocuments(std::execution::seq, query, key);
    }
};

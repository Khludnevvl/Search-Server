#include "search_server.h"
#include "document.h"
#include "string_processing.h"

#include <cmath>
#include <execution>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

using namespace std;

void SearchServer::AddDocument(int document_id, const string_view document, DocumentStatus status,
                               const vector<int> &ratings)
{
    if (document_id < 0)
        throw invalid_argument("Document id must be positive"s);
    if (documents_.count(document_id))
        throw invalid_argument("Document id - "s + to_string(document_id) + " is already exists"s);
    const vector<string> words = SplitIntoWordsNoStop(string(document));
    const double inv_word_count = 1.0 / words.size();
    for (const string &word : words)
    {
        auto [word_view, is_insert] = all_words_.emplace(word);
        string_view view_on_word(*word_view);
        documents_to_word_freqs_[document_id][view_on_word] += inv_word_count;
        word_to_document_freqs_[view_on_word][document_id] += inv_word_count;
    }
    documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
    doc_ids_.insert(document_id);
}

size_t SearchServer::GetDocumentCount() const
{
    return documents_.size();
}

const map<string_view, double> &SearchServer::GetWordFrequencies(int document_id) const
{

    if (documents_to_word_freqs_.count(document_id))
    {
        return documents_to_word_freqs_.at(document_id);
    }
    else
    {
        const static map<string_view, double> empty_;
        return empty_;
    }
}

const set<int> &SearchServer::GetAllDocumentsId() const
{
    return doc_ids_;
}

SearchServer::QueryWord SearchServer::ParseQueryWord(string_view word) const
{
    bool is_minus = false;
    if (word.empty())
    {
        throw invalid_argument("Word shouldn't be empty");
    }
    if (!IsValidWord(word))
        throw invalid_argument("Text contains invalid characters");
    if (word[0] == '-')
    {
        is_minus = true;
        word = word.substr(1);
    }
    return {word, is_minus, IsStopWord(word)};
}

bool SearchServer::IsStopWord(const string_view word) const
{
    return stop_words_.count(string(word)) > 0;
}

bool SearchServer::IsCorrectString(const string_view str)
{
    for (size_t i = 0; i < str.size(); i++)
    {
        if (str[i] == '-' && i + 1 == str.size())
        {
            return false;
        }
        if (str[i] == '-' && str[i + 1] == '-')
        {
            return false;
        }
    }
    return true;
}

// A valid word must not contain special characters
bool SearchServer::IsValidWord(const string_view word)
{
    return none_of(word.begin(), word.end(), [](char c) { return c >= '\0' && c < ' '; });
}

vector<string> SearchServer::SplitIntoWordsNoStop(const string &text) const
{
    if (!IsValidWord(text))
        throw invalid_argument("Some of words are invalid"s);
    vector<string> words;
    for (const string &word : SplitIntoWords(text))
    {
        if (!IsStopWord(word))
        {
            words.push_back(word);
        }
    }
    return words;
}

int SearchServer::ComputeAverageRating(const vector<int> &ratings)
{
    int rating_sum = 0;
    for (const int rating : ratings)
    {
        rating_sum += rating;
    }
    return rating_sum / static_cast<int>(ratings.size());
}

double SearchServer::ComputeWordInverseDocumentFreq(const string_view word) const
{
    return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}

#pragma once

#include <iostream>

enum class DocumentStatus
{
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

struct Document
{
    Document() = default;
    Document(int id_, double relevance_, int rating_) 
        : id(id_), relevance(relevance_), rating(rating_) {}

    int id = 0;
    double relevance = 0;
    int rating = 0;
};
struct DocumentData
{
    int rating;
    DocumentStatus status;
};

std::ostream &operator<<(std::ostream &out, const Document &document);

void PrintDocument(const Document &document);

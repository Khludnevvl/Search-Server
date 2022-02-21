#include "document.h"

#include <iostream>

using namespace std;
ostream &operator<<(ostream &out, const Document &document)
{
    out << "{ "s
        << "document_id = "s << document.id << ", "s
        << "relevance = "s << document.relevance << ", "s
        << "rating = "s << document.rating << " }"s;
    return out;
}

void PrintDocument(const Document &document)
{
    cout << document << endl;
}
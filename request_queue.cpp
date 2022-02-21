#include "request_queue.h"
#include "document.h"

#include <string>
#include <vector>

using namespace std;

vector<Document> RequestQueue::AddFindRequest(const string &raw_query, DocumentStatus status)
{

  return AddFindRequest(raw_query, [status](int document_id, DocumentStatus document_status, int rating)
                        { return document_status == status; });
}

vector<Document> RequestQueue::AddFindRequest(const string &raw_query)
{
  return AddFindRequest(raw_query, DocumentStatus::ACTUAL);
}

int RequestQueue::GetNoResultRequests() const
{
  return count_of_empty_;
}
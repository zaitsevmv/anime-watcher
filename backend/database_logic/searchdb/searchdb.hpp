#ifndef __SEARCH_DB__
#define __SEARCH_DB__

#include <cstdint>
#include <optional>
#include <string>

class SearchDB{
public:
    std::optional<int32_t> LoadData();

    std::optional<int32_t> AddDocument();
    std::optional<int32_t> DeleteDocument();

    std::optional<std::string> ProcessSearchRequest(const std::string& search_request);
};

#endif
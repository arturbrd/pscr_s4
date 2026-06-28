#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <fmt/core.h>

#include <iostream>
#include <string>
#include <vector>

using json = nlohmann::json;

class InfluxClient {
public:
    bool write(std::string_view measurement,
               std::span<const Tag> tags,
               std::span<const Field> fields);

    QueryResult query(std::string_view sql);

private:
    HttpClient http_;
};
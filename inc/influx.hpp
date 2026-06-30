#pragma once

#include <curl/curl.hpp>

class InfluxWriter {
public:
    InfluxWriter(const std::string& host,
                 const std::string& db,
                 const std::string& token);

    ~InfluxWriter();

    bool write(std::string_view lp);

private:
    CURL* curl_ = nullptr;
    curl_slist* headers_ = nullptr;
};
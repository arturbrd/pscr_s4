#include "influx.hpp"
#include <iostream>
#include <stdexcept>

static int curl_debug_fn(
    CURL*,
    curl_infotype type,
    char* data,
    size_t size,
    void*)
{
    std::cerr.write(data, size);
    return 0;
}

InfluxWriter::InfluxWriter(const std::string& host,
                           const std::string& db,
                           const std::string& token)
{
    curl_ = curl_easy_init();
    if (!curl_)
        throw std::runtime_error("curl_easy_init failed");

    std::string url = host + "/api/v3/write_lp?db=" + db;

    headers_ = curl_slist_append(
        headers_,
        ("Authorization: Bearer " + token).c_str());

    headers_ = curl_slist_append(
        headers_,
        "Content-Type: text/plain");

    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers_);
    curl_easy_setopt(curl_, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl_, CURLOPT_DEBUGFUNCTION, curl_debug_fn);
    // opcjonalnie
    curl_easy_setopt(curl_, CURLOPT_TCP_KEEPALIVE, 1L);
}



bool InfluxWriter::write(std::string_view lp)
{
    curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, lp.data());
    curl_easy_setopt(curl_, CURLOPT_POSTFIELDSIZE, lp.size());

    CURLcode res = curl_easy_perform(curl_);

    if (res != CURLE_OK)
    {
        std::cerr << "curl: "
                  << curl_easy_strerror(res)
                  << '\n';
        return false;
    }

    long response = 0;
    curl_easy_getinfo(
        curl_,
        CURLINFO_RESPONSE_CODE,
        &response);

    if (response != 204)
    {
        std::cerr << "Influx HTTP " << response << '\n';
        return false;
    }

    return true;
}

InfluxWriter::~InfluxWriter()
{
    curl_slist_free_all(headers_);
    curl_easy_cleanup(curl_);
}
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
    std::string response_body;

    // 🔥 log wysyłanego payloadu
    // std::cerr << "[CURL] sending payload:\n"
    //           << lp << "\n";

    // 🔥 odbiór odpowiedzi (body)
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION,
        +[](char* ptr, size_t size, size_t nmemb, void* userdata) -> size_t {
            auto* out = static_cast<std::string*>(userdata);
            out->append(ptr, size * nmemb);
            return size * nmemb;
        });

    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response_body);

    // 🔥 request
    curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, lp.data());
    curl_easy_setopt(curl_, CURLOPT_POSTFIELDSIZE, lp.size());

    CURLcode res = curl_easy_perform(curl_);

    // 🔥 curl error
    if (res != CURLE_OK)
    {
        std::cerr << "[CURL ERROR] "
                  << curl_easy_strerror(res)
                  << "\n";
        return false;
    }

    // 🔥 HTTP code
    long response = 0;
    curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &response);

    std::cerr << "[CURL] HTTP response: " << response << "\n";

    // 🔥 response body (KLUCZOWE DO DEBUGA INFLUXA)
    if (!response_body.empty())
    {
        std::cerr << "[CURL] response body:\n"
                  << response_body << "\n";
    }

    // 🔥 success check
    if (response != 204)
    {
        std::cerr << "[INFLUX ERROR] write failed\n";
        return false;
    }

    return true;
}

InfluxWriter::~InfluxWriter()
{
    curl_slist_free_all(headers_);
    curl_easy_cleanup(curl_);
}
#include "TileSourceUrl.h"

#include <future>
#include <vector>
#include <cstddef>
#include <curl/curl.h>
#include <optional>

constexpr long SHUT_OFF_THE_PROGRESS_METER = 1;

static size_t curlCallback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    auto data = reinterpret_cast<std::vector<std::byte>*>(userdata);
    data->reserve(nmemb);

    data->insert(data->cend(), ptr, ptr + nmemb);

    return nmemb;
}

static std::optional<std::vector<std::byte>> requestData(const char* url)
{
    std::vector<std::byte> data;

    const auto curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, SHUT_OFF_THE_PROGRESS_METER);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, 'curl');
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 1);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 1);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, reinterpret_cast<void*>(&data));
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlCallback);
    const bool ok{curl_easy_perform(curl) == CURLE_OK};
    curl_easy_cleanup(curl);

    return data;
}

bool TileSoureUrl::request(int x, int y, int z)
{
    
}
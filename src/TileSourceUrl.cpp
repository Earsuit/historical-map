#include "TileSourceUrl.h"

#include <future>
#include <vector>
#include <cstddef>
#include <curl/curl.h>
#include <optional>

constexpr long SHUT_OFF_THE_PROGRESS_METER = 1;

namespace {
size_t curlCallback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    auto data = reinterpret_cast<std::vector<std::byte>*>(userdata);
    data->reserve(nmemb);

    data->insert(data->cend(), ptr, ptr + nmemb);

    return nmemb;
}

std::vector<std::byte> requestData(const char* url)
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
    const auto res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res == CURLE_OK) {
        return data;
    } else {
        return {};
    }
}
}

bool TileSoureUrl::request(int x, int y, int z)
{
    requests.emplace_back(std::async(std::launch::async, 
                                [x, y, z, this](){
        return Tile{x ,y, z, requestData(this->makeUrl(x, y, z))};
    }));
}
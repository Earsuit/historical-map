#include "TileSourceUrl.h"

#include <future>
#include <vector>
#include <cstddef>
#include <curl/curl.h>
#include <optional>
#include <chrono>

using namespace std::chrono_literals;

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

void TileSoureUrl::request(int x, int y, int z)
{
    requests.emplace_back(std::async(std::launch::async, 
                                [x, y, z, this](){
        return std::make_shared<Tile>(x ,y, z, requestData(this->makeUrl(x, y, z)));
    }));
}

void TileSoureUrl::waitAll()
{
    std::for_each(requests.cbegin(), requests.cend(), [](auto& fut){
        fut.wait();
    });
}

bool TileSoureUrl::isAllReady()
{
    return std::all_of(requests.cbegin(), requests.cend(), [](auto& fut){
        return fut.wait_for(0s) == std::future_status::ready;
    });
}

void TileSoureUrl::takeReady(std::vector<std::shared_ptr<Tile>>& tiles)
{
    requests.remove_if([&tiles](auto& fut){
        if (fut.wait_for(0s) == std::future_status::ready) {
            tiles.emplace_back(fut.get());
            return true;
        }

        return false;
    });
}

void TileSoureUrl::setUrl(const std::string& url)
{
    this->url = url;
}
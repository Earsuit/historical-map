#include "TileSourceUrl.h"

#include <future>
#include <vector>
#include <cstddef>
#include <curl/curl.h>
#include <optional>
#include <chrono>
#include <string_view>

namespace tile {

using namespace std::chrono_literals;

constexpr long SHUT_OFF_THE_PROGRESS_METER = 1;
constexpr std::string_view ZOOM_MATCHER = "{zoom}";
constexpr std::string_view X_MATCHER = "{x}";
constexpr std::string_view Y_MATCHER = "{y}";
constexpr auto ZOOM_MATCHER_LEN = ZOOM_MATCHER.size();
constexpr auto X_MATCHER_LEN = X_MATCHER.size();
constexpr auto Y_MATCHER_LEN = Y_MATCHER.size();

namespace {
size_t curlCallback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    const auto bytePtr = reinterpret_cast<std::byte*>(ptr);
    auto data = reinterpret_cast<std::vector<std::byte>*>(userdata);
    data->reserve(nmemb);

    data->insert(data->cend(), bytePtr, bytePtr + nmemb);

    return nmemb;
}

std::vector<std::byte> requestData(const std::string& url)
{
    std::vector<std::byte> data;

    const auto curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, SHUT_OFF_THE_PROGRESS_METER);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl");
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

TileSourceUrl::TileSourceUrl(const std::string& url)
{
    setUrl(url);
}

void TileSourceUrl::request(int x, int y, int z)
{
    requests.emplace_back(std::async(std::launch::async, 
                                [x, y, z, this](){
        return std::make_shared<Tile>(x ,y, z, requestData(this->makeUrl(x, y, z)));
    }));
}

void TileSourceUrl::waitAll()
{
    std::for_each(requests.cbegin(), requests.cend(), [](auto& fut){
        fut.wait();
    });
}

bool TileSourceUrl::isAllReady()
{
    return std::all_of(requests.cbegin(), requests.cend(), [](auto& fut){
        return fut.wait_for(0s) == std::future_status::ready;
    });
}

void TileSourceUrl::takeReady(std::vector<std::shared_ptr<Tile>>& tiles)
{
    requests.remove_if([&tiles](auto& fut){
        if (fut.wait_for(0s) == std::future_status::ready) {
            tiles.emplace_back(fut.get());
            return true;
        }

        return false;
    });
}

// tile server url format specified by https://wiki.openstreetmap.org/wiki/Slippy_map_tilenames
bool TileSourceUrl::setUrl(const std::string& url)
{
    this->url.url = url;
    this->url.zoomPos = url.find_first_of(ZOOM_MATCHER);
    this->url.xPos = url.find_first_of(X_MATCHER);
    this->url.xPos = url.find_first_of(Y_MATCHER);

    return this->url.zoomPos != std::string::npos &&
           this->url.xPos != std::string::npos &&
           this->url.yPos != std::string::npos;
}

const std::string TileSourceUrl::makeUrl(int x, int y, int z)
{
    auto realUrl = url.url;

    realUrl.replace(url.zoomPos, ZOOM_MATCHER_LEN, std::to_string(z));
    realUrl.replace(url.xPos, X_MATCHER_LEN, std::to_string(x));
    realUrl.replace(url.yPos, Y_MATCHER_LEN, std::to_string(y));
    
    return realUrl;
}

}
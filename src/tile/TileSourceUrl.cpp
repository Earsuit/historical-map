#include "TileSourceUrl.h"
#include "src/logger/Util.h"

#include <future>
#include <vector>
#include <curl/curl.h>
#include <optional>
#include <chrono>
#include <string_view>

namespace tile {

using namespace std::chrono_literals;

constexpr long SHUT_OFF_THE_PROGRESS_METER = 1;
constexpr std::string_view Z_MATCHER = "{Z}";
constexpr std::string_view X_MATCHER = "{X}";
constexpr std::string_view Y_MATCHER = "{Y}";
constexpr auto MATCHER_LEN = 3;

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
    auto logger = spdlog::get(logger::LOGGER_NAME);

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
        logger->debug("CURL get success for url {}", url);
        return data;
    } else {
        logger->error("CURL get fail for url {}, error code {}", url, static_cast<int>(res));
        return {};
    }
}
}

TileSourceUrl::TileSourceUrl(const std::string& url):
    logger{spdlog::get(logger::LOGGER_NAME)}
{
    setUrl(url);
}

std::vector<std::byte> TileSourceUrl::request(const Coordinate& coord)
{
    return requestData(makeUrl(coord));
}

// tile server url format specified by https://www.trailnotes.org/FetchMap/TileServeSource.html
bool TileSourceUrl::setUrl(const std::string& url)
{
    if (url.find(Z_MATCHER) != std::string::npos &&
        url.find(X_MATCHER) != std::string::npos &&
        url.find(Y_MATCHER) != std::string::npos) {
        logger->info("Set url {} success", url);

        this->url = url;

        return true;
    } else {
        logger->error("Set url {} fail", url);
        this->url = "";
        return false;
    }
}

const std::string TileSourceUrl::makeUrl(const Coordinate& coord)
{
    auto realUrl = url;
    std::string x{std::to_string(coord.x)};
    std::string y{std::to_string(coord.y)};
    std::string z{std::to_string(coord.z)};
    realUrl.reserve(realUrl.size() - MATCHER_LEN*3 + x.size() + y.size() + z.size());

    for (auto it = realUrl.cbegin(); it != realUrl.cend(); it++) {
        if (*it == '{') {
            switch(*(it+1)) {
                case 'X':
                    realUrl.replace(it, it + MATCHER_LEN, x);
                    break;
                case 'Y':
                    realUrl.replace(it, it + MATCHER_LEN, y);
                    break;
                case 'Z':
                    realUrl.replace(it, it + MATCHER_LEN, z);
                    break;
            }
        }
    }
    
    logger->debug("From z={}, x={}, y={} make url {}", coord.z, coord.x, coord.y ,realUrl);

    return realUrl;
}

}
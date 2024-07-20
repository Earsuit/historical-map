#include "TileSourceUrl.h"
#include "src/logger/LoggerManager.h"
#include "src/util/Error.h"

#include <future>
#include <vector>
#include <curl/curl.h>
#include <optional>
#include <chrono>
#include <string_view>
#include <string>
#include <regex>

// proxy
#ifdef __APPLE__
#include <SystemConfiguration/SystemConfiguration.h>
#include <CoreFoundation/CoreFoundation.h>
#elif defined(_WIN32)
#include <windows.h>
#include <winhttp.h>
#include <atlstr.h>
#endif

namespace tile {

using namespace std::chrono_literals;

constexpr long SHUT_OFF_THE_PROGRESS_METER = 1;
constexpr std::string_view Z_MATCHER = "{z}";
constexpr std::string_view X_MATCHER = "{x}";
constexpr std::string_view Y_MATCHER = "{y}";
constexpr auto MATCHER_LEN = 3;
constexpr auto LOGGER_NAME = "TileSourceUrl";

namespace {
constexpr int MAX_IP_TEXTUAL_REPRESENTATION = 40;   // ipv6 with a NULL terminator
constexpr auto WIN_PROXY_WITH_PROTOCOL_REGEX = "(\\w*)=(.*)";
constexpr auto NO_PROXY = "";
constexpr auto ENABLE = 1L;
constexpr auto DISABLE = 0L;
constexpr auto STOP = 1;

std::vector<std::string> getProxySettings(logger::ModuleLogger& logger) {
    std::vector<std::string> proxys;

#ifdef __APPLE__
    static char ip[MAX_IP_TEXTUAL_REPRESENTATION];
    CFDictionaryRef proxySettings = SCDynamicStoreCopyProxies(NULL);

    if (proxySettings) {
        auto assembleProxy = [proxySettings, &logger, &proxys](const std::string& name,
                                                              const void* enablekey, 
                                                              const void* ipKey, 
                                                              const void* portKey){
            if (CFNumberRef enabledPtr = reinterpret_cast<CFNumberRef>(CFDictionaryGetValue(proxySettings, enablekey)); enabledPtr) {
                bool enabled = false;
                CFNumberGetValue(enabledPtr, kCFNumberIntType, &enabled);
                if (enabled) {
                    CFNumberRef portPtr = reinterpret_cast<CFNumberRef>(CFDictionaryGetValue(proxySettings, portKey));
                    CFStringRef ipPtr = reinterpret_cast<CFStringRef>(CFDictionaryGetValue(proxySettings, ipKey));
                    if (portPtr && ipPtr) {
                        bzero(ip, MAX_IP_TEXTUAL_REPRESENTATION);
                        CFStringGetCString(ipPtr, ip, MAX_IP_TEXTUAL_REPRESENTATION, kCFStringEncodingUTF8);
                        
                        int port;
                        CFNumberGetValue(portPtr, kCFNumberIntType, &port);
                        proxys.emplace_back(name + 
                                            std::string{"://"} + 
                                            std::string{ip} + 
                                            std::string{":"} +  
                                            std::to_string(port));
                        logger.debug("{} proxy enabled: {}", name, proxys.back());
                    } 
                } else {
                    logger.debug("{} proxy not enabled.", name);
                }
            } else {
                logger.error("Failed to retrieve {} proxy settings.", name);
            }
        };

        assembleProxy("http", kSCPropNetProxiesHTTPEnable, kSCPropNetProxiesHTTPProxy, kSCPropNetProxiesHTTPPort);
        assembleProxy("https", kSCPropNetProxiesHTTPSEnable, kSCPropNetProxiesHTTPSProxy, kSCPropNetProxiesHTTPSPort);
        assembleProxy("socks5", kSCPropNetProxiesSOCKSEnable, kSCPropNetProxiesSOCKSProxy, kSCPropNetProxiesSOCKSPort);
        CFRelease(proxySettings);
    } else {
        logger.error("Failed to retrieve proxy settings.");
    }
#elif defined(_WIN32)
    WINHTTP_CURRENT_USER_IE_PROXY_CONFIG proxyConfig;

    if (WinHttpGetIEProxyConfigForCurrentUser(&proxyConfig))
    {
        if (proxyConfig.lpszProxy)
        {
            const auto systemProxy = std::string{CW2A(proxyConfig.lpszProxy)};
            static auto regex = std::regex{WIN_PROXY_WITH_PROTOCOL_REGEX};
            std::smatch bestMatch;
            if (std::regex_match(systemProxy, bestMatch, regex) && bestMatch.size() == 3) {
                proxys.emplace_back(bestMatch[1].str() + "://" + bestMatch[2].str());
            } else {
                proxys.emplace_back(systemProxy);
            }

            logger.debug("proxy enabled {}", proxys.back());
            
            GlobalFree(proxyConfig.lpszProxy);
        }
    }
    else
    {
        logger.error("Failed to get proxy settings.");
    }
#endif

    // add no proxy as the default fall back
    proxys.emplace_back(NO_PROXY);

    return proxys;
}

size_t curlCallback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    const auto bytePtr = reinterpret_cast<std::byte*>(ptr);
    auto data = reinterpret_cast<std::vector<std::byte>*>(userdata);
    data->reserve(data->size() + nmemb);

    data->insert(data->cend(), bytePtr, bytePtr + nmemb);

    return nmemb;
}

size_t progressCallback(void *clientp,
                        curl_off_t dltotal,
                        curl_off_t dlnow,
                        curl_off_t ultotal,
                        curl_off_t ulnow)
{
    auto run = reinterpret_cast<std::atomic_bool*>(clientp);
    if (!run->load()) { 
        return STOP;
    }

    return CURL_PROGRESSFUNC_CONTINUE; /* all is good */
}

util::Expected<std::vector<std::byte>> requestData(const std::string& url, 
                                                   const std::string& proxy, 
                                                   std::atomic_bool& stop)
{
    std::vector<std::byte> data;

    const auto curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, SHUT_OFF_THE_PROGRESS_METER);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/8.8.0");
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, reinterpret_cast<void*>(&data));
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlCallback);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, ENABLE);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, DISABLE); // enable progress callback getting called
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &stop);
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progressCallback);

    if (!proxy.empty()) {
        curl_easy_setopt(curl, CURLOPT_PROXY, proxy.c_str());
    }

    const auto res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res == CURLE_OK) {
        return data;
    } else {
        if (res == CURLE_ABORTED_BY_CALLBACK) {
            return util::Unexpected{util::ErrorCode::OPERATION_CANCELED, curl_easy_strerror(res)};
        }

        return util::Unexpected{util::ErrorCode::NETWORK_ERROR, curl_easy_strerror(res)};
    }
}
}

TileSourceUrl::TileSourceUrl(const std::string& url):
    logger{logger::LoggerManager::getInstance().getLogger(LOGGER_NAME)}
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
    setUrl(url);
}

std::vector<std::byte> TileSourceUrl::request(const Coordinate& coord)
{
    const auto proxys = getProxySettings(logger);
    const auto url = makeUrl(coord);

    for (const auto& proxy : proxys) {
        if (!run) {
            logger.debug("Current TileSourceUrl object is stopped, stop request.");
            return {};
        }

        logger.debug("Request {} using proxy: {}", url, proxy.empty()? "no proxy": proxy);
        if (const auto& ret = requestData(url, proxy, run); ret) {
            logger.debug("CURL get success for url {}", url);
            return ret.value();
        } else {
            if (ret.error().code == util::ErrorCode::OPERATION_CANCELED) {
                logger.debug("Request {} canceled", url);
            } else {
                logger.error("Request {} fail, error: {}", url, ret.error().msg);
            }
        }
    }

    return {};
}

// tile server url format specified by https://www.trailnotes.org/FetchMap/TileServeSource.html
bool TileSourceUrl::setUrl(const std::string& url)
{
    std::string lowerCase = url;
    std::transform(lowerCase.cbegin(), lowerCase.cend(), lowerCase.begin(), ::tolower);
    if (lowerCase.find(Z_MATCHER) != std::string::npos &&
        lowerCase.find(X_MATCHER) != std::string::npos &&
        lowerCase.find(Y_MATCHER) != std::string::npos) {
        logger.info("Set url {} success", url);

        this->url = url;

        return true;
    } else {
        logger.error("Set url {} fail", url);
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
                case 'x':
                case 'X':
                    realUrl.replace(it, it + MATCHER_LEN, x);
                    break;
                case 'y':
                case 'Y':
                    realUrl.replace(it, it + MATCHER_LEN, y);
                    break;
                case 'z':
                case 'Z':
                    realUrl.replace(it, it + MATCHER_LEN, z);
                    break;
            }
        }
    }
    
    logger.debug("From z={}, x={}, y={} make url {}", coord.z, coord.x, coord.y ,realUrl);

    return realUrl;
}

void TileSourceUrl::stop()
{
    run = false;
}

void TileSourceUrl::restart()
{
    run = true;
}
}
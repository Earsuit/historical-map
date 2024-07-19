#include "src/presentation/ImportPresenter.h"
#include "src/presentation/Util.h"
#include "src/logger/LoggerManager.h"

#include <chrono>
#include <limits>
#include <filesystem>

namespace presentation {
using namespace std::chrono_literals;

constexpr int PERIOD_INDEX = 1;
constexpr auto LOGGER_NAME = "ImportPresenter";

ImportPresenter::ImportPresenter(const std::string& source):
    logger{logger::LoggerManager::getInstance().getLogger(LOGGER_NAME)}, 
    cacheModel{model::CacheModel::getInstance()},
    source{source}
{
    cacheModel.addSource(source);
}

ImportPresenter::~ImportPresenter()
{
    cacheModel.removeSource(source);
}

void ImportPresenter::handleDoImport(const std::string& file)
{
    task = std::async(std::launch::async, [this, file]() -> util::Expected<void> {
        const auto format = std::filesystem::u8path(file).extension().string().substr(PERIOD_INDEX);
        if (auto ret = this->importModel.setFormat(format); ret) {
            auto loader = this->importModel.loadFromFile(file);
            int count = 0;

            while (loader.next()) {
                if (stopImport) {
                    return util::SUCCESS;
                }

                if (const auto& ret = loader.getValue(); ret) {
                    auto info = ret.value();
                    this->cacheModel.upsert(this->source, std::move(info));
                    count++;
                } else {
                    return util::Unexpected{ret.error()};
                }
            }
            
            if (count == 0) {
                return util::Unexpected{util::Error{util::ErrorCode::FILE_EMPTY}};
            }

            return util::SUCCESS;
        } else {
            return util::Unexpected{ret.error()};
        }
    });
}

util::Expected<bool> ImportPresenter::handleCheckImportComplete()
{
    if (task.valid() && task.wait_for(0s) == std::future_status::ready) {
        if (auto ret = task.get(); ret) {
            return true;
        } else {
            return util::Unexpected{ret.error()};
        }
    } else {
        return false;
    }
}
}
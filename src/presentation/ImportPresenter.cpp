#include "src/presentation/ImportPresenter.h"
#include "src/logger/Util.h"

#include <chrono>
#include <limits>

namespace presentation {
using namespace std::chrono_literals;

constexpr int PERIOD_INDEX = 1;

ImportPresenter::ImportPresenter(const std::string& source):
    logger{spdlog::get(logger::LOGGER_NAME)}, 
    dynamicInfoModel{model::DynamicInfoModel::getInstance()},
    source{source}
{
    dynamicInfoModel.addSource(source);
}

void ImportPresenter::handleDoImport(const std::string& file)
{
    task = std::async(std::launch::async, [this, file]() -> tl::expected<void, util::Error> {
        const auto format = std::filesystem::u8path(file).extension().string().substr(PERIOD_INDEX);
        if (auto ret = this->importModel.setFormat(format); ret) {
            auto loader = this->importModel.loadFromFile(file);
            int count = 0;
            int firstYear = std::numeric_limits<int>::max();

            while (loader.next()) {
                if (const auto& ret = loader.getValue(); ret) {
                    auto info = ret.value();
                    firstYear = std::min(info.year, firstYear);
                    this->dynamicInfoModel.upsert(this->source, std::move(info));
                    count++;
                } else {
                    return tl::unexpected{ret.error()};
                }
            }
            
            if (count == 0) {
                return tl::unexpected{util::Error{util::ErrorCode::FILE_EMPTY}};
            }

            this->dynamicInfoModel.setCurrentYear(firstYear);

            return util::SUCCESS;
        } else {
            return tl::unexpected{ret.error()};
        }
    });
}

tl::expected<bool, util::Error> ImportPresenter::handleCheckImportComplete()
{
    if (task.valid() && task.wait_for(0s) == std::future_status::ready) {
        if (auto ret = task.get(); ret) {
            return true;
        } else {
            return tl::unexpected{ret.error()};
        }
    } else {
        return false;
    }
}
}
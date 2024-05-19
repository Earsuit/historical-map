#include "src/presentation/ExportPresenter.h"
#include "src/logger/Util.h"

#include <chrono>

namespace presentation {
using namespace std::chrono_literals;

ExportPresenter::ExportPresenter(const std::string& source):
    logger{spdlog::get(logger::LOGGER_NAME)}, 
    dynamicModel{model::DynamicInfoModel::getInstance()},
    source{source}
{
}

void ExportPresenter::handleDoExport(const std::string& file)
{
    task = std::async(std::launch::async, [this, file]() -> tl::expected<void, util::Error> {
        const auto years = this->dynamicModel.getYearList(this->source);
        this->total = years.size();

        for (const auto year : years) {
            if (auto info = this->dynamicModel.getHistoricalInfo(this->source, year); info) {
                this->exportModel.insert(info->getData());
            }
            this->progress++;
        }

        return this->exportModel.writeToFile(file, true);
    });
}

tl::expected<bool, util::Error> ExportPresenter::handleCheckExportComplete()
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

float ExportPresenter::handleRequestExportProgress() const noexcept
{
    return static_cast<float>(progress) / total;
}
}
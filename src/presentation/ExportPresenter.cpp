#include "src/presentation/ExportPresenter.h"
#include "src/logger/LoggerManager.h"

#include <chrono>

namespace presentation {
using namespace std::chrono_literals;

constexpr auto LOGGER_NAME = "ExportPresenter";

ExportPresenter::ExportPresenter(const std::string& source):
    logger{logger::LoggerManager::getInstance().getLogger(LOGGER_NAME)}, 
    dynamicModel{model::CacheModel::getInstance()},
    source{source}
{
}

void ExportPresenter::handleDoExport(const std::string& file)
{
    task = std::async(std::launch::async, [this, file]() -> util::Expected<void> {
        const auto years = this->dynamicModel.getYearList(this->source);
        this->total = years.size();

        for (const auto year : years) {
            if (auto data = this->dynamicModel.getData(this->source, year); data) {
                this->exportModel.insert(*data);
            }
            this->progress++;
        }

        return this->exportModel.writeToFile(file, true);
    });
}

util::Expected<bool> ExportPresenter::handleCheckExportComplete()
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

float ExportPresenter::handleRequestExportProgress() const noexcept
{
    return static_cast<float>(progress) / total;
}
}
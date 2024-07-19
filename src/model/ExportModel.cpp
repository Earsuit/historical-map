#include "src/model/ExportModel.h"

#include <thread>
#include <chrono>

namespace model {
ExportModel::ExportModel():
    factory{persistence::ExporterImporterFactory::getInstance()}
{
}

std::vector<std::string> ExportModel::getSupportedFormat() const
{
    return factory.supportedFormat();
}

util::Expected<void> ExportModel::setFormat(const std::string& format)
{
    if (auto ret = factory.createExporter(format); ret) {
        exporter = std::move(*ret);
        return util::SUCCESS;
    } else {
        return util::Unexpected{ret.error()};
    }
}

void ExportModel::insert(const persistence::Data& info)
{
    exporter->insert(info);
}

util::Expected<void> ExportModel::writeToFile(const std::string& path, bool overwrite)
{
    return exporter->writeToFile(path, overwrite);
}
}
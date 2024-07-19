#include "src/model/ImportModel.h"

namespace model {
ImportModel::ImportModel():
    factory{persistence::ExporterImporterFactory::getInstance()}
{
}

std::vector<std::string> ImportModel::getSupportedFormat() const
{
    return factory.supportedFormat();
}

util::Expected<void> ImportModel::setFormat(const std::string& format)
{
    if (auto ret = factory.createImporter(format); ret) {
        importer = std::move(*ret);
        return util::SUCCESS;
    } else {
        return util::Unexpected{ret.error()};
    }
}

util::Generator<util::Expected<persistence::Data>> ImportModel::loadFromFile(const std::string& file)
{
    return importer->loadFromFile(file);
}
}
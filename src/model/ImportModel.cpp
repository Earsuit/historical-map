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

tl::expected<void, util::Error> ImportModel::setFormat(const std::string& format)
{
    if (auto ret = factory.createImporter(format); ret) {
        importer = std::move(*ret);
        return util::SUCCESS;
    } else {
        return tl::unexpected{ret.error()};
    }
}

util::Generator<tl::expected<persistence::Data, util::Error>> ImportModel::loadFromFile(const std::string& file)
{
    return importer->loadFromFile(file);
}
}
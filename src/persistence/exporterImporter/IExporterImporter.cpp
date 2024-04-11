#include "src/persistence/exporterImporter/IExporterImporter.h"

namespace persistence {
const Data& IImporter::front() const noexcept
{
    return *infos.begin();
}

void IImporter::pop()
{
    infos.erase(infos.begin());
}

bool IImporter::empty() const noexcept
{
    return infos.empty();
}

size_t IImporter::size() const noexcept
{
    return infos.size();
}

tl::expected<void, Error> IImporter::loadFromFile(const std::string& file)
{
    if (auto&& ret = openFile(file); ret) {
        return loadTo(std::move(ret).value(), infos);
    } else {
        return tl::unexpected{ret.error()};
    }
}
}
#include "src/persistence/exporterImporter/IExporterImporter.h"

namespace persistence {
// util::Generator<tl::expected<Data, Error>> IImporter::loadFromFile(const std::string& file)
// {
//     if (auto&& ret = openFile(file); ret) {
//         return load(std::move(ret).value());
//     } else {
//         co_yield tl::unexpected{ret.error()};
//     }
// }
}
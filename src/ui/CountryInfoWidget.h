#ifndef SRC_UI_COUNTRY_INFO_WIDGET_H
#define SRC_UI_COUNTRY_INFO_WIDGET_H

#include "src/persistence/Data.h"
#include "src/logger/Util.h"

#include "spdlog/spdlog.h"

#include <string>
#include <optional>
#include <memory>
#include <vector>

namespace ui {
class CountryInfoWidget {
public:
    CountryInfoWidget(std::list<persistence::Country>::iterator it):
        logger{spdlog::get(logger::LOGGER_NAME)},
        it{it},
        country{*it}
    {
    }

    void paint(std::optional<persistence::Coordinate>& selected);

    std::string getName();

    auto getCountryIterator() { return it; }

private:
    std::shared_ptr<spdlog::logger> logger;
    std::list<persistence::Country>::iterator it;
    persistence::Country& country;
    std::string latitude{};
    std::string longitude{};
};
}

#endif

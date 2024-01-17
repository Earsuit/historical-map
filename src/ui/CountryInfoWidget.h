#ifndef SRC_UI_COUNTRY_INFO_WIDGET_H
#define SRC_UI_COUNTRY_INFO_WIDGET_H

#include "src/persistence/Data.h"
#include "src/logger/Util.h"

#include "spdlog/spdlog.h"

#include <string>
#include <optional>
#include <memory>

namespace ui {
class CountryInfoWidget {
public:
    CountryInfoWidget(persistence::Country& country):
        logger{spdlog::get(logger::LOGGER_NAME)},
        country{country}
    {
    }

    void paint(std::optional<persistence::Coordinate>& selected);

    std::string getName();

private:
    std::shared_ptr<spdlog::logger> logger;
    persistence::Country& country;
    std::string latitude{};
    std::string longitude{};
};
}

#endif

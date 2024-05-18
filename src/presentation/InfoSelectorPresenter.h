#ifndef SRC_PRESENTATION_INFO_SELECTOR_PRESENTER_H
#define SRC_PRESENTATION_INFO_SELECTOR_PRESENTER_H

#include "src/model/DynamicInfoModel.h"
#include "src/model/DatabaseModel.h"

#include "spdlog/spdlog.h"

#include <string>
#include <memory>

namespace presentation {
class InfoSelectorPresenter {
public:
    InfoSelectorPresenter(const std::string& fromSouce, const std::string& toSource);
    ~InfoSelectorPresenter();

    void handleSelectCountry(const std::string& name);
    void handleSelectCity(const std::string& name);
    void handleSelectNote();
    void handleDeselectCountry(const std::string& name);
    void handleDeselectCity(const std::string& name);
    void handleDeselectNote();
    bool handkeCheckIsCountrySelected(const std::string& name);
    bool handleCheckIsCitySelected(const std::string& name);
    bool handleCheckIsNoteSelected();
    bool handleCheckIsAllSelected();
    void handleSelectAll();
    void handleDeselectAll();

private:
    std::shared_ptr<spdlog::logger> logger;
    model::DynamicInfoModel& dynamicModel;
    std::string fromSource;
    std::string toSource;

    std::shared_ptr<persistence::HistoricalStorage> upsertHistoricalStroageIfNotExists();
};
}

#endif

#ifndef SRC_PRESENTATION_INFO_SELECTOR_PRESENTER_H
#define SRC_PRESENTATION_INFO_SELECTOR_PRESENTER_H

#include "src/model/CacheModel.h"
#include "src/model/DatabaseModel.h"
#include "src/util/Signal.h"

#include "spdlog/spdlog.h"

#include <string>
#include <memory>
#include <future>
#include <atomic>

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

    void handleSelectAllForMultipleYears(int startYear, int endYear);
    float handleGetSelectAllForMultipleYearsProgress() const noexcept;
    bool handleCheckSelectAllForMultipleYearsComplete();
    void handleCancelSelectAllForMultipleYears() noexcept { stopTask = true; };

    util::signal::Signal<void()> setRefreshSelectAll;

private:
    std::shared_ptr<spdlog::logger> logger;
    model::DatabaseModel& databaseModel;
    model::CacheModel& cacheModel;
    std::string fromSource;
    std::string toSource;
    std::future<void> task;
    std::atomic_bool stopTask;
    int total;
    std::atomic_int progress;

    void upsertHistoricalStroageIfNotExists(int year);
    void onUpdate(const std::string& source, int year);
};
}

#endif

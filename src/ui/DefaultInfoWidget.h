#ifndef SRC_UI_DEFAULT_INFO_WIDGET_H
#define SRC_UI_DEFAULT_INFO_WIDGET_H

#include "src/presentation/HistoricalInfoPresenter.h"
#include "src/presentation/DatabaseSaverPresenter.h"
#include "src/presentation/DatabaseYearPresenter.h"
#include "src/presentation/Util.h"
#include "src/logger/Util.h"
#include "src/ui/IInfoWidget.h"

#include "spdlog/spdlog.h"

#include <string>
#include <memory>
#include <map>
#include <atomic>

namespace ui {
class DefaultInfoWidget: public IInfoWidget {
public:
    DefaultInfoWidget();
    ~DefaultInfoWidget();

    virtual bool complete() const noexcept override { return false; };
    virtual void paint() override;

    void setRefreshCountries() noexcept { countryResourceUpdated = true; }
    void setRefreshCities() noexcept { cityResourceUpdated = true; }
    void setRefreshNote() noexcept { noteResourceUpdated = true; }
    void setOnYearChange(int year) noexcept;
    void setUnsavedState(bool isUnsaved) noexcept { this->isUnsaved = isUnsaved; }

private:
    std::shared_ptr<spdlog::logger> logger;
    presentation::HistoricalInfoPresenter infoPresenter;
    presentation::DatabaseSaverPresenter databaseSaverPresenter;
    presentation::DatabaseYearPresenter yearPresenter;
    int startYear;
    int endYear;
    std::atomic_int currentYear;
    std::string countryName;
    std::atomic_bool clearNewInfoEntry = false;
    std::map<std::string, std::pair<std::string, std::string>> countryNewCoordinateCache;
    std::string newCityName, newCityLatitude, newCityLongitude;
    std::atomic_bool countryResourceUpdated = false;
    std::map<std::string, std::vector<persistence::Coordinate>> countries;
    std::atomic_bool cityResourceUpdated = false;
    std::map<std::string, persistence::Coordinate> cities;
    std::atomic_bool noteResourceUpdated = false;
    std::string note;
    std::atomic_bool isUnsaved;

    void displayCountryInfos();
    void displayCityInfos();
    void displayNote();
    void displayYearControlSection();
    void saveInfo(int year);
    void saveInfoRange(int startYear, int endYear);
    void savePopupWindow();
    void saveProgressPopUp();
    void displayCountry(const std::string& name, const std::vector<persistence::Coordinate>& contour);
    void displayCity(const std::string& name, const persistence::Coordinate& coord);
    persistence::Coordinate displayCoordinate(const std::string& uniqueId, const persistence::Coordinate& coord);

    void updateCountryResources();
    void updateCityResources();
    void updateNoteResources();
    void updateNewInfoEntry();
};

}

#endif

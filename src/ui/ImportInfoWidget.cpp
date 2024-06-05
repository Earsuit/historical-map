#include "src/ui/ImportInfoWidget.h"
#include "src/ui/Util.h"
#include "src/presentation/Util.h"
#include "src/logger/LoggerManager.h"
#include "src/util/Signal.h"

#include "imgui.h"
#include "ImFileDialog.h"

namespace ui {
const auto SELECTION = "Selected";
constexpr auto FILE_SELECT_POPUP_NAME = "Select file";
constexpr auto IMPORT_PROGRESS_POPUP_NAME = "Loading";
constexpr auto IMPORT_COMPLETE_BUTTON = "Complete";
constexpr auto IMPORT_FAIL_POPUP_NAME = "Import fail";
constexpr auto DONE_BUTTON = "Done";
constexpr auto WRITE_TO_DATABASE_PROGRESS_POPUP = "Write to database";
constexpr auto LOGGER_NAME = "ImportInfoWidget";

ImportInfoWidget::ImportInfoWidget():
    logger{logger::LoggerManager::getInstance().getLogger(LOGGER_NAME)}, 
    databaseInfoPresenter{model::PERMENANT_SOURCE},
    importInfoPresenter{presentation::IMPORT_SOURCE},
    infoSelectorPresenter{presentation::IMPORT_SOURCE, SELECTION},
    yearPresenter{presentation::IMPORT_SOURCE},
    importPresenter{presentation::IMPORT_SOURCE},
    databaseSaverPresenter{SELECTION}
{
    util::signal::connect(&databaseInfoPresenter, 
                          &presentation::HistoricalInfoPresenter::setCountriesUpdated,
                          this, 
                          &ImportInfoWidget::setRefreshDatabaseCountries);
    util::signal::connect(&databaseInfoPresenter, 
                          &presentation::HistoricalInfoPresenter::setCityUpdated,
                          this, 
                          &ImportInfoWidget::setRefreshDatabaseCities);
    util::signal::connect(&databaseInfoPresenter, 
                          &presentation::HistoricalInfoPresenter::setNoteUpdated,
                          this, 
                          &ImportInfoWidget::setRefreshDatabaseNote);
    util::signal::connect(&importInfoPresenter, 
                          &presentation::HistoricalInfoPresenter::setCountriesUpdated,
                          this, 
                          &ImportInfoWidget::setRefreshImportedCountries);
    util::signal::connect(&importInfoPresenter, 
                          &presentation::HistoricalInfoPresenter::setCityUpdated,
                          this, 
                          &ImportInfoWidget::setRefreshImportedCities);
    util::signal::connect(&importInfoPresenter, 
                          &presentation::HistoricalInfoPresenter::setNoteUpdated,
                          this, 
                          &ImportInfoWidget::setRefreshImportedNote);
    util::signal::connect(&infoSelectorPresenter, 
                          &presentation::InfoSelectorPresenter::setRefreshSelectAll,
                          this, 
                          &ImportInfoWidget::setRefreshSelectAll);
    util::signal::connect(&yearPresenter,
                          &presentation::ImportYearPresenter::onYearChange,
                          this,
                          &ImportInfoWidget::setRefreshAll);

    ifd::FileDialog::getInstance().open(FILE_SELECT_POPUP_NAME, FILE_SELECT_POPUP_NAME, fileExtensionFormat());
}

ImportInfoWidget::~ImportInfoWidget()
{
    util::signal::disconnectAll(&databaseInfoPresenter, 
                                &presentation::HistoricalInfoPresenter::setCountriesUpdated,
                                this);
    util::signal::disconnectAll(&databaseInfoPresenter, 
                                &presentation::HistoricalInfoPresenter::setCityUpdated,
                                this);
    util::signal::disconnectAll(&databaseInfoPresenter, 
                                &presentation::HistoricalInfoPresenter::setNoteUpdated,
                                this);
    util::signal::disconnectAll(&importInfoPresenter, 
                                &presentation::HistoricalInfoPresenter::setCountriesUpdated,
                                this);
    util::signal::disconnectAll(&importInfoPresenter, 
                                &presentation::HistoricalInfoPresenter::setCityUpdated,
                                this);
    util::signal::disconnectAll(&importInfoPresenter, 
                                &presentation::HistoricalInfoPresenter::setNoteUpdated,
                                this);
    util::signal::disconnectAll(&infoSelectorPresenter, 
                                &presentation::InfoSelectorPresenter::setRefreshSelectAll,
                                this);
    util::signal::disconnectAll(&yearPresenter,
                                &presentation::ImportYearPresenter::onYearChange,
                                this);
}

void ImportInfoWidget::setRefreshAll(int year) noexcept
{
    currentYear = year;
    setRefreshDatabaseCountries();
    setRefreshDatabaseCities();
    setRefreshDatabaseNote();
    setRefreshImportedCountries();
    setRefreshImportedCities();
    setRefreshImportedNote();
    setRefreshSelectAll();
}

void ImportInfoWidget::displayYearControlSection()
{
    int year = currentYear;
    if (ImGui::SliderInt("##", &year, yearPresenter.handleGetMinYear(), yearPresenter.handleGetMaxYear(), "Year %d", ImGuiSliderFlags_AlwaysClamp)) {
        yearPresenter.handleSetYear(year);
    }
    
    ImGui::SameLine();
    ImGui::PushButtonRepeat(true);
    if (ImGui::ArrowButton("##left", ImGuiDir_Left)) {
        yearPresenter.handleMoveYearBackward();
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("##right", ImGuiDir_Right)) {
        yearPresenter.handleMoveYearForward();
    }
    ImGui::PopButtonRepeat();

    ImGui::SameLine();
    helpMarker("Ctrl + click to maually set the year");
}

std::string ImportInfoWidget::fileExtensionFormat() const
{
    const auto formats = importPresenter.handleGetSupportedFormat();
    std::string extensions = "(";
    for (const auto& format: formats) {
        extensions += "*." + format + ";";
    }
    extensions.back() = ')';
    extensions += "{";
    for (const auto& format: formats) {
        extensions += "." + format + ",";
    }
    extensions.back() = '}';

    return extensions;
}

void ImportInfoWidget::doImport()
{
    if (ifd::FileDialog::getInstance().isDone(FILE_SELECT_POPUP_NAME)) {
        if (ifd::FileDialog::getInstance().hasResult()) {
            const std::string file = ifd::FileDialog::getInstance().getResult().u8string();
            logger.debug("Open file {}", file);
            importPresenter.handleDoImport(file);
            openErrorPopup = false;
            ImGui::OpenPopup(IMPORT_PROGRESS_POPUP_NAME);
        } else {
            isComplete = true;
        }
        ifd::FileDialog::getInstance().close();
    }

    if (ImGui::BeginPopupModal(IMPORT_PROGRESS_POPUP_NAME, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Imported: ");
        ImGui::SameLine();

        if (ImGui::BeginListBox("##")) {
            for (const auto year : importPresenter.handleGetImportedYears()) {
                ImGui::Text("Year %d", year);
            }
            
            ImGui::EndListBox();
        }

        if (!importComplete) {
            if (auto ret = importPresenter.handleCheckImportComplete(); ret) {
                if (importComplete = ret.value(); importComplete) {
                    yearPresenter.initYearsList();
                }
            } else {
                ImGui::CloseCurrentPopup();
                errorMsg = ret.error().msg;
                openErrorPopup = true;
            }
        }

        centeredEnableableButton(IMPORT_COMPLETE_BUTTON,
                                 this->importComplete,
                                 [](){});

        ImGui::EndPopup();

        if (openErrorPopup) {
            ImGui::OpenPopup(IMPORT_FAIL_POPUP_NAME);
        }
    }

    if (ImGui::BeginPopupModal(IMPORT_FAIL_POPUP_NAME, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        alignForWidth(ImGui::CalcTextSize(errorMsg.c_str()).x);
        ImGui::Text("%s", errorMsg.c_str());

        alignForWidth(ImGui::CalcTextSize(DONE_BUTTON).x);
        if (ImGui::Button(DONE_BUTTON)) {
            ImGui::CloseCurrentPopup();
            isComplete = true;
        }

        ImGui::EndPopup();
    }
}

void ImportInfoWidget::paint()
{
    doImport();

    if (ImGui::Begin(INFO_WIDGET_NAME, nullptr,  ImGuiWindowFlags_NoTitleBar)) {
        if (importComplete) {
            displayYearControlSection();

            updateCountryResources();
            updateCityResources();
            updateNoteResources();
            updateSelectAll();

            if (ImGui::Button("Confirm")) {
                databaseSaverPresenter.handleSaveAll();
                ImGui::OpenPopup(WRITE_TO_DATABASE_PROGRESS_POPUP);
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel")) {
                isComplete = true;
            }

            displaySaveToDatabasePopup();

            if (ImGui::Checkbox("Select all", &selectAll)) {
                if (selectAll) {
                    infoSelectorPresenter.handleSelectAll();
                } else {
                    infoSelectorPresenter.handleDeselectAll();
                }
            }

            if (ImGui::TreeNode("Imported")) {
                ImGui::SeparatorText("Countries");
                for (const auto& [country, contour] : importedCountries) {
                    selectCountry(country);
                    ImGui::SameLine();
                    displayCountry(country, contour);
                }

                ImGui::SeparatorText("Cities");
                for (const auto& [city, coord] : importedCities) {
                    selectCity(city);
                    ImGui::SameLine();
                    displayCity(city, coord);
                }

                ImGui::SeparatorText("Note");
                selectNote(importedNote);
                displayNote(importedNote);

                ImGui::TreePop();
                ImGui::Spacing();
            }

            if (ImGui::TreeNode("Database")) {
                ImGui::SeparatorText("Countries");
                for (const auto& [country, contour] : databaseCountries) {
                    displayCountry(country, contour);
                }

                ImGui::SeparatorText("Cities");
                for (const auto& [city, coord] : databaseCities) {
                    displayCity(city, coord);
                }

                ImGui::SeparatorText("Note");
                displayNote(databaseNote);

                ImGui::TreePop();
                ImGui::Spacing();
            }
        }

        ImGui::End();
    }
}

void ImportInfoWidget::selectCountry(const std::string& name)
{
    bool select = selectAll || infoSelectorPresenter.handkeCheckIsCountrySelected(name);
    if (ImGui::Checkbox(("##country" + name).c_str(), &select)) {
        if (select) {
            infoSelectorPresenter.handleSelectCountry(name);
        } else {
            infoSelectorPresenter.handleDeselectCountry(name);
        }
    }
}

void ImportInfoWidget::displayCountry(const std::string& name, const std::vector<persistence::Coordinate>& contour)
{
    if (ImGui::TreeNode((name + "##country").c_str())) {
        int idx = 0;
        for (const auto& coordinate : contour) {
            displayCoordinate(name + std::to_string(idx++), coordinate);
        }

        ImGui::TreePop();
        ImGui::Spacing();
    }
}

void ImportInfoWidget::selectCity(const std::string& name)
{
    bool select = selectAll || infoSelectorPresenter.handleCheckIsCitySelected(name);
    if (ImGui::Checkbox(("##city" + name).c_str(), &select)) {
        if (select) {
            infoSelectorPresenter.handleSelectCity(name);
        } else {
            infoSelectorPresenter.handleDeselectCity(name);
        }
    }
}

void ImportInfoWidget::displayCity(const std::string& name, const persistence::Coordinate& coord)
{
    if (ImGui::TreeNode((name + "##city").c_str())) {
        displayCoordinate(name + "city", coord);
        ImGui::TreePop();
        ImGui::Spacing();
    }
}

void ImportInfoWidget::selectNote(const std::string& note)
{
    if (note.empty()) {
        return;
    }

    bool select = selectAll || infoSelectorPresenter.handleCheckIsNoteSelected();
    if (ImGui::Checkbox("##note", &select)) {
        if (select) {
            infoSelectorPresenter.handleSelectNote();
        } else {
            infoSelectorPresenter.handleDeselectNote();
        }
    }
}

void ImportInfoWidget::displayNote(const std::string& note)
{
    if (note.empty()) {
        return;
    }

    ImGui::TextUnformatted(note.c_str(), note.c_str() + note.size());
}

void ImportInfoWidget::displayCoordinate(const std::string& uniqueId, 
                                         const persistence::Coordinate& coord)
{
    ImGui::PushItemWidth(COORDINATE_INPUT_WIDTH);

    auto latitude = coord.latitude;
    auto longitude = coord.longitude;
    ImGui::PushID(uniqueId.c_str());
    textFloatWithLabelOnLeft("latitude", latitude);
    if (ImGui::IsItemHovered()) {
        databaseInfoPresenter.setHoveredCoord(coord);
    }
    ImGui::SameLine();
    textFloatWithLabelOnLeft("longitude", longitude);
    if (ImGui::IsItemHovered()) {
        databaseInfoPresenter.setHoveredCoord(coord);
    }
    ImGui::PopID();
}

void ImportInfoWidget::displaySaveToDatabasePopup()
{
    if (ImGui::BeginPopupModal(WRITE_TO_DATABASE_PROGRESS_POPUP)) {
        simpleProgressDisplayer(databaseSaverPresenter.getProgress(),
                                DONE_BUTTON,
                                databaseSaverPresenter.isSaveComplete(),
                                [this](){
                                    this->isComplete = true;
                                });
        ImGui::EndPopup();
    }
}

void ImportInfoWidget::updateCountryResources(const presentation::HistoricalInfoPresenter& presenter,
                                              std::map<std::string, std::vector<persistence::Coordinate>>& cache)
{
    cache.clear();
    for (const auto& country : presenter.handleRequestCountryList()) {
        cache.emplace(std::make_pair(country, std::vector<persistence::Coordinate>{}));
        for (const auto& coord : presenter.handleRequestContour(country)) {
            cache[country].emplace_back(coord);
        }
    }
}

void ImportInfoWidget::updateCityResources(const presentation::HistoricalInfoPresenter& presenter, 
                                           std::map<std::string, persistence::Coordinate>& cache)
{
    cache.clear();
    for (const auto& city : presenter.handleRequestCityList()) {
        if (const auto coord = presenter.handleRequestCityCoordinate(city); coord) {
            cache.emplace(std::make_pair(city, *coord));
        }
    }
}

void ImportInfoWidget::updateNoteResources(const presentation::HistoricalInfoPresenter& presenter, std::string& cache)
{
    cache = presenter.handleGetNote();
}

void ImportInfoWidget::updateSelectAll()
{
    if (needUpdateSelectAll) {
        needUpdateSelectAll = false;
        selectAll = infoSelectorPresenter.handleCheckIsAllSelected();
    }
}

void ImportInfoWidget::updateCountryResources()
{
    if (databaseCountryResourceUpdated) {
        databaseCountryResourceUpdated = false;
        updateCountryResources(databaseInfoPresenter, databaseCountries);
    }

    if (importedCountryResourceUpdated) {
        importedCountryResourceUpdated = false;
        updateCountryResources(importInfoPresenter, importedCountries);
    }
}

void ImportInfoWidget::updateCityResources()
{
    if (databaseCityResourceUpdated) {
        databaseCityResourceUpdated = false;
        updateCityResources(databaseInfoPresenter, databaseCities);
    }

    if (importedCityResourceUpdated) {
        importedCityResourceUpdated = false;
        updateCityResources(importInfoPresenter, importedCities);
    }
}

void ImportInfoWidget::updateNoteResources()
{
    if (databaseNoteResourceUpdated) {
        databaseNoteResourceUpdated = false;
        updateNoteResources(databaseInfoPresenter, databaseNote);
    }

    if (importedNoteResourceUpdated) {
        importedNoteResourceUpdated = false;
        updateNoteResources(importInfoPresenter, importedNote);
    }
}
}
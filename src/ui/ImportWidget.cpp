#include "src/ui/ImportWidget.h"
#include "src/ui/Util.h"

#include "imgui.h"
#include "ImFileDialog.h"
#include "magic_enum/magic_enum.hpp"

#include <string>
#include <chrono>

namespace ui {
using namespace std::chrono_literals;

constexpr auto FILE_SELECT_POPUP_NAME = "Select file";
constexpr auto IMPORT_PROGRESS_POPUP_NAME = "Loading";
constexpr auto DONE_BUTTON_LABEL = "Done";
constexpr auto WRITE_TO_DATABASE_PROGRESS_POPUP = "Write to database";
constexpr auto COMPLETE = 1.0f;

bool ImportWidget::complete() const noexcept
{
    return isComplete;
}

ImportWidget::ImportWidget(int year):
    ExportImportWidget{year},
    logger{spdlog::get(logger::LOGGER_NAME)}, 
    database{persistence::DatabaseManager::getInstance()},
    currentYear{year}
{
    ifd::FileDialog::getInstance().open(FILE_SELECT_POPUP_NAME, FILE_SELECT_POPUP_NAME, fileExtensionFormat());
}

std::string ImportWidget::fileExtensionFormat() const
{
    std::string extensions = "(";
    for (const auto& format: importer.supportedFormat()) {
        extensions += "*." + format + ";";
    }
    extensions.back() = ')';
    extensions += "{";
    for (const auto& format: importer.supportedFormat()) {
        extensions += "." + format + ",";
    }
    extensions.back() = '}';

    return extensions;
}

bool ImportWidget::cacheReady() const noexcept
{
    return cache && cache->year == currentYear;
}

int ImportWidget::overwriteYear(int year)
{
    if (importComplete) {
        if (moveYearToFirst) {
            moveYearToFirst = false;
            return currentYear;
        }

        // only allow years that are imported
        std::optional<int> newYear;
        if (year > currentYear) {
            logger->debug("Next year {} is larger than current year {}, searching one larger than the current", year, currentYear);
            newYear = importer.nextYear(currentYear);
        } else if (year < currentYear) {
            logger->debug("Next year {} is less than current year {}, searching one less than the current", year, currentYear);
            newYear = importer.previousYear(currentYear - 1);
        }

        if (newYear) {
            currentYear = *newYear;
        }
    }

    return currentYear;
}

std::optional<HistoricalInfoPack> ImportWidget::getSelectableInfo() const
{
    if (importComplete) {
        return HistoricalInfoPack{importer.find(currentYear), "Import"};
    }

    return std::nullopt;
}

std::optional<HistoricalInfoPack> ImportWidget::getUnselectableInfo() const
{
    if (importComplete) {
        return HistoricalInfoPack{cache, "Database"};
    }

    return std::nullopt;
}

void ImportWidget::updateInfo()
{
    if (!cache || cache->year != currentYear) {
        logger->debug("Load data of year {} from database.", currentYear);

        cache = database.load(currentYear);
    }
}

void ImportWidget::doExportImport(const persistence::Selector& selector)
{
    doImport();

    writeToDatabase(selector);
}

void ImportWidget::buttons()
{
    if (ImGui::Button("Confirm")) {
        ImGui::OpenPopup(WRITE_TO_DATABASE_PROGRESS_POPUP);
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel")) {
        isComplete = true;
    }
}

void ImportWidget::doImport()
{
    if (ifd::FileDialog::getInstance().isDone(FILE_SELECT_POPUP_NAME)) {
        if (ifd::FileDialog::getInstance().hasResult()) {
            const auto res = ifd::FileDialog::getInstance().getResult();
            logger->debug("Open file {}", res.string());
            importTask = importer.doImport(res.string());
            ImGui::OpenPopup(IMPORT_PROGRESS_POPUP_NAME);
        } else {
            isComplete = true;
        }
        ifd::FileDialog::getInstance().close();
    }

    if (ImGui::BeginPopupModal(IMPORT_PROGRESS_POPUP_NAME, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        if (importTask.valid() && importTask.wait_for(0s) == std::future_status::ready) {
            if (auto ret = importTask.get(); ret) {
                if (const auto first = importer.firstYear(); first) {
                    currentYear = *first;
                    moveYearToFirst = true;
                }

                importProgressMessage = "Load complete, loaded " + 
                                        std::to_string(importer.numOfYearsImported()) + 
                                        " years";

                importComplete = true;
            } else {
                importProgressMessage = std::string(magic_enum::enum_name(ret.error().code)) + 
                                        " " + 
                                        ret.error().msg;

                importFail = true;
            }
        } else if (!importFail) {
            importProgressMessage = "Loaded " + std::to_string(importer.numOfYearsImported()) + " years";
            
        }

        alignForWidth(ImGui::CalcTextSize(importProgressMessage.c_str()).x);
        ImGui::Text("%s", importProgressMessage.c_str());

        centeredEnableableButton(DONE_BUTTON_LABEL,
                                 [this](){
                                    return !this->importTask.valid();
                                 },
                                 [this](){
                                    if (this->importFail) {
                                        this->isComplete = true;
                                    }
                                 });

        ImGui::EndPopup();
    }
}

void ImportWidget::writeToDatabase(const persistence::Selector& selector)
{
    if (ImGui::BeginPopupModal(WRITE_TO_DATABASE_PROGRESS_POPUP, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        if (!writeToDatabaseDone && !writeToDatabaseTask.valid()) {
            writeToDatabaseTask = std::async(std::launch::async, [this, &selector](){
                auto selections = selector.getSelections();
                const float quantity = selector.getQuantity();

                while (selections.next()) {
                    this->database.update(selections.getValue());
                }

                while (database.getWorkLoad() != 0) {
                    this->progress = (quantity - database.getWorkLoad()) / quantity;
                }

                this->progress = COMPLETE;
                this->writeToDatabaseDone = true;
            });
        }

        if (writeToDatabaseTask.valid() && writeToDatabaseTask.wait_for(0s) == std::future_status::ready) {
            writeToDatabaseTask.get();
        }

        simpleProgressDisplayer(progress, 
                                DONE_BUTTON_LABEL,
                                [this]() -> bool {
                                    return !this->writeToDatabaseTask.valid();
                                },
                                [this](){
                                    this->isComplete = true;;
                                });

        ImGui::EndPopup();
    }
}
}
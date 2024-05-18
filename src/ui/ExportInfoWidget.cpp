#include "src/ui/ExportInfoWidget.h"
#include "src/ui/Util.h"
#include "src/logger/Util.h"
#include "src/presentation/Util.h"

#include "external/imgui/imgui.h"

namespace ui {
const auto TO_SOURCE = "Export";
constexpr auto DECIMAL_PRECISION = "%.2f";

ExportInfoWidget::ExportInfoWidget():
    logger{spdlog::get(logger::LOGGER_NAME)}, 
    infoPresenter{presentation::DEFAULT_HISTORICAL_INFO_SOURCE},
    infoSelectorPresenter{presentation::DEFAULT_HISTORICAL_INFO_SOURCE, TO_SOURCE}
{   
}

void ExportInfoWidget::displayYearControlSection()
{
    currentYear = yearPresenter.handelGetYear();

    if (ImGui::SliderInt("##", &currentYear, yearPresenter.handleGetMinYear(), yearPresenter.handleGetMaxYear(), "Year %d", ImGuiSliderFlags_AlwaysClamp)) {
        yearPresenter.handleSetYear(currentYear);
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

    currentYear = yearPresenter.handelGetYear();
}

void ExportInfoWidget::displayCoordinate(const std::string& uniqueId, const persistence::Coordinate& coord)
{
    ImGui::PushItemWidth(COORDINATE_INPUT_WIDTH);

    auto latitude = coord.latitude;
    auto longitude = coord.longitude;
    ImGui::PushID(uniqueId.c_str());
    ImGui::LabelText("latitude", DECIMAL_PRECISION, latitude);
    if (ImGui::IsItemHovered()) {
        infoPresenter.setHoveredCoord(coord);
    }
    ImGui::SameLine();
    ImGui::LabelText("longitude", DECIMAL_PRECISION, longitude);
    if (ImGui::IsItemHovered()) {
        infoPresenter.setHoveredCoord(coord);
    }
    ImGui::PopID();
}

void ExportInfoWidget::displayCountry(const std::string& name)
{
    bool select = selectAll || infoSelectorPresenter.handkeCheckIsCountrySelected(name);
    if (ImGui::Checkbox(("##country" + name).c_str(), &select)) {
        if (select) {
            infoSelectorPresenter.handleSelectCountry(name);
        } else {
            infoSelectorPresenter.handleDeselectCountry(name);
        }
    }
    ImGui::SameLine();

    if (ImGui::TreeNode((name + "##country").c_str())) {
        int idx = 0;
        for (auto& coordinate : infoPresenter.handleRequestContour(name)) {
            displayCoordinate(name + std::to_string(idx++), coordinate);
        }

        ImGui::TreePop();
        ImGui::Spacing();
    }
}

void ExportInfoWidget::displayCity(const std::string& name)
{
    bool select = selectAll || infoSelectorPresenter.handleCheckIsCitySelected(name);
    if (ImGui::Checkbox(("##city" + name).c_str(), &select)) {
        if (select) {
            infoSelectorPresenter.handleSelectCity(name);
        } else {
            infoSelectorPresenter.handleDeselectCity(name);
        }
    }
    ImGui::SameLine();

    if (const auto ret = infoPresenter.handleRequestCityCoordinate(name); ret) {
        if (ImGui::TreeNode((name + "##city").c_str())) {
            displayCoordinate(name + "city", *ret);
            ImGui::TreePop();
            ImGui::Spacing();
        }
    }
}

void ExportInfoWidget::displayNote()
{
    if (auto note = infoPresenter.handleGetNote(); note && !note->empty()) {
        bool select = selectAll || infoSelectorPresenter.handleCheckIsNoteSelected();
        if (ImGui::Checkbox("##note", &select)) {
            if (select) {
                infoSelectorPresenter.handleSelectNote();
            } else {
                infoSelectorPresenter.handleDeselectNote();
            }
        }
        ImGui::SameLine();    
        ImGui::SeparatorText("Note");
        ImGui::TextUnformatted(note->c_str(), note->c_str() + note->size());
    }
}

void ExportInfoWidget::paint()
{
    if (ImGui::Begin(INFO_WIDGET_NAME, nullptr,  ImGuiWindowFlags_NoTitleBar)) {
        displayYearControlSection();
        infoPresenter.clearHoveredCoord();

        if (ImGui::Button("Export as")) {
            
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            isComplete = true;
        }

        selectAll = infoSelectorPresenter.handleCheckIsAllSelected();
        if (ImGui::Checkbox("Select all", &selectAll)) {
            if (selectAll) {
                infoSelectorPresenter.handleSelectAll();
            } else {
                infoSelectorPresenter.handleDeselectAll();
            }
        }

        ImGui::SeparatorText("Countries");
        for (const auto& country : infoPresenter.handleRequestCountryList()) {
            displayCountry(country);
        }

        ImGui::SeparatorText("Cities");
        for (const auto& city : infoPresenter.handleRequestCityList()) {
            displayCity(city);
        }

        displayNote();

        ImGui::End();
    }
}


}
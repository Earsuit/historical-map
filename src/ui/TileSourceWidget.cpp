#include "src/ui/TileSourceWidget.h"
#include "src/presentation/TileSourceUrlPresenter.h"
#include "src/logger/LoggerManager.h"

#include "external/imgui/imgui.h"
#include "external/imgui/misc/cpp/imgui_stdlib.h"

namespace ui {
constexpr auto TRANSPARENT = IM_COL32(0, 0, 0, 0);
const auto TILE_SERVER_LOOKUP = "For different tile server url, please check https://www.trailnotes.org/FetchMap/TileServeSource.html";
constexpr auto LOGGER_NAME = "TileSourceWidget";

TileSourceWidget::TileSourceWidget():
    logger{logger::LoggerManager::getInstance().getLogger(LOGGER_NAME)},
    showConfigWidget{[this](){this->showTileSourceUrlConfig();}}
{
    widgetPresenter.handleSetTileEngine(widgetPresenter.handleGetTileEngineList().front());

    for (const auto& source : widgetPresenter.handleGetTileSourceList()) {
        sourceList += source + "\0";
    }

    engineList = widgetPresenter.handleGetTileEngineList();
    for (const auto& engine : engineList) {
        engineListString += engine + "\0";
    }
}

void TileSourceWidget::paint()
{
    ImGui::Begin(TILE_SOURCE_WIDGET_NAME);

    ImGui::Combo("Source", &sourceIdx, sourceList.c_str());
    
    if (ImGui::Combo("Tile Data Processor", &tileEngineIdx, engineListString.c_str())) {
        if (auto ret = widgetPresenter.handleSetTileEngine(engineList[tileEngineIdx]); !ret) {
            logger.error("Failed to set tile engine, error {}", ret.error().msg);
        }
    }

    ImGui::SeparatorText("Configuration");

    showConfigWidget[sourceIdx]();

    ImGui::End();
}

void TileSourceWidget::showTileSourceUrlConfig()
{
    static presentation::TileSourceUrlPresenter presenter{};
    auto url = presenter.handleGetUrl();
    if (ImGui::InputText("##url", &url)) {
        presenter.handleSetUrl(url);
    }
    ImGui::SameLine();
    if (ImGui::Button("Set")) {
        presenter.handleSetTileSource();
    }
    ImGui::PushStyleColor(ImGuiCol_FrameBg, TRANSPARENT);  // Transparent background
    ImGui::InputText("##text", (char*)TILE_SERVER_LOOKUP, strlen(TILE_SERVER_LOOKUP) + 1, ImGuiInputTextFlags_ReadOnly);
    ImGui::PopStyleColor(1);
}
}

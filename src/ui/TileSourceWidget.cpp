#include "src/ui/TileSourceWidget.h"
#include "src/ui/TileSourceUrlWidget.h"
#include "src/presentation/TileSourceUrlPresenter.h"
#include "src/logger/LoggerManager.h"

#include "external/imgui/imgui.h"

#include <libintl.h>

namespace ui {
constexpr auto LOGGER_NAME = "TileSourceWidget";

TileSourceWidget::TileSourceWidget():
    logger{logger::LoggerManager::getInstance().getLogger(LOGGER_NAME)},
    getDetailWidget{[](){ return std::make_unique<TileSourceUrlWidget>(); }}
{
    for (const auto& source : widgetPresenter.handleGetTileSourceList()) {
        sourceList += source + "\0";
    }

    engineList = widgetPresenter.handleGetTileEngineList();
    for (const auto& engine : engineList) {
        engineListString += engine + "\0";
    }

    widgetPresenter.handleSetTileEngine(engineList.front());
    detail = getDetailWidget.front()();
}

void TileSourceWidget::paint()
{
    ImGui::Begin(gettext(TILE_SOURCE_WIDGET_NAME));

    if (ImGui::Combo(gettext("Source"), &sourceIdx, sourceList.c_str())) {
        detail = getDetailWidget[sourceIdx]();
    }
    
    if (ImGui::Combo(gettext("Tile Data Processor"), &tileEngineIdx, engineListString.c_str())) {
        if (auto ret = widgetPresenter.handleSetTileEngine(engineList[tileEngineIdx]); !ret) {
            logger.error("{} {}", gettext("Failed to set tile engine, error"), ret.error().msg);
        }
    }

    ImGui::SeparatorText(gettext("Configuration"));

    detail->paint();

    ImGui::End();
}
}

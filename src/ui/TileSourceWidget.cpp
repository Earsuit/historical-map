#include "src/ui/TileSourceWidget.h"
#include "src/tile/TileSourceUrl.h"
#include "src/tile/RasterTileEngine.h"
#include "src/tile/TileEngineFactory.h"
#include "src/logger/Util.h"

#include "external/imgui/imgui.h"
#include "external/imgui/misc/cpp/imgui_stdlib.h"

namespace ui {

constexpr auto DEFAULT_URL = "https://a.tile.openstreetmap.org/{Z}/{X}/{Y}.png";
constexpr auto SOURCE_TYPE = "URL\0";
constexpr const char* TILE_ENGINES[] = {tile::RASTER_TILE_ENGINE_NAME};
constexpr auto TRANSPARENT = IM_COL32(0, 0, 0, 0);
constexpr auto DEFAULT_TILE_ENGINE = TILE_ENGINES[0];

TileSourceWidget::TileSourceWidget() :
    logger{spdlog::get(logger::LOGGER_NAME)},
    tileSource{std::make_shared<tile::TileSourceUrl>(DEFAULT_URL)},
    tileEngine{tile::TileEngineFactory::createInstance(DEFAULT_TILE_ENGINE)},
    showConfigWidget{[this](){this->showTileSourceUrlConfig();}}
{ 
}

void TileSourceWidget::paint()
{
    ImGui::Begin(TILE_SOURCE_WIDGET_NAME);

    ImGui::Combo("Source", &sourceIdx, SOURCE_TYPE);
    
    if (ImGui::Combo("Tile Data Processor", &tileEngineIdx, TILE_ENGINES, sizeof(TILE_ENGINES)/sizeof(TILE_ENGINES[0]))) {
        tileEngine = tile::TileEngineFactory::createInstance(TILE_ENGINES[tileEngineIdx]);
    }

    ImGui::SeparatorText("Configuration");

    showConfigWidget[sourceIdx]();

    ImGui::End();
}

void TileSourceWidget::showTileSourceUrlConfig()
{
    static std::string url = DEFAULT_URL;
    const auto text = "For different tile server url, please check https://www.trailnotes.org/FetchMap/TileServeSource.html";
    ImGui::InputText("##url", &url);
    ImGui::SameLine();
    if (ImGui::Button("Set")) {
        if (auto tileSourceUrl = std::dynamic_pointer_cast<tile::TileSourceUrl>(tileSource); tileSourceUrl) {
            tileSourceUrl->setUrl(url);
        } else {
            tileSource = std::make_shared<tile::TileSourceUrl>(url);
        }
    }
    ImGui::PushStyleColor(ImGuiCol_FrameBg, TRANSPARENT);  // Transparent background
    ImGui::InputText("##text", (char*)text, strlen(text) + 1, ImGuiInputTextFlags_ReadOnly);
    ImGui::PopStyleColor(1);
}

std::shared_ptr<tile::TileSource> TileSourceWidget::getTileSource()
{
    return tileSource;
}

std::shared_ptr<tile::TileEngine> TileSourceWidget::getTileEngine()
{
    return tileEngine;
}

}

#include "src/ui/TileSourceWidget.h"
#include "src/tile/TileSourceUrl.h"
#include "src/ui/Util.h"

#include "external/imgui/imgui.h"
#include "external/imgui/misc/cpp/imgui_stdlib.h"

namespace ui {

constexpr auto DEFAULT_URL = "https://a.tile.openstreetmap.org/{Z}/{X}/{Y}.png";
constexpr auto CONFIG_TYPE = "URL\0";

TileSourceWidget::TileSourceWidget() :
    tileSource{std::make_shared<tile::TileSourceUrl>(DEFAULT_URL)},
    showConfigWidget{[this](){this->showTileSourceUrlConfig();}}
{
}

void TileSourceWidget::paint()
{
    ImGui::Begin("##Tile source widget");

    ImGui::Combo("Source", &sourceIdx, CONFIG_TYPE);

    ImGui::SeparatorText("Configuration");

    showConfigWidget[sourceIdx]();

    ImGui::End();
}

void TileSourceWidget::showTileSourceUrlConfig()
{
    static std::string url = DEFAULT_URL;
    ImGui::InputText("##url", &url);
    ImGui::SameLine();
    if (ImGui::Button("Set")) {
        if (auto tileSourceUrl = std::dynamic_pointer_cast<tile::TileSourceUrl>(tileSource); tileSourceUrl) {
            tileSourceUrl->setUrl(url);
        } else {
            tileSource = std::make_shared<tile::TileSourceUrl>(url);
        }
    }
    ImGui::SameLine(); 
    HelpMarker("For different tile server url, please check https://www.trailnotes.org/FetchMap/TileServeSource.html");
}

std::shared_ptr<tile::TileSource> TileSourceWidget::getTileSource()
{
    return tileSource;
}

}

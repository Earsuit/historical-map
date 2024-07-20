#include "src/ui/TileSourceUrlWidget.h"

#include "external/imgui/imgui.h"
#include "external/imgui/misc/cpp/imgui_stdlib.h"

#include <libintl.h>

#ifdef _WIN32
    #undef  TRANSPARENT // there is a marco defined somewhere
#endif

namespace ui {
#define __(x) x     // gettext translation registration for constexpr

constexpr auto TRANSPARENT = IM_COL32(0, 0, 0, 0);
const auto TILE_SERVER_LOOKUP = __("For different tile server url, please check https://www.trailnotes.org/FetchMap/TileServeSource.html");

TileSourceUrlWidget::TileSourceUrlWidget()
{
    url = presenter.handleGetDefaultUrl();
}

void TileSourceUrlWidget::paint()
{
    ImGui::InputText("##url", &url);
    ImGui::SameLine();
    if (ImGui::Button(gettext("Set"))) {
        presenter.handleSetUrl(url);
    }
    ImGui::SameLine();
    if (ImGui::Button(gettext("reset"))) {
        url = presenter.handleGetDefaultUrl();
        presenter.handleSetUrl(url);
    }
    ImGui::PushStyleColor(ImGuiCol_FrameBg, TRANSPARENT);  // Transparent background
    ImGui::InputText("##text", gettext(TILE_SERVER_LOOKUP), strlen(gettext(TILE_SERVER_LOOKUP)) + 1, ImGuiInputTextFlags_ReadOnly);
    ImGui::PopStyleColor(1);
}
}
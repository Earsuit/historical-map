#include "src/ui/Util.h"

#include "external/imgui/imgui.h"

namespace ui {
void helpMarker(const char* message)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip())
    {
        ImGui::TextUnformatted(message);
        ImGui::EndTooltip();
    }
}

void alignForWidth(float width, float alignment)
{
    ImGuiStyle& style = ImGui::GetStyle();
    float avail = ImGui::GetContentRegionAvail().x;
    float offset = (avail - width) * alignment;
    if (offset > 0.0f) {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);
    }
}
}
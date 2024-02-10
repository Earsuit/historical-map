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
}
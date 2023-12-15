#include "src/ui/util.h"

namespace ui {
void HelpMarker(const std::string_view desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc.data());
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

}
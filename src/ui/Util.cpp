#include "src/ui/Util.h"

#include "external/imgui/imgui.h"

namespace ui {
constexpr float STEP = 0;
constexpr float STEP_FAST = 0;
constexpr auto DECIMAL_PRECISION = "%.2f";

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

void textFloatWithLabelOnLeft(const std::string& label, float value)
{
    ImGui::Text("%s %.2f", label.c_str(), value);
}

void inputFloatWithLabelOnLeft(const std::string& label, float& value)
{
    ImGui::Text("%s", label.c_str());
    ImGui::SameLine();
    ImGui::InputFloat(("##" + label).c_str(), &value, STEP, STEP_FAST, DECIMAL_PRECISION);
}
}
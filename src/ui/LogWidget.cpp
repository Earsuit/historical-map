#include "src/ui/LogWidget.h"

#include "external/imgui/imgui.h"

namespace ui {

constexpr ImVec2 WINDOW_SIZE{800, 200};
constexpr ImVec2 INHERIT_PARENT_SIZE{0,0};
constexpr ImVec2 IMEM_SPACING{0,0};
constexpr float Y_BOTTOM = 1.0;

void LogWidget::paint()
{
    ImGui::SetNextWindowSize(WINDOW_SIZE, ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Log")) {

        bool clear = ImGui::Button("Clear");
        ImGui::SameLine();
        bool copy = ImGui::Button("Copy");
        ImGui::Separator();

        if (ImGui::BeginChild("scrolling", INHERIT_PARENT_SIZE, ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar)) {
            if (copy) {
                ImGui::LogToClipboard();
            } 

            if (clear) {
                sink->dumpLogs();
                std::vector<std::string>().swap(logs);
            } else if (const auto newLogs = sink->dumpLogs(); !newLogs.empty()){
                logs.reserve(logs.size() + newLogs.size());
                logs.insert(logs.end(), newLogs.cbegin(), newLogs.cend());
            }
            
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, IMEM_SPACING);
            std::for_each(logs.cbegin(), logs.cend(), [](const auto& message){
                ImGui::TextUnformatted(message.c_str());
            });
            ImGui::PopStyleVar();

            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
                ImGui::SetScrollHereY(Y_BOTTOM);
            }
        }

        ImGui::EndChild();
        ImGui::End();
    }
}

}
#include "src/ui/LogWidget.h"

#include "external/imgui/imgui.h"

namespace ui {

constexpr ImVec2 WINDOW_SIZE{800, 200};
constexpr ImVec2 INHERIT_PARENT_SIZE{0,0};
constexpr ImVec2 IMEM_SPACING{0,0};
constexpr float Y_BOTTOM = 1.0;
constexpr const char* LOG_LEVEL[] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "CRITICAL", "OFF"};

LogWidget::LogWidget(spdlog::logger& logger, std::shared_ptr<logger::StringSink> sink) :
    sink{sink},
    logger{logger} 
{
    logger.set_level(spdlog::level::level_enum{logLevel});
}

void LogWidget::paint()
{
    ImGui::SetNextWindowSize(WINDOW_SIZE, ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Log")) {

        bool clear = ImGui::Button("Clear");
        ImGui::SameLine();
        bool copy = ImGui::Button("Copy");
        ImGui::SameLine();
        if (ImGui::Combo("##Level", &logLevel, LOG_LEVEL, IM_ARRAYSIZE(LOG_LEVEL))) {
            logger.set_level(spdlog::level::level_enum{logLevel});
        }

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
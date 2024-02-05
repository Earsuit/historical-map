#include "src/ui/LogWidget.h"
#include "src/logger/Util.h"

#include "external/imgui/imgui.h"
#include "external/imgui/misc/cpp/imgui_stdlib.h"

#include <regex>
#include <iostream>
namespace ui {

constexpr ImVec2 WINDOW_SIZE{800, 200};
constexpr ImVec2 INHERIT_PARENT_SIZE{0,0};
constexpr ImVec2 ITEM_SPACING{0,0};
constexpr float Y_BOTTOM = 1.0;
constexpr const char* LOG_LEVEL[] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "CRITICAL", "OFF"};
constexpr int LEVEL_COMBO_WIDTH = 100;
constexpr auto TRANSPARENT = IM_COL32(0, 0, 0, 0);

LogWidget::LogWidget() :
    sink{std::make_shared<logger::StringSink>()},
    logger{std::make_shared<spdlog::logger>(logger::LOGGER_NAME, sink)}
{
    spdlog::initialize_logger(logger);
    logger->set_pattern("[%D %T %z] [%l] %v");
    logger->set_level(spdlog::level::level_enum{logLevel});
}

void LogWidget::paint()
{
    ImGui::SetNextWindowSize(WINDOW_SIZE, ImGuiCond_FirstUseEver);
    if (ImGui::Begin(LOG_WIDGET_NAME)) {
        bool clear = ImGui::Button("Clear");
        ImGui::SameLine();
        bool copy = ImGui::Button("Copy");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(LEVEL_COMBO_WIDTH);
        if (ImGui::Combo("##Level", &logLevel, LOG_LEVEL, IM_ARRAYSIZE(LOG_LEVEL))) {
            logger->set_level(spdlog::level::level_enum{logLevel});
        }
        ImGui::SameLine();

        if (filterEnable) {
            ImGui::BeginDisabled();
        }
        ImGui::InputText("Filter", &filter);
        if (filterEnable) {
            ImGui::EndDisabled();
        }

        ImGui::SameLine();
        ImGui::Checkbox("Set", &filterEnable);

        ImGui::Separator();

        if (ImGui::BeginChild("scrolling", INHERIT_PARENT_SIZE, ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar)) {
            if (copy) {
                ImGui::LogToClipboard();
            } 

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ITEM_SPACING);

            if (clear) {
                logger->flush();
                start = end = 0;
            } else {
                updateLogs();
                
                ImGui::PushStyleColor(ImGuiCol_FrameBg, TRANSPARENT);  // Transparent background
                for (auto i = start; i != end; i++) {
                    ImGui::InputText(("##" + std::to_string(i)).c_str(), &logs[i], ImGuiInputTextFlags_ReadOnly);
                }
                ImGui::PopStyleColor(1);
            }

            ImGui::PopStyleVar();

            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
                ImGui::SetScrollHereY(Y_BOTTOM);
            }
        }

        ImGui::EndChild();
    }

    ImGui::End();
}

void LogWidget::updateLogs()
{
    for (auto&& newLog : sink->dumpLogs()) {
        if (filterEnable) {
            try {
                if (std::regex_search(newLog, std::regex{filter})) {
                    logs[end++] = newLog;
                }
            }
            catch (const std::regex_error& e) {
                filterEnable = false;
                logs[end++] = "Invalid regex string: " + std::string(e.what());
            }
        } else {
            logs[end++] = newLog;
        }

        if (end == start) {
            start++;
        }
    }
}

}
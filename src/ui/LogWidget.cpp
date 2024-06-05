#include "src/ui/LogWidget.h"
#include "src/logger/GuiSink.h"

#include "external/imgui/imgui.h"
#include "external/imgui/misc/cpp/imgui_stdlib.h"

#include <regex>

namespace ui {

constexpr ImVec2 WINDOW_SIZE{800, 200};
constexpr ImVec2 INHERIT_PARENT_SIZE{0,0};
constexpr ImVec2 ITEM_SPACING{0,0};
constexpr float Y_BOTTOM = 1.0;
constexpr const char* LOG_LEVEL[] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "CRITICAL", "OFF"};
constexpr int LEVEL_COMBO_WIDTH = 100;
constexpr auto TRANSPARENT = IM_COL32(0, 0, 0, 0);
constexpr auto ERROR_COLLOR = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);

LogWidget::LogWidget():
    loggerManager{logger::LoggerManager::getInstance({std::make_shared<logger::GuiSink>(*this)})}
{
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
            loggerManager.setLevel(spdlog::level::level_enum{logLevel});
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
                start = end = 0;
            } else {
                updateLogs();
                
                ImGui::PushStyleColor(ImGuiCol_FrameBg, TRANSPARENT);  // Transparent background
                ImGui::PushItemWidth(-FLT_MIN);
                for (auto i = start; i != end; i++) {
                    if (logs[i].color) {
                        ImGui::PushStyleColor(ImGuiCol_Text, logs[i].color.value());
                    }
                    
                    ImGui::InputText(("##" + std::to_string(i)).c_str(), &logs[i].msg, ImGuiInputTextFlags_ReadOnly);

                    if (logs[i].color) {
                        ImGui::PopStyleColor();
                    }
                }
                ImGui::PopItemWidth();
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
    Log log;

    while (queue.try_dequeue(log)) {
        if (filterEnable) {
            try {
                if (std::regex_search(log.msg, std::regex{filter})) {
                    logs[end++] = std::move(log);
                }
            }
            catch (const std::regex_error& e) {
                filterEnable = false;
                logs[end++] = Log{"Invalid regex string: " + std::string(e.what()), ERROR_COLLOR};
            }
        } else {
            logs[end++] = std::move(log);
        }

        if (end == start) {
            start++;
        }
    }
    
}

void LogWidget::log(const std::string& log, spdlog::level::level_enum lvl)
{
    std::optional<ImVec4> color;

    if (lvl >= spdlog::level::level_enum::err) {
        color = ERROR_COLLOR;
    }

    queue.enqueue({log, color});
}

}
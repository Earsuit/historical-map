#include "src/ui/LogWidget.h"
#include "src/logger/Util.h"

#include "external/imgui/imgui.h"

namespace ui {

constexpr ImVec2 WINDOW_SIZE{800, 200};
constexpr ImVec2 INHERIT_PARENT_SIZE{0,0};
constexpr ImVec2 IMEM_SPACING{0,0};
constexpr float Y_BOTTOM = 1.0;
constexpr const char* LOG_LEVEL[] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "CRITICAL", "OFF"};
constexpr int LEVEL_COMBO_WIDTH = 100;

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
    if (ImGui::Begin("Log")) {

        bool clear = ImGui::Button("Clear");
        ImGui::SameLine();
        bool copy = ImGui::Button("Copy");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(LEVEL_COMBO_WIDTH);
        if (ImGui::Combo("##Level", &logLevel, LOG_LEVEL, IM_ARRAYSIZE(LOG_LEVEL))) {
            logger->set_level(spdlog::level::level_enum{logLevel});
        }

        ImGui::Separator();

        if (ImGui::BeginChild("scrolling", INHERIT_PARENT_SIZE, ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar)) {
            if (copy) {
                ImGui::LogToClipboard();
            } 

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, IMEM_SPACING);

            if (clear) {
                logger->flush();
                start = end = 0;
            } else {
                updateLogs();

                for (auto i = start; i != end; i++) {
                    ImGui::TextUnformatted(logs[i].c_str());
                }
            }

            ImGui::PopStyleVar();

            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
                ImGui::SetScrollHereY(Y_BOTTOM);
            }
        }

        ImGui::EndChild();
        ImGui::End();
    }
}

void LogWidget::updateLogs()
{
    const auto& newLogs = sink->dumpLogs();
    for (auto& newLog : newLogs) {
        logs[end++] = newLog;

        if (end == start) {
            start++;
        }
    }
}

}
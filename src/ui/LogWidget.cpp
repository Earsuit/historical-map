#include "src/ui/LogWidget.h"

#include "external/imgui/imgui.h"
#include "external/imgui/misc/cpp/imgui_stdlib.h"

#include <regex>
#include <libintl.h>

#ifdef _WIN32
    #undef  TRANSPARENT // there is a marco defined somewhere
#endif

namespace ui {

constexpr ImVec2 WINDOW_SIZE{800, 200};
constexpr ImVec2 INHERIT_PARENT_SIZE{0,0};
constexpr ImVec2 ITEM_SPACING{0,0};
constexpr float Y_BOTTOM = 1.0;
constexpr int LEVEL_COMBO_WIDTH = 100;
constexpr auto TRANSPARENT = IM_COL32(0, 0, 0, 0);

LogWidget::LogWidget():
    presenter{*this}
{
    levels = presenter.handleGetLevels();
    logLevel = presenter.handleGetLevel();
}

void LogWidget::paint()
{
    ImGui::SetNextWindowSize(WINDOW_SIZE, ImGuiCond_FirstUseEver);
    if (ImGui::Begin(gettext(LOG_WIDGET_NAME))) {
        if (ImGui::Button(gettext("Clear"))) {
            presenter.handleClearLogs();
        }
        ImGui::SameLine();

        bool copy = ImGui::Button(gettext("Copy"));
        ImGui::SameLine();
        ImGui::SetNextItemWidth(LEVEL_COMBO_WIDTH);

        if (ImGui::BeginCombo("##Level", levels[logLevel].c_str())) {
            for (int i = 0; i < levels.size(); i++) {
                if (ImGui::Selectable(levels[i].c_str(), i == logLevel)) {
                    logLevel = i;
                    presenter.handleSetLevel(logLevel);
                    ImGui::SetItemDefaultFocus();
                }
            }

            ImGui::EndCombo();
        }

        ImGui::SameLine();

        bool filterEnable = presenter.handleCheckIsFilterSet();
        if (filterEnable) {
            ImGui::BeginDisabled();
        }
        ImGui::InputText(gettext("Filter"), &filter);
        if (filterEnable) {
            ImGui::EndDisabled();
        }

        ImGui::SameLine();
        if(ImGui::Checkbox(gettext("Set"), &filterEnable)) {
            if (filterEnable) {
                presenter.handleSetFilter(filter);
            } else {
                presenter.handleUnsetFilter();
            }
        }

        ImGui::Separator();

        if (ImGui::BeginChild("scrolling", INHERIT_PARENT_SIZE, ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar)) {
            if (copy) {
                ImGui::LogToClipboard();
            } 

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ITEM_SPACING);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, TRANSPARENT);  // Transparent background
            ImGui::PushItemWidth(-FLT_MIN);
            logId = 0;
            presenter.handleDisplayLogs();
            ImGui::PopItemWidth();
            ImGui::PopStyleColor(1);
            ImGui::PopStyleVar();

            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
                ImGui::SetScrollHereY(Y_BOTTOM);
            }
        }

        ImGui::EndChild();
    }

    ImGui::End();
}

void LogWidget::displayLog(ImVec4 color, const std::string& msg)
{
    ImGui::PushID(logId++);

    ImGui::PushStyleColor(ImGuiCol_Text, color);
    ImGui::InputText("##", const_cast<std::string*>(&msg), ImGuiInputTextFlags_ReadOnly);
    ImGui::PopStyleColor();

    ImGui::PopID();
}
}
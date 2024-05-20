#ifndef SRC_UI_UTIL_H
#define SRC_UI_UTIL_H

#include "src/persistence/Data.h"
#include "src/util/TypeTraits.h"

#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

#include <optional>
#include <type_traits>
#include <string_view>
#include <functional>

namespace ui {
constexpr int COORDINATE_INPUT_WIDTH = 50;
constexpr auto PROGRESS_BAR_SIZES = ImVec2{400, 0};
 
void helpMarker(const char* message);
void alignForWidth(float width, float alignment = 0.5f);

template<util::Callable<void> T>
void centeredEnableableButton(std::string_view buttonLabel, 
                              bool enable,
                              T&& completeCallback)
{
    alignForWidth(ImGui::CalcTextSize(buttonLabel.data()).x);
    if(!enable) {
        ImGui::BeginDisabled();
    }
    if (ImGui::Button(buttonLabel.data())) {
        completeCallback();
        ImGui::CloseCurrentPopup();
    }
    if(!enable) {
        ImGui::EndDisabled();
    }
}

template<util::Callable<void> T>
void simpleProgressDisplayer(float progress, 
                             std::string_view buttonLabel, 
                             bool isComplete,
                             T&& completeCallback)
{
    ImGui::ProgressBar(progress, PROGRESS_BAR_SIZES);

    centeredEnableableButton(buttonLabel, isComplete, std::forward<T>(completeCallback));
}

// we are not so care about the error of floating comparison 
template<typename T>
bool inBound(T v, T min, T max)
{
    return (v > min) && (v < max);
}
}

#endif

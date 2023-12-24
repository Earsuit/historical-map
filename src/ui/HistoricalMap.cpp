#include "src/ui/HistoricalMap.h"
#include "src/logger/Util.h"

#include "external/imgui/imgui.h"
#include "external/imgui/imgui_internal.h"
#include "external/imgui/backends/imgui_impl_glfw.h"
#include "external/imgui/backends/imgui_impl_opengl3.h"
#include "external/implot/implot.h"

#include "spdlog/spdlog.h"

#include <stdexcept>
#include <string>

namespace ui {

constexpr int WINDOW_WIDTH = 720;
constexpr int WINDOW_HEIGHT = 1080;
constexpr float MAP_WIDGET_RATIO = 2.0f/3.0f;
constexpr ImVec4 backgroundColor = {0.45f, 0.55f, 0.60f, 1.00f};

namespace {
static void glfwErrorCallback(int error, const char* description)
{
    const std::string message = "Glfw Error " + std::to_string(error) + ": " + description;
    throw std::runtime_error(message);
}
}

HistoricalMap::HistoricalMap()
{
    glfwSetErrorCallback(glfwErrorCallback);
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize glfw.");
    }
    
    const auto glslVersion = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // Create window with graphics context
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Historical Map", NULL, NULL);
    if (window == NULL) {
        throw std::runtime_error("Failed to create window.");
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    if (glewInit()) {
        throw std::runtime_error("Failed to initialize OpenGL loader!");
    }
    
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glslVersion);

    spdlog::get(logger::LOGGER_NAME)->info("Initialization complete.");
}

HistoricalMap::~HistoricalMap()
{
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}

void HistoricalMap::start()
{
    auto& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigDockingWithShift = true;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        buildDockSpace(io);

        tileSourceWidget.paint();
        mapWidget.setTileSource(tileSourceWidget.getTileSource());
        mapWidget.paint();
        logWidget.paint();

        ImGui::Render();
        int displayWidth, displayHeight;
        glfwGetFramebufferSize(window, &displayWidth, &displayHeight);
        glViewport(0, 0, displayWidth, displayHeight);
        glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, backgroundColor.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }
}

// based on example https://gist.github.com/moebiussurfing/d7e6ec46a44985dd557d7678ddfeda99
void HistoricalMap::buildDockSpace(ImGuiIO& io)
{
    static auto firstTime = true;
    ImGuiID dockspace = ImGui::GetID("MyDockSpace");

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    const ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDocking |
                                         ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | 
                                         ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                         ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);

    // Docked windows currently have the flag ImGuiWindowFlags_ChildWindow
    // If we want to remvoe the space between the docked window and the main window, we have to set the style here
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,  ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
    ImGui::Begin("DockSpace", nullptr, windowFlags);
    ImGui::PopStyleVar(3);

    ImGui::DockSpace(dockspace, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_AutoHideTabBar);

    if (firstTime) {
        firstTime = false;

        ImGui::DockBuilderRemoveNode(dockspace); // clear any previous layout
		ImGui::DockBuilderAddNode(dockspace, ImGuiDockNodeFlags_DockSpace | ImGuiDockNodeFlags_PassthruCentralNode);
		ImGui::DockBuilderSetNodeSize(dockspace, viewport->Size);

		// split the dockspace into nodes -- DockBuilderSplitNode takes in the following args in the following order
		//   window ID to split, direction, fraction (between 0 and 1), the final two setting let's us choose which id we want (which ever one we DON'T set as NULL, will be returned by the function)
		//                                                              out_id_at_dir is the id of the node in the direction we specified earlier, out_id_at_opposite_dir is in the opposite direction
        const auto down = ImGui::DockBuilderSplitNode(dockspace, ImGuiDir_Down, 1 - MAP_WIDGET_RATIO, nullptr, &dockspace);

		// we now dock our windows into the docking node we made above
		ImGui::DockBuilderDockWindow(MAP_WIDGET_NAME, dockspace);
		ImGui::DockBuilderDockWindow(LOG_WIDGET_NAME, down);
		ImGui::DockBuilderDockWindow(TILE_SOURCE_WIDGET_NAME, down);
		ImGui::DockBuilderFinish(dockspace);
    }

    ImGui::End();
}

}
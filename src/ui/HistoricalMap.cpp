#include "src/ui/HistoricalMap.h"
#include "src/logger/Util.h"
#include "src/ui/IInfoWidget.h"
#include "src/ui/DefaultInfoWidget.h"
#include "src/ui/ExportInfoWidget.h"
#include "src/ui/ImportInfoWidget.h"
#include "src/ui/MapWidgetNoninteractive.h"

#include "external/imgui/imgui.h"
#include "external/imgui/imgui_internal.h"
#include "external/imgui/backends/imgui_impl_glfw.h"
#include "external/imgui/backends/imgui_impl_opengl3.h"
#include "external/implot/implot.h"
#include "ImFileDialog.h"

#include "spdlog/spdlog.h"

#include <stdexcept>
#include <string>

namespace ui {

constexpr int WINDOW_WIDTH = 1080 * 1.5;
constexpr int WINDOW_HEIGHT = 1080;
constexpr float MAIN_DOCKSPACE_HORIZONTAL_RATIO = 3.6f/5.0f;
constexpr float MAIN_DOCKSPACE_VERTICAL_RATIO = 2.0f/3.0f;
constexpr float MAP_WIDGET_DOCKSPACE_RATIO = 0.5f;
constexpr ImVec4 backgroundColor = {0.45f, 0.55f, 0.60f, 1.00f};
// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
// because it would be confusing to have two docking targets within each others.
constexpr ImGuiWindowFlags WINDOW_FLAGS = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoCollapse |
                                         ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                         ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
constexpr auto MAP_WIDGETS_DOCKSPACE_WINDOW_NAME = "MapDockSpace";
constexpr ImGuiDockNodeFlags DOCKSPACE_FLAG = ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_AutoHideTabBar;
constexpr int MAX_MAP_WIDGET_NUM = 2;
constexpr ImVec2 DOCKSPACE_DEFAULT_ARG = ImVec2{0.0f, 0.0f};

namespace {
static void glfwErrorCallback(int error, const char* description)
{
    const std::string message = "Glfw Error " + std::to_string(error) + ": " + description;
    throw std::runtime_error(message);
}
}

void HistoricalMap::addInteractiveMapWidget(const std::string& source)
{
    mapWidgets.emplace_back(std::make_unique<MapWidget>(source));
}

void HistoricalMap::addNoninteractiveMapWidget(const std::string& source)
{
    mapWidgets.emplace_back(std::make_unique<MapWidgetNoninteractive>(source));
}

void HistoricalMap::clearMapWidgets()
{
    mapWidgets.clear();
}

HistoricalMap::HistoricalMap():
    infoWidget{std::make_unique<DefaultInfoWidget>()},
    tileSourceWidget{},
    presenter{*this}
{   
    mapWidgets.emplace_back(std::make_unique<MapWidget>(presenter.handleGetDefaultMapWidgetSouceName()));

    glfwSetErrorCallback(glfwErrorCallback);
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize glfw.");
    }
    
    const auto glslVersion = "#version 330";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

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

    ifd::FileDialog::getInstance().createTexture = [](const uint8_t* data, int w, int h, ifd::Format fmt) -> void* {
		GLuint tex;

		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		if (fmt == ifd::Format::BGRA) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, data);
		} else if (fmt == ifd::Format::RGBA) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		} else {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		}

		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		return reinterpret_cast<void*>(tex);
	};

    ifd::FileDialog::getInstance().deleteTexture = [](void* tex) {
		GLuint texID = (GLuint)((uintptr_t)tex);
		glDeleteTextures(1, &texID);
	};

    spdlog::get(logger::LOGGER_NAME)->info("Initialization complete.");
}

HistoricalMap::~HistoricalMap()
{
    ImGui_ImplOpenGL3_Shutdown();
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

        buildDockSpace();

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Import")) {
                    if (dynamic_cast<DefaultInfoWidget*>(infoWidget.get()) != nullptr) {
                        presenter.handleClickImport();
                        infoWidget = std::make_unique<ImportInfoWidget>();
                    }
                }

                if (ImGui::MenuItem("Export")) {
                    if (dynamic_cast<DefaultInfoWidget*>(infoWidget.get()) != nullptr) {
                        presenter.handleClickExport();
                        infoWidget = std::make_unique<ExportInfoWidget>();
                    }
                }

                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        infoWidget->paint();
        tileSourceWidget.paint();
        for (auto& mapWidget : mapWidgets) {
            mapWidget->paint();
        }
        logWidget.paint();

        if (infoWidget->complete()) {
            presenter.handleImportExportComplete();
            infoWidget = std::make_unique<DefaultInfoWidget>();
        }

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

void HistoricalMap::buildMapDockSpace()
{
    auto dockspace = ImGui::GetID("MyMapDockSpace");
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,  ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
    ImGui::Begin(MAP_WIDGETS_DOCKSPACE_WINDOW_NAME, nullptr, WINDOW_FLAGS);
    ImGui::PopStyleVar(3);

    ImGui::DockSpace(dockspace, DOCKSPACE_DEFAULT_ARG, DOCKSPACE_FLAG);

    if (mapWidgets.size() != previousDockedMapWidget) {
        ImGui::DockBuilderRemoveNode(dockspace); // clear any previous layout
		ImGui::DockBuilderAddNode(dockspace, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspace, ImGui::GetWindowSize());
        
        const auto right = ImGui::DockBuilderSplitNode(dockspace, ImGuiDir_Right, MAP_WIDGET_DOCKSPACE_RATIO, nullptr, &dockspace);
        ImGui::DockBuilderDockWindow(mapWidgets.front()->getName().c_str(), dockspace);
        if (mapWidgets.size() == MAX_MAP_WIDGET_NUM) {
            ImGui::DockBuilderDockWindow(mapWidgets.back()->getName().c_str(), right);
        }

        previousDockedMapWidget = mapWidgets.size();
    }

    ImGui::End();
}

// based on example https://gist.github.com/moebiussurfing/d7e6ec46a44985dd557d7678ddfeda99
void HistoricalMap::buildDockSpace()
{
    static auto firstTime = true;
    auto dockspace = ImGui::GetID("MyDockSpace");

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);

    // Docked windows currently have the flag ImGuiWindowFlags_ChildWindow
    // If we want to remvoe the space between the docked window and the main window, we have to set the style here
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,  ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
    ImGui::Begin("DockSpace", nullptr, WINDOW_FLAGS);
    ImGui::PopStyleVar(3);

    ImGui::DockSpace(dockspace, DOCKSPACE_DEFAULT_ARG, DOCKSPACE_FLAG);

    if (firstTime) {
        firstTime = false;

        ImGui::DockBuilderRemoveNode(dockspace); // clear any previous layout
		ImGui::DockBuilderAddNode(dockspace, ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(dockspace, viewport->Size);

		// split the dockspace into nodes -- DockBuilderSplitNode takes in the following args in the following order
		// window ID to split, direction, fraction (between 0 and 1), the final two setting let's us choose which id we want (which ever one we DON'T set as NULL, will be returned by the function)
		// out_id_at_dir is the id of the node in the direction we specified earlier, out_id_at_opposite_dir is in the opposite direction
        const auto down = ImGui::DockBuilderSplitNode(dockspace, ImGuiDir_Down, 1 - MAIN_DOCKSPACE_VERTICAL_RATIO, nullptr, &dockspace);
        const auto right = ImGui::DockBuilderSplitNode(dockspace, ImGuiDir_Right, 1 - MAIN_DOCKSPACE_HORIZONTAL_RATIO, nullptr, &dockspace);

		// we now dock our windows into the docking node we made above
        ImGui::DockBuilderDockWindow(MAP_WIDGETS_DOCKSPACE_WINDOW_NAME, dockspace);
		ImGui::DockBuilderDockWindow(LOG_WIDGET_NAME, down);
		ImGui::DockBuilderDockWindow(TILE_SOURCE_WIDGET_NAME, down);
        ImGui::DockBuilderDockWindow(INFO_WIDGET_NAME, right);
		ImGui::DockBuilderFinish(dockspace);
    }

    ImGui::End();

    buildMapDockSpace();
}

}
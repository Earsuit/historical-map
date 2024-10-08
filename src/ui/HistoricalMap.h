#ifndef SRC_UI_HISTORICAL_MAP_H
#define SRC_UI_HISTORICAL_MAP_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "src/ui/TileSourceWidget.h"
#include "src/ui/LogWidget.h"
#include "src/ui/MapWidget.h"
#include "src/ui/IInfoWidget.h"
#include "src/presentation/MainViewInterface.h"
#include "src/presentation/MainViewPresenter.h"
#include "src/logger/ModuleLogger.h"

#include <filesystem>
#include <memory>
#include <vector>
#include <atomic>

namespace ui {

class HistoricalMap: public presentation::MainViewInterface {
public:
    HistoricalMap();
    ~HistoricalMap();

    void start();

    virtual void addInteractiveMapWidget(const std::string& source) override;
    virtual void addNoninteractiveMapWidget(const std::string& source) override;
    virtual void clearMapWidgets() override;

    void setDpiScale(float scale);

private:
    GLFWwindow* window;
    LogWidget logWidget;
    logger::ModuleLogger logger;
    std::unique_ptr<IInfoWidget> infoWidget;
    std::vector<std::unique_ptr<MapWidget>> mapWidgets;
    TileSourceWidget tileSourceWidget;
    presentation::MainViewPresenter presenter;
    int previousDockedMapWidget = 0;
    ImGuiID down, left;
    std::string iniFilePath;
    std::filesystem::path executableLocation;
    std::atomic<float> scale;
    std::atomic_bool isDpiChanged = false;
    ImGuiStyle backupStyle;

    void buildDockSpace();
    void buildMapDockSpace();
    ImGuiStyle setStyle();
    void loadDefaultFonts(float scaleFactor);
    void handleDpiScaleChange();
    void scaleUiElement(float scaleFactor);
    void initializeUi();
};

}

#endif

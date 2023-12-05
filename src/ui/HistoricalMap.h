#ifndef SRC_UI_HISTORICAL_MAP_H
#define SRC_UI_HISTORICAL_MAP_H

namespace ui {

class HistoricalMap {
public:
    bool start();

private:
    bool prepare();
    void mainLoop();
};

}

#endif

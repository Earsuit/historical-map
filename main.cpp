#include "src/ui/HistoricalMap.h"

#ifdef _WIN32
#include <windows.h>
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    ui::HistoricalMap historicalMap;

    historicalMap.start();

    return 0;
}
#else
int main()
{
    ui::HistoricalMap historicalMap;

    historicalMap.start();

    return 0;
}
#endif
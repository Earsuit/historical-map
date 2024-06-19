#include "src/presentation/MainViewPresenter.h"
#include "src/presentation/Util.h"
#include "src/util/ExecuteablePath.h"
#include "src/logger/LoggerManager.h"

#include <locale>
#include <libintl.h>

namespace presentation {
constexpr auto ENGLISH = "English";
constexpr auto ENGLISH_LOCALE = "en_US.UTF-8";
constexpr auto CHINESE = "中文";
constexpr auto CHINESE_LOCALE = "zh_CN.UTF-8";
constexpr auto DOMAIN_NAME = "HistoricalInfo";
constexpr auto DOMAIN_PATH = "locale";
constexpr auto CODE_SET = "UTF-8";
constexpr auto LOGGER_NAME = "MainViewPresenter";
constexpr auto FOLLOW_SYSTEM = "SYSTEM";
constexpr bool OVERWRITE = true;

void setLocale(const char* locale)
{
#ifdef __linux__
    setlocale (LC_ALL, locale);
#elif defined(__APPLE__)
    setenv("LC_ALL", locale, OVERWRITE);
#else
    setlocale (LC_ALL, locale);
#endif
}

MainViewPresenter::MainViewPresenter(MainViewInterface& view):
    view{view},
    databaseModel{model::DatabaseModel::getInstance()},
    model{model::CacheModel::getInstance()},
    logger{logger::LoggerManager::getInstance().getLogger(LOGGER_NAME)},
    executableDirectory{util::getExecutablePath().remove_filename()}
{
    handleSetLanguage(FOLLOW_SYSTEM);
}

std::string MainViewPresenter::handleGetDefaultMapWidgetSouceName() const noexcept
{
    return model::PERMENANT_SOURCE;
}

void MainViewPresenter::handleClickImport()
{
    clearCache();
    view.clearMapWidgets();
    view.addNoninteractiveMapWidget(handleGetDefaultMapWidgetSouceName());
    view.addNoninteractiveMapWidget(IMPORT_SOURCE);
}

void MainViewPresenter::handleClickExport()
{
    clearCache();
    view.clearMapWidgets();
    view.addNoninteractiveMapWidget(handleGetDefaultMapWidgetSouceName());
}

void MainViewPresenter::handleImportExportComplete()
{
    view.clearMapWidgets();
    view.addInteractiveMapWidget(handleGetDefaultMapWidgetSouceName());
}

void MainViewPresenter::clearCache()
{
    model.removeHistoricalInfoFromSource(handleGetDefaultMapWidgetSouceName(), databaseModel.getYear());
}

void MainViewPresenter::handleSetLanguage(const std::string& language)
{
    if (language == ENGLISH) {
        setLocale(ENGLISH_LOCALE);
        logger.debug("Set locale {}, current LC_ALL {}", ENGLISH_LOCALE, getenv("LC_ALL"));
    } else if (language == CHINESE) {
        setLocale(CHINESE_LOCALE);
        logger.debug("Set locale {}, current LC_ALL {}", CHINESE_LOCALE, getenv("LC_ALL"));
    } else {
        logger.debug("Follow system locale");
    }

    bindtextdomain(DOMAIN_NAME, (executableDirectory / DOMAIN_PATH).string().c_str());
    bind_textdomain_codeset(DOMAIN_NAME, CODE_SET);
    textdomain(DOMAIN_NAME);
}

std::vector<std::string> MainViewPresenter::handleGetLanguages() const
{
    return {CHINESE, ENGLISH};
}
}
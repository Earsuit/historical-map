#include "src/presentation/InfoSelectorPresenter.h"

#include "src/logger/Util.h"

namespace presentation {
InfoSelectorPresenter::InfoSelectorPresenter(const std::string& fromSource, const std::string& toSource):
    logger{spdlog::get(logger::LOGGER_NAME)},
    dynamicModel{model::DynamicInfoModel::getInstance()},
    fromSource{fromSource},
    toSource{toSource}
{
    dynamicModel.addSource(toSource);
}

InfoSelectorPresenter::~InfoSelectorPresenter()
{
    dynamicModel.removeSource(toSource);
}

void InfoSelectorPresenter::handleSelectCountry(const std::string& name)
{
    if (auto fromInfo = dynamicModel.getHistoricalInfo(fromSource); fromInfo) {
        if (fromInfo->containsCountry(name)) {
            auto& country = fromInfo->getCountry(name);

            if (auto toInfo = dynamicModel.getHistoricalInfo(toSource); toInfo) {
                toInfo->addCountry(country);
            } else {
                logger->error("Select country {} fail because the target {} doesn't exists", name, toSource);
            }
        } else {
            logger->error("Select country {} fail because it doesn't exists in {}", name, fromSource);
        }
    } else {
        logger->error("Select country {} fail due to invalid source {}", name, fromSource);
    }
}

void InfoSelectorPresenter::handleSelectCity(const std::string& name)
{
    if (auto fromInfo = dynamicModel.getHistoricalInfo(fromSource); fromInfo) {
        if (fromInfo->containsCity(name)) {
            auto& city = fromInfo->getCity(name);

            if (auto toInfo = dynamicModel.getHistoricalInfo(toSource); toInfo) {
                toInfo->addCity(name, city.coordinate);
            } else {
                logger->error("Select city {} fail because the target {} doesn't exists", name, toSource);
            }
        } else {
            logger->error("Select city {} fail because it doesn't exists in {}", name, fromSource);
        }
    } else {
        logger->error("Select city {} fail due to invalid source {}", name, fromSource);
    }
}

void InfoSelectorPresenter::handleSelectNote()
{
    if (auto fromInfo = dynamicModel.getHistoricalInfo(fromSource); fromInfo) {
        if (auto toInfo = dynamicModel.getHistoricalInfo(toSource); toInfo) {
            toInfo->getNote() = fromInfo->getNote();
        } else {
            logger->error("Select note fail because the target {} doesn't exists", toSource);
        }
    } else {
        logger->error("Select note fail due to invalid source {}", fromSource);
    }
}

void InfoSelectorPresenter::handleDeselectCountry(const std::string& name)
{
    if (auto info = dynamicModel.getHistoricalInfo(toSource); info) {
        info->removeCountry(name);
    } else {
        logger->error("Deselect country {} fail due to invalid target {}", name, fromSource);
    }
}

void InfoSelectorPresenter::handleDeselectCity(const std::string& name)
{
    if (auto info = dynamicModel.getHistoricalInfo(toSource); info) {
        info->removeCity(name);
    } else {
        logger->error("Deselect city {} fail due to invalid target {}", name, fromSource);
    }
}

void InfoSelectorPresenter::handleDeselectNote()
{
    if (auto info = dynamicModel.getHistoricalInfo(toSource); info) {
        info->getNote().text.clear();
    } else {
        logger->error("Deselect note fail due to invalid target {}", fromSource);
    }
}

bool InfoSelectorPresenter::isCountrySelected(const std::string& name)
{
    if (auto info = dynamicModel.getHistoricalInfo(toSource); info) {
        return info->containsCountry(name);
    }
    
    logger->error("Check country {} selection fail due to invalid target {}", name, fromSource);

    return false;
}

bool InfoSelectorPresenter::isCitySelected(const std::string& name)
{
    if (auto info = dynamicModel.getHistoricalInfo(toSource); info) {
        return info->containsCity(name);
    }
        
    logger->error("Check city {} selection fail due to invalid target {}", name, fromSource);

    return false;
}

bool InfoSelectorPresenter::isNoteSelected()
{
    if (auto info = dynamicModel.getHistoricalInfo(toSource); info) {
        return !info->getNote().text.empty();
    }
        
    logger->error("Check note selection fail due to invalid target {}", fromSource);

    return false;
}
}
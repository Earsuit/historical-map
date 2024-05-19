#ifndef SRC_PRESENTATION_IMPORT_YEAR_PRESENTER_H
#define SRC_PRESENTATION_IMPORT_YEAR_PRESENTER_H

#include "src/model/DynamicInfoModel.h"
#include "src/presentation/DatabaseYearPresenter.h"

#include "spdlog/spdlog.h"

#include <string>
#include <memory>
#include <set>

namespace presentation {
class ImportYearPresenter: public DatabaseYearPresenter {
public:
    ImportYearPresenter(const std::string& source);

private:
    std::shared_ptr<spdlog::logger> logger;
    model::DynamicInfoModel& dynamicInfoModel;
    std::string source;
    std::set<int> years;

    void initYearsList();

    virtual int moveYearForward() noexcept override;
    virtual int moveYearBackward() noexcept override;
    virtual int setYear(int year) noexcept override;
    virtual int getMaxYear() const noexcept override;
    virtual int getMinYear() const noexcept override;
};
}

#endif

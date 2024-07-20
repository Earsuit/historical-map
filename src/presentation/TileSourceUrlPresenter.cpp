#include "src/presentation/TileSourceUrlPresenter.h"

namespace presentation {
TileSourceUrlPresenter::TileSourceUrlPresenter():
    model{model::TileModel::getInstance()},
    source{std::make_shared<tile::TileSourceUrl>(url)}
{
    model.setTileSource(source); 
}

bool TileSourceUrlPresenter::handleSetUrl(const std::string& url)
{
    if (source->setUrl(url)) {
        this->url = url;
        return true;
    }
    
    return false;
}
}
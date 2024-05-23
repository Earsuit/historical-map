#include "src/presentation/TileSourceUrlPresenter.h"

namespace presentation {
TileSourceUrlPresenter::TileSourceUrlPresenter():
    model{model::TileModel::getInstance()}
{
    handleSetTileSource();
}

void TileSourceUrlPresenter::handleSetUrl(const std::string& url)
{
    this->url = url;
}

void TileSourceUrlPresenter::handleSetTileSource() 
{ 
    model.setTileSource(std::make_shared<tile::TileSourceUrl>(url)); 
}

}
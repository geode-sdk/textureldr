#define GEODE_DEFINE_EVENT_EXPORTS
#include "../include/TextureLoader.hpp"

#include "PackManager.hpp"
#include "PackSelectPopup.hpp"

using namespace geode;

namespace api = geode::texture_loader;

api::Pack convertPack(std::shared_ptr<Pack> const& pack) {
    api::Pack res;
    res.id = pack->getID();
    res.name = pack->getDisplayName();
    res.path = pack->getOriginPath();
    res.resourcesPath = pack->getResourcesPath();
    // if the pack has a mod.json
    if (auto opt = pack->getInfo()) {
        auto info = opt.value();
        res.version = info.m_version;
        res.authors = info.m_authors;
    }
    return res;
}

std::vector<api::Pack> api::getAvailablePacks() {
    return utils::ranges::map<std::vector<api::Pack>>(PackManager::get()->getAvailablePacks(), convertPack);
}
std::vector<api::Pack> api::getAppliedPacks() {
    return utils::ranges::map<std::vector<api::Pack>>(PackManager::get()->getAppliedPacks(), convertPack);
}
std::vector<api::Pack> api::getFailedPacks() {
    return utils::ranges::map<std::vector<api::Pack>>(PackManager::get()->getFailedPacks(), convertPack);
}

void api::openPopup() {
    PackSelectPopup::create()->show();
}

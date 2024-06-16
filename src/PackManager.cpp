#include "PackManager.hpp"
#include <Geode/loader/Mod.hpp>
#include <Geode/loader/ModEvent.hpp>
#include <Geode/loader/Loader.hpp>
#include <Geode/utils/file.hpp>
#include <Geode/utils/ranges.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/FMODAudioEngine.hpp>
#include <utility>

using namespace geode::prelude;

PackManager* PackManager::get() {
    static auto inst = new PackManager;
    return inst;
}

std::vector<std::shared_ptr<Pack>> PackManager::getAvailablePacks() const {
    return m_available;
}

std::vector<std::shared_ptr<Pack>> PackManager::getAppliedPacks() const {
    return m_applied;
}

void PackManager::movePackToIdx(const std::shared_ptr<Pack>& pack, PackListType to, size_t index) {
    auto& destination = to == PackListType::Applied ? m_applied : m_available;
    if (ranges::contains(destination, pack)) {
        ranges::move(destination, pack, index);
    } else {
        auto& from = to != PackListType::Applied ? m_applied : m_available;
        ranges::remove(from, pack);
        if (index < destination.size()) {
            destination.insert(destination.begin() + static_cast<ptrdiff_t>(index), pack);
        } else {
            destination.push_back(pack);
        }
    }
}

void PackManager::savePacks() {
    Mod::get()->getSaveContainer()["applied"] = m_applied;
}

std::filesystem::path PackManager::getPackDir() {
    return Mod::get()->getConfigDir() / "packs";
}

size_t PackManager::loadPacks() {
    auto packDir = PackManager::getPackDir();
    (void)file::createDirectoryAll(packDir);

    size_t loaded = 0;

    std::vector<std::shared_ptr<Pack>> found;

    // Load new packs
    for (auto& dir : std::filesystem::directory_iterator(packDir)) {
        auto packRes = Pack::from(dir);
        if (!packRes) {
            log::warn("Unable to load pack {}: {}", dir, packRes.unwrapErr());
        } else {
            found.push_back(packRes.unwrap());
            loaded++;
        }
    }


    std::vector<std::shared_ptr<Pack>> newApplied;
    
    std::vector<std::shared_ptr<Pack>> savedApplied;
    // manually do this so we can skip packs that fail
    for (auto const& obj : Mod::get()->getSavedValue<matjson::Array>("applied")) {
        if (obj.is_object() && obj.contains("path") && obj["path"].is_string()) {
            auto res = Pack::from(obj["path"].as<std::filesystem::path>());
            if (res) {
                savedApplied.push_back(res.unwrap());
            }
        }
    }

    // on startup (when no packs have been loaded yet) iterate through savedApplied
    // this whole code would be a lot simpler if the "applied" saved value wasnt just updated on closing the game..
    auto& iterApplied = (m_available.empty() && m_applied.empty()) ? savedApplied : m_applied;
    for (auto& pack : iterApplied) {
        const auto pred = [&](auto const& other) {
            return pack->getID() == other->getID();
        };
        if (ranges::contains(found, pred)) {
            ranges::remove(found, pred);
            newApplied.push_back(pack);
        }
    }

    m_applied = newApplied;
    m_available = found;

    this->updateAppliedPacks();

    log::info("Loaded {} packs", loaded);

    return loaded;
}

void PackManager::updateAppliedPacks() {
    for (auto& pack : m_available) {
        (void)pack->unapply();
    }
    for (auto& pack : ranges::reverse(m_applied)) {
        (void)pack->apply();
    }
}

void PackManager::applyPacks(CreateLayerFunc func) {
    this->updateAppliedPacks();
    // TODO: find this function
    // FMODAudioEngine::sharedEngine()->stopAllMusic();
    loadPacks();    
    reloadTextures(std::move(func));
}

$on_mod(Loaded) {
    log::info("Loading texture packs");
    PackManager::get()->loadPacks();
}

$on_mod(DataSaved) {
    log::info("Saving texture packs");
    PackManager::get()->savePacks();
}

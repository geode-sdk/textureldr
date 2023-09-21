#include "PackManager.hpp"
#include <Geode/loader/Mod.hpp>
#include <Geode/loader/ModEvent.hpp>
#include <Geode/loader/ModJsonTest.hpp>
#include <Geode/loader/Loader.hpp>
#include <Geode/utils/file.hpp>
#include <Geode/utils/map.hpp>
#include <Geode/utils/ranges.hpp>
#include <Geode/binding/GameManager.hpp>

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

void PackManager::movePackToIdx(std::shared_ptr<Pack> pack, PackListType to, size_t index) {
    auto& destination = to == PackListType::Applied ? m_applied : m_available;
    if (ranges::contains(destination, pack)) {
        ranges::move(destination, pack, index);
    } else {
        auto& from = to != PackListType::Applied ? m_applied : m_available;
        ranges::remove(from, pack);
        if (index < destination.size()) {
            destination.insert(destination.begin() + index, pack);
        } else {
            destination.push_back(pack);
        }
    }
}

bool PackManager::isApplied(std::shared_ptr<Pack> pack) const {
    return ranges::contains(m_applied, pack);
}

void PackManager::savePacks() {
    Mod::get()->setSavedValue("applied", m_applied);
}

ghc::filesystem::path PackManager::getPackDir() const {
    return Mod::get()->getConfigDir() / "packs";
}

size_t PackManager::loadPacks() {
    auto packDir = this->getPackDir();
    (void)file::createDirectoryAll(packDir);

    size_t loaded = 0;

    std::vector<std::shared_ptr<Pack>> found;

    // Load new packs
    for (auto& dir : ghc::filesystem::directory_iterator(packDir)) {
        if (ghc::filesystem::is_directory(dir)) {
            auto packRes = Pack::from(dir);
            if (!packRes) {
                log::warn("Unable to load pack {}: {}", dir, packRes.unwrapErr());
            } else {
                found.push_back(packRes.unwrap());
                loaded++;
            }
        }
    }

    std::vector<std::shared_ptr<Pack>> newApplied;
    auto savedApplied = Mod::get()->getSavedValue<std::vector<std::shared_ptr<Pack>>>("applied");
    // on startup (when no packs have been loaded yet) iterate through savedApplied
    // this whole code would be a lot simpler if the "applied" saved value wasnt just updated on closing the game..
    auto& iterApplied = (m_available.empty() && m_applied.empty()) ? savedApplied : m_applied;
    for (auto& pack : iterApplied) {
        const auto pred = [&](auto const& other) {
            return pack->getPath() == other->getPath();
        };
        if (ranges::contains(found, pred)) {
            ranges::remove(found, pred);
            newApplied.push_back(pack);
        }
    }

    m_applied = newApplied;
    m_available = found;

    this->addPackPaths();

    log::info("Loaded {} packs", loaded);

    return loaded;
}

void PackManager::addPackPaths() {
    for (auto& pack : m_available) {
        (void)pack->unapply();
    }
    for (auto& pack : ranges::reverse(m_applied)) {
        (void)pack->apply();
    }
}

void PackManager::applyPacks(CreateLayerFunc func) {
    this->addPackPaths();
    reloadTextures(func);
}

$on_mod(Loaded) {
    log::info("Loading texture packs");
    PackManager::get()->loadPacks();
}

$on_mod(DataSaved) {
    log::info("Saving texture packs");
    PackManager::get()->savePacks();
}

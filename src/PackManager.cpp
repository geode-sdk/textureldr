#include "PackManager.hpp"
#include <Geode/loader/Mod.hpp>
#include <Geode/loader/ModEvent.hpp>
#include <Geode/loader/ModJsonTest.hpp>
#include <Geode/loader/Loader.hpp>
#include <Geode/utils/file.hpp>
#include <Geode/utils/map.hpp>
#include <Geode/utils/ranges.hpp>
#include <Geode/binding/GameManager.hpp>

USE_GEODE_NAMESPACE();

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

void PackManager::movePackUp(std::shared_ptr<Pack> pack) {
    if (auto ix = ranges::indexOf(m_available, pack)) {
        if (ix.value() > 0) {
            ranges::move(m_available, pack, ix.value() - 1);
        }
    }
    if (auto ix = ranges::indexOf(m_applied, pack)) {
        if (ix.value() > 0) {
            ranges::move(m_applied, pack, ix.value() - 1);
        }
    }
}

void PackManager::movePackDown(std::shared_ptr<Pack> pack) {
    if (auto ix = ranges::indexOf(m_available, pack)) {
        if (ix.value() < m_available.size() - 1) {
            ranges::move(m_available, pack, ix.value() + 1);
        }
    }
    if (auto ix = ranges::indexOf(m_applied, pack)) {
        if (ix.value() < m_applied.size() - 1) {
            ranges::move(m_applied, pack, ix.value() + 1);
        }
    }
}

bool PackManager::isApplied(std::shared_ptr<Pack> pack) const {
    return ranges::contains(m_applied, pack);
}

void PackManager::moveToApplied(std::shared_ptr<Pack> pack) {
    if (!ranges::contains(m_applied, pack)) {
        m_applied.insert(m_applied.begin(), pack);
        ranges::remove(m_available, pack);
    }
}

void PackManager::moveToAvailable(std::shared_ptr<Pack> pack) {
    if (!ranges::contains(m_available, pack)) {
        m_available.insert(m_available.begin(), pack);
        ranges::remove(m_applied, pack);
    }
}

void PackManager::savePacks() {
    Mod::get()->setSavedValue("available", m_available);
    Mod::get()->setSavedValue("applied", m_applied);
}

size_t PackManager::loadPacks() {
    auto packDir = Mod::get()->getConfigDir() / "packs";
    (void)file::createDirectoryAll(packDir);

    // Load saved packs
    m_available = Mod::get()->getSavedValue<std::vector<std::shared_ptr<Pack>>>("available");
    m_applied = Mod::get()->getSavedValue<std::vector<std::shared_ptr<Pack>>>("applied");

    auto loaded = m_available.size() + m_applied.size();

    // Load new packs
    for (auto& dir : ghc::filesystem::directory_iterator(packDir)) {
        if (
            ghc::filesystem::is_directory(dir) && 
            !ranges::contains(m_available, [dir](auto pack) {
                return pack->getPath() == dir;
            }) &&
            !ranges::contains(m_applied, [dir](auto pack) {
                return pack->getPath() == dir;
            })
        ) {
            auto packRes = Pack::from(dir);
            if (!packRes) {
                log::warn("Unable to load pack {}: {}", dir, packRes.unwrapErr());
            } else {
                m_available.push_back(packRes.unwrap());
                loaded++;
            }
        }
    }

    this->addPackPaths();

    log::info("Loaded {} packs", loaded);

    return loaded;
}

void PackManager::addPackPaths() {
    for (auto& pack : m_available) {
        (void)pack->unapply();
    }
    for (auto& pack : m_applied) {
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

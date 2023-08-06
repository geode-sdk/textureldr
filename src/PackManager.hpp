#pragma once

#include "Pack.hpp"
#include <unordered_map>
#include <Geode/utils/cocos.hpp>

class PackManager {
protected:
    std::vector<std::shared_ptr<Pack>> m_available;
    std::vector<std::shared_ptr<Pack>> m_applied;

    void addPackPaths();

public:
    static PackManager* get();

    std::vector<std::shared_ptr<Pack>> getAvailablePacks() const;
    std::vector<std::shared_ptr<Pack>> getAppliedPacks() const;

    void movePackUp(std::shared_ptr<Pack> pack);
    void movePackDown(std::shared_ptr<Pack> pack);

    bool isApplied(std::shared_ptr<Pack> pack) const;
    void moveToApplied(std::shared_ptr<Pack> pack);
    void moveToAvailable(std::shared_ptr<Pack> pack);

    void savePacks();
    size_t loadPacks();
    void applyPacks(geode::cocos::CreateLayerFunc func = nullptr);

    ghc::filesystem::path getPackDir() const;
};

#pragma once

#include "Pack.hpp"
#include <unordered_map>
#include <Geode/utils/cocos.hpp>

enum class PackListType {
    Available,
    Applied
};

class PackManager {
protected:
    std::vector<std::shared_ptr<Pack>> m_available;
    std::vector<std::shared_ptr<Pack>> m_applied;

    void addPackPaths();

public:
    static PackManager* get();

    std::vector<std::shared_ptr<Pack>> getAvailablePacks() const;
    std::vector<std::shared_ptr<Pack>> getAppliedPacks() const;

    void movePackToIdx(std::shared_ptr<Pack> pack, PackListType to, size_t index);

    bool isApplied(std::shared_ptr<Pack> pack) const;

    void savePacks();
    size_t loadPacks();
    void applyPacks(geode::cocos::CreateLayerFunc func = nullptr);

    ghc::filesystem::path getPackDir() const;
};

#pragma once

#include "Pack.hpp"
#include <Geode/utils/cocos.hpp>
#include <asp/fs.hpp>

enum class PackListType {
    Available,
    Applied
};

class PackManager {
protected:
    std::vector<std::shared_ptr<Pack>> m_available;
    std::vector<std::shared_ptr<Pack>> m_applied;

    void updateAppliedPacks();

public:
    static PackManager* get();

    [[nodiscard]] std::vector<std::shared_ptr<Pack>> getAvailablePacks() const;
    [[nodiscard]] std::vector<std::shared_ptr<Pack>> getAppliedPacks() const;

    void movePackToIdx(const std::shared_ptr<Pack>& pack, PackListType to, size_t index);

    void savePacks();
    size_t loadPacks();
    void applyPacks(geode::cocos::CreateSceneFunc func = nullptr);

    [[nodiscard]] static asp::fs::Path getPackDir();
};

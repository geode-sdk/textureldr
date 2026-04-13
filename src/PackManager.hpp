#pragma once

#include "Pack.hpp"
#include <Geode/utils/cocos.hpp>

enum class PackListType {
    Blocked,
    Available,
    Applied
};

class PackManager {
protected:
    std::vector<std::shared_ptr<Pack>> m_loadfailed;
    std::vector<std::shared_ptr<Pack>> m_available;
    std::vector<std::shared_ptr<Pack>> m_applied;

    void updateAppliedPacks();

public:
    static PackManager* get();

    [[nodiscard]] std::vector<std::shared_ptr<Pack>> getAvailablePacks() const;
    [[nodiscard]] std::vector<std::shared_ptr<Pack>> getAppliedPacks() const;
    [[nodiscard]] std::vector<std::shared_ptr<Pack>> getFailedPacks() const;

    void movePackToIdx(const std::shared_ptr<Pack>& pack, PackListType to, size_t index);

    void savePacks();
    size_t loadPacks();
    void applyPacks(geode::cocos::CreateLayerFunc func = nullptr);

    [[nodiscard]] static std::filesystem::path getPackDir();
};

#pragma once

#include <Geode/ui/Popup.hpp>
#include "Pack.hpp"

using namespace geode::prelude;

class PackInfoPopup : public Popup {
protected:
    std::shared_ptr<Pack> m_pack;

    bool init(const std::shared_ptr<Pack>& pack);

    std::filesystem::path getPathInPack(const char* filename) const;
public:
    static PackInfoPopup* create(const std::shared_ptr<Pack>& pack);
};

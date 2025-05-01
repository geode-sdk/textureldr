#pragma once

#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include "Pack.hpp"
#include <functional>

using namespace geode::prelude;

class PackSelectPopup;

class PackNode : public CCNode {
protected:
    PackSelectPopup* m_layer;
    std::shared_ptr<Pack> m_pack;
    CCScale9Sprite* m_draggingBg;

    bool init(
        PackSelectPopup* layer,
        const std::shared_ptr<Pack>& pack,
        float width
    );

    void onView(CCObject*);

public:
    static PackNode* create(
        PackSelectPopup* layer,
        const std::shared_ptr<Pack>& pack,
        float width
    );

    std::shared_ptr<Pack> getPack() { return m_pack; }

    static constexpr float HEIGHT = 35.f * .88f;
};

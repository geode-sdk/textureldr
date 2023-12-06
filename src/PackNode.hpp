#pragma once

#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include "Pack.hpp"
#include <functional>

using namespace geode::prelude;

class PackSelectLayer;

class PackNode : public CCNode {
protected:
    PackSelectLayer* m_layer;
    std::shared_ptr<Pack> m_pack;
    CCNode* m_draggingBg;

    bool init(
        PackSelectLayer* layer,
        std::shared_ptr<Pack> pack,
        float width
    );

    void onView(CCObject*);

public:
    static PackNode* create(
        PackSelectLayer* layer,
        std::shared_ptr<Pack> pack,
        float width
    );

    std::shared_ptr<Pack> getPack() { return m_pack; }

    static constexpr float HEIGHT = 35.f;
};

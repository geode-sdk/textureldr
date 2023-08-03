#pragma once

#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include "Pack.hpp"

using namespace geode::prelude;

class PackSelectLayer;

class PackNode : public CCNode {
protected:
    PackSelectLayer* m_layer;
    std::shared_ptr<Pack> m_pack;

    bool init(
        PackSelectLayer* layer,
        std::shared_ptr<Pack> pack,
        float width
    );

    void onMoveUp(CCObject*);
    void onMoveDown(CCObject*);
    void onMoveToApplied(CCObject*);
    void onMoveToAvailable(CCObject*);
    void onView(CCObject*);

public:
    static PackNode* create(
        PackSelectLayer* layer,
        std::shared_ptr<Pack> pack,
        float width
    );
};

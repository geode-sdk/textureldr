#pragma once

#include <Geode/DefaultInclude.hpp>
#include <cocos2d.h>
#include <Geode/ui/ScrollLayer.hpp>

using namespace geode::prelude;

class BoundedScrollLayer : public ScrollLayer {
protected:
    BoundedScrollLayer(CCRect const& rect) : ScrollLayer(rect, true, true) {}

public:
    bool m_doScroll = false;
    static BoundedScrollLayer* create(
        cocos2d::CCSize const& size
    );

    bool testLocation(CCPoint point);
    void scrollWheel(float y, float) override;
};
#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class DragThingy : public CCLayer {
protected:
    Function<void()> m_onClick;
    Function<void(CCPoint)> m_onMove;
    Function<void()> m_onRelease;

    bool init(Function<void()> onClick, Function<void(CCPoint)> onMove, Function<void()> onRelease);
    
    bool ccTouchBegan(CCTouch* touch, CCEvent*) override;
    void ccTouchMoved(CCTouch* touch, CCEvent*) override;
    void ccTouchEnded(CCTouch* touch, CCEvent*) override;

public:
    static DragThingy* create(Function<void()> onClick, Function<void(CCPoint)> onMove, Function<void()> onRelease);
};
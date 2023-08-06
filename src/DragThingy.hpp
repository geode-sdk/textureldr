#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class DragThingy : public CCLayer {
protected:
    std::function<void()> m_onClick;
    std::function<void(CCPoint)> m_onMove;
    std::function<void()> m_onRelease;

    bool init(std::function<void()> onClick, std::function<void(CCPoint)> onMove, std::function<void()> onRelease);
    
    bool ccTouchBegan(CCTouch* touch, CCEvent*) override;
    void ccTouchMoved(CCTouch* touch, CCEvent*) override;
    void ccTouchEnded(CCTouch* touch, CCEvent*) override;

public:
    static DragThingy* create(std::function<void()> onClick, std::function<void(CCPoint)> onMove, std::function<void()> onRelease);
};
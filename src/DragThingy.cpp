#include "DragThingy.hpp"

bool DragThingy::init(std::function<void()> onClick, std::function<void(CCPoint)> onMove, std::function<void()> onRelease) {
    if (!CCLayer::init()) return false;

    m_onClick = onClick;
    m_onMove = onMove;
    m_onRelease = onRelease;

    this->setTouchMode(kCCTouchesOneByOne);
    this->setTouchEnabled(true);

    this->ignoreAnchorPointForPosition(false);

    return true;
}

bool DragThingy::ccTouchBegan(CCTouch* touch, CCEvent*) {
    auto const point = this->getParent()->convertTouchToNodeSpace(touch);
    auto const rect = this->boundingBox();
    if (rect.containsPoint(point)) {
        if (m_onClick)
            m_onClick();
        return true;
    }
    return false;
}

void DragThingy::ccTouchMoved(CCTouch* touch, CCEvent*) {
    if (m_onMove)
        m_onMove(touch->getDelta());
}

void DragThingy::ccTouchEnded(CCTouch* touch, CCEvent*) {
    if (m_onRelease)
        m_onRelease();
}

DragThingy* DragThingy::create(std::function<void()> onClick, std::function<void(CCPoint)> onMove, std::function<void()> onRelease) {
    auto ret = new DragThingy;
    if (ret && ret->init(onClick, onMove, onRelease)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}
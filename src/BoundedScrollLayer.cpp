#include "BoundedScrollLayer.hpp"
#include <Geode/modify/CCMouseDispatcher.hpp>

// a bit hacky but allows for multiple scroll layers at once
class $modify(CCMouseDispatcher) {

	bool dispatchScrollMSG(float x, float y) {
        for (CCMouseHandler* handler : CCArrayExt<CCMouseHandler*>(m_pMouseHandlers)) {
            if (BoundedScrollLayer* scroll = typeinfo_cast<BoundedScrollLayer*>(handler->getDelegate())) {
                scroll->m_doScroll = true;
                scroll->scrollWheel(x, y);
                scroll->m_doScroll = false;
            }
        }
        return CCMouseDispatcher::dispatchScrollMSG(x, y);
    }
};


BoundedScrollLayer* BoundedScrollLayer::create(CCSize const& size) {
    auto ret = new BoundedScrollLayer({ 0, 0, size.width, size.height });
    ret->autorelease();
    return ret;
}

bool BoundedScrollLayer::testLocation(CCPoint point) {
    
    CCPoint mousePoint = convertToNodeSpace({point.x + getPositionX(), point.y + getPositionY()});

    if (boundingBox().containsPoint(mousePoint)) {
        return true;
    }
    
    return false;
}

void BoundedScrollLayer::scrollWheel(float y, float x)  {
    if (!m_doScroll || !testLocation(getMousePos())) return;

    ScrollLayer::scrollWheel(y, x);
}

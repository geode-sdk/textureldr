#include <Geode/modify/CCSprite.hpp>
#include <Geode/modify/CCSpriteFrameCache.hpp>

using namespace geode::prelude;

class $modify(CCSprite) {
    static CCSprite* create(const char* name) {
        auto* sprite = CCSprite::create(name);
        if (sprite == nullptr) {
            sprite = CCSprite::create("fallback.png"_spr);
        }
        // in dire cases, since no one is stupid enough to delete this texture
        if (sprite == nullptr) {
            sprite = CCSprite::create("bigFont.png");
        }
        return sprite;
    }
    static CCSprite* createWithSpriteFrameName(const char* name) {
        auto* sprite = CCSprite::createWithSpriteFrameName(name);
        if (sprite == nullptr) {
            sprite = CCSprite::create("fallback.png"_spr);
        }
        if (sprite == nullptr) {
            sprite = CCSprite::create("bigFont.png");
        }
        return sprite;
    }

    bool initWithSpriteFrame(CCSpriteFrame* frame) {
        if (frame == nullptr) {
            return CCSprite::initWithFile("fallback.png"_spr);
        }
        return CCSprite::initWithSpriteFrame(frame);
    }
};

class $modify(CCSpriteFrameCache) {
    cocos2d::CCSpriteFrame* spriteFrameByName(char const* name) {
        auto* frame = CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName(name);
        if (frame == nullptr) {
            frame = CCSpriteFrame::create("fallback.png"_spr, {ccp(0, 0), ccp(128, 128)});
        }
        return frame;
    }
};
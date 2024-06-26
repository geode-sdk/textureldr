#include <Geode/modify/CCSprite.hpp>
#include <Geode/modify/CCSpriteFrameCache.hpp>

using namespace geode::prelude;

void assignFallbackObj(CCNode* node) {
    if (!node) return;
    node->setUserObject("fallback"_spr, CCBool::create(true));
}

class $modify(CCSprite) {
    static CCSprite* create(const char* name) {
        auto* sprite = CCSprite::create(name);
        if (sprite == nullptr) {
            sprite = CCSprite::create("fallback.png"_spr);
            // in dire cases, since no one is stupid enough to delete this texture
            if (sprite == nullptr) {
                sprite = CCSprite::create("bigFont.png");
            }
            assignFallbackObj(sprite);
        }
        return sprite;
    }
    static CCSprite* createWithSpriteFrameName(const char* name) {
        auto* sprite = CCSprite::createWithSpriteFrameName(name);
        if (sprite == nullptr) {
            sprite = CCSprite::create("fallback.png"_spr);
            if (sprite == nullptr) {
                sprite = CCSprite::create("bigFont.png");
            }
            assignFallbackObj(sprite);
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
            // this is stupid but rob intentionally doesnt load all icons at startup,
            // probably to save memory, so do this to not use fallback on icons
            static constexpr std::string_view prefixes[] = {
                "player_",
                "ship_",
                "dart_",
                "bird_",
                "robot_",
                "spider_",
                "swing_",
                "jetpack_",
            };
            const std::string_view nameStr = name;
            for (auto const& prefix : prefixes) {
                if (nameStr.find(prefix) != -1) {
                    return frame;
                }
            }
            frame = CCSpriteFrame::create("fallback.png"_spr, {ccp(0, 0), ccp(128, 128)});
        }
        return frame;
    }
};
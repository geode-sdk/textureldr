#include <Geode/modify/CCSprite.hpp>
#include <Geode/modify/CCSpriteFrameCache.hpp>

using namespace geode::prelude;
void assignFallbackObj(CCNode* node) {
    if (!node) return;
    node->setUserObject("fallback"_spr, CCBool::create(true));
}

CCSprite* fallback(auto name) {
    const char * modpath = ""_spr;
    const char * layout = fmt::format("{}{}",modpath,name).c_str();
    log::debug("texture replaced: {}", layout);
    auto* sprite = CCSprite::create(layout);
    if (sprite == nullptr) {
          sprite = CCSprite::create("fallback.png"_spr);
            if (sprite == nullptr) {
                sprite = CCSprite::create("bigFont.png");
            }
            assignFallbackObj(sprite);
    }
    return sprite;
};


class $modify(CCSprite) {
    static CCSprite* create(const char* name) {
        auto* sprite = CCSprite::create(name);
        if (sprite == nullptr) {
            sprite = fallback(name);
        }
        return sprite;
    }
    static CCSprite* createWithSpriteFrameName(const char* name) {
        auto* sprite = CCSprite::createWithSpriteFrameName(name);
        if (sprite == nullptr) {
            sprite = fallback(name);
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
            const char * modpath = ""_spr;
            const char * layout = fmt::format("{}{}",modpath,name).c_str();
            frame = CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName(layout);
            if (frame == nullptr) {
                frame = CCSpriteFrame::create("fallback.png"_spr, {ccp(0, 0), ccp(128, 128)});
            }
        }
        return frame;
    }
};
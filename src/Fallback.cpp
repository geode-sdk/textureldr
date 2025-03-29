#include <Geode/modify/CCSprite.hpp>
#include <Geode/modify/CCSpriteFrameCache.hpp>

using namespace geode::prelude;

static constexpr int FALLBACK_TAG = 105871529;

static void assignFallbackObj(CCNode* node) {
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
        auto* spriteFrame = CCSpriteFrameCache::get()->spriteFrameByName(name);

        // we check for tag instead of the frame name because this is significantly better for performance
        bool needFallback = !spriteFrame || spriteFrame->getTag() == FALLBACK_TAG;

        if (!needFallback) {
            return CCSprite::createWithSpriteFrame(spriteFrame);
        }

        CCSprite* sprite = CCSprite::create("fallback.png"_spr);
        if (sprite == nullptr) {
            sprite = CCSprite::create("bigFont.png");
        }
        assignFallbackObj(sprite);
        return sprite;
    }

    bool initWithSpriteFrame(CCSpriteFrame* frame) {
        if (frame == nullptr) {
            bool result = CCSprite::initWithFile("fallback.png"_spr);
            if (result) {
                assignFallbackObj(this);
            }

            return result;
        }
        return CCSprite::initWithSpriteFrame(frame);
    }
};

class $modify(CCSpriteFrameCache) {
    CCSpriteFrame* spriteFrameByName(char const* name) {
        auto* frame = CCSpriteFrameCache::spriteFrameByName(name);

        if (frame != nullptr) {
            return frame;
        }

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

        // check if the fallback was already added
        auto fallbackFrame = CCSpriteFrameCache::spriteFrameByName("fallback.png"_spr);
        if (fallbackFrame) {
            return fallbackFrame;
        }

        // create the fallback frame and add to cache
        fallbackFrame = CCSpriteFrame::create("fallback.png"_spr, {ccp(0, 0), ccp(128, 128)});

        if (fallbackFrame) {
            fallbackFrame->setTag(FALLBACK_TAG);
            this->addSpriteFrame(fallbackFrame, "fallback.png"_spr);
        }

        return fallbackFrame;
    }
};
#include <Geode/modify/CCSprite.hpp>
#include <Geode/modify/CCSpriteFrameCache.hpp>

using namespace geode::prelude;

static constexpr int FALLBACK_TAG = 105871529;

static void assignFallbackObj(CCNode* node) {
    if (!node) return;
    node->setUserObject("fallback"_spr, CCBool::create(true));
}

static CCTexture2D* generateFallback() {
    auto* image = new CCImage();
    if (!image->initWithImageFile("fallback.png"_spr)) {
        image->release();
        return nullptr;
    }

    auto* texture = new CCTexture2D();
    auto result = texture->initWithImage(image);
    image->release();
    if (!result) {
        texture->release();
        return nullptr;
    }

    texture->autorelease();
    return texture;
}

class $modify(CCSprite) {
    static CCSprite* create(const char* name) {
        auto* sprite = CCSprite::create(name);
        if (sprite == nullptr) {
            auto textureCache = CCTextureCache::get();
            auto fallbackTexture = static_cast<CCTexture2D*>(textureCache->m_pTextures->objectForKey(name));
            if (!fallbackTexture) {
                fallbackTexture = generateFallback();
                textureCache->m_pTextures->setObject(fallbackTexture, name);
            }

            sprite = CCSprite::createWithTexture(fallbackTexture);
            assignFallbackObj(sprite);
        }
        return sprite;
    }

    static CCSprite* createWithSpriteFrameName(const char* name) {
        auto* spriteFrame = CCSpriteFrameCache::get()->spriteFrameByName(name);

        // we check for tag instead of the frame name because this is significantly better for performance
        bool needFallback = !spriteFrame || spriteFrame->getTag() == FALLBACK_TAG;

        CCSprite* sprite = CCSprite::createWithSpriteFrame(spriteFrame);
        if (needFallback) {
            assignFallbackObj(sprite);
        }
        return sprite;
    }

    bool initWithSpriteFrame(CCSpriteFrame* frame) {
        if (frame == nullptr) {
            bool result = CCSprite::initWithSpriteFrame(CCSpriteFrameCache::get()->spriteFrameByName("fallback.png"_spr));
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
        auto fallbackFrame = CCSpriteFrameCache::spriteFrameByName(name);
        if (fallbackFrame) {
            return fallbackFrame;
        }

        // create the fallback frame and add to cache
        fallbackFrame = CCSpriteFrame::createWithTexture(generateFallback(), {ccp(0, 0), ccp(32, 32)});
        fallbackFrame->setTag(FALLBACK_TAG);
        this->addSpriteFrame(fallbackFrame, name);

        return fallbackFrame;
    }
};
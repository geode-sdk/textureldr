#include <Geode/modify/CCSprite.hpp>

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
};
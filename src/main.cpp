#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include "PackSelectPopup.hpp"
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/VideoOptionsLayer.hpp>
#include <Geode/modify/OptionsLayer.hpp>
#include <Geode/modify/IDManager.hpp>
#include <Geode/ui/BasedButtonSprite.hpp>

using namespace geode::prelude;

// Mobile support
class $modify(MyOptionsLayer, OptionsLayer) {
    static void onModify(auto& self) {
        std::ignore = self.setHookPriority("OptionsLayer::customSetup", Priority::LatePost);
    }

    void customSetup() {
        OptionsLayer::customSetup();
        if (CCNode* optionsMenu = m_mainLayer->getChildByID("options-menu")) {
            if (optionsMenu->getChildByID("graphics-button")) return;

            ButtonSprite* buttonSprite = ButtonSprite::create("Textures", 130, true, "goldFont.fnt", "GJ_button_01.png", 32, 1);

            CCMenuItemSpriteExtra* textureLoaderBtn = CCMenuItemSpriteExtra::create(buttonSprite, this, menu_selector(MyOptionsLayer::onTextureLdr));

            if (CCNode* optionsButton = optionsMenu->getChildByID("options-button")) {
                optionsMenu->insertAfter(textureLoaderBtn, optionsButton);
            }

            optionsMenu->updateLayout();
        }
    }

    void onTextureLdr(CCObject*) {
        PackSelectPopup::create()->show();
    }
};


class $modify(MyVideoOptionsLayer, VideoOptionsLayer) {
    bool init() {
        if (!VideoOptionsLayer::init()) return false;

        for (CCNode* child : CCArrayExt<CCNode*>(m_buttonMenu->getChildren())) {
            if (CCMenuItemSpriteExtra* btn = typeinfo_cast<CCMenuItemSpriteExtra*>(child)) {
                if (ButtonSprite* btnSpr = btn->getChildByType<ButtonSprite*>(0)) {
                    if (std::string_view(btnSpr->m_label->getString()) == "Advanced") {
                        ButtonSprite* buttonSprite = ButtonSprite::create("Textures", 60, true, "goldFont.fnt", "GJ_button_04.png", 25, 0.5f);

                        CCMenuItemSpriteExtra* textureLoaderBtn = CCMenuItemSpriteExtra::create(buttonSprite, this, menu_selector(MyVideoOptionsLayer::onTextureLdr));
                        textureLoaderBtn->setID("texture-loader-button"_spr);

                        textureLoaderBtn->setPositionX(btn->getPositionX());
                        textureLoaderBtn->setPositionY(btn->getPositionY() - 30);

                        m_buttonMenu->addChild(textureLoaderBtn);
                    }
                }
            }
        }
        return true;
    }

    void onTextureLdr(CCObject*) {
        PackSelectPopup::create()->show();
    }
};

class $modify(MyMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init())
            return false;

        if (Mod::get()->getSavedValue<bool>("shown-moved-alert")) return true;

        NodeIDs::provideFor(this);

        auto menu = this->getChildByID("right-side-menu");

        auto button = CCMenuItemSpriteExtra::create(
            CCSprite::createWithSpriteFrameName("gj_folderBtn_001.png"),
            this, menu_selector(MyMenuLayer::onTextureLdr)
        );
        button->setID("texture-loader-button"_spr);
        menu->addChild(button);
        menu->updateLayout();

        return true;
    }

    void onTextureLdr(CCObject*) {
        bool isDesktop = false;
        #ifdef GEODE_IS_DESKTOP
        isDesktop = true;
        #endif
        // Prompt where the button is relocated, since many mobile users use the high graphics mod, we want to direct them to the right place still.
        if (isDesktop || Loader::get()->isModLoaded("weebify.high-graphics-android")) {
            createQuickPopup("Woah!", "Texture Loader has been moved into <cb>Graphics Settings</c>, go to graphics settings and click the <cg>Textures</c> button to manage your Texture Packs!", "I UNDERSTAND", nullptr, [] (FLAlertLayer*, bool selected) {
                Mod::get()->setSavedValue("shown-moved-alert", true);
            }, true);
        }
        else {
            createQuickPopup("Woah!", "Texture Loader has been moved into <cb>Settings</c>, go to settings and click the <cg>Textures</c> button to manage your Texture Packs!", "I UNDERSTAND", nullptr, [] (FLAlertLayer*, bool selected) {
                Mod::get()->setSavedValue("shown-moved-alert", true);
            }, true);
        }
    }
};


#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include "PackSelectPopup.hpp"
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/OptionsLayer.hpp>
#include <Geode/modify/IDManager.hpp>
#include <Geode/ui/BasedButtonSprite.hpp>

using namespace geode::prelude;

class $modify(MyOptionsLayer, OptionsLayer) {

    void customSetup() {
        OptionsLayer::customSetup();
        if (CCNode* optionsMenu = m_mainLayer->getChildByID("options-menu")) {
            if (AxisLayout* layout = typeinfo_cast<AxisLayout*>(optionsMenu->getLayout())) {
                layout->ignoreInvisibleChildren(true);
            }
            
            ButtonSprite* buttonSprite = ButtonSprite::create("Textures", 130, true, "goldFont.fnt", "GJ_button_01.png", 30, 1);

            CCMenuItemSpriteExtra* textureLoaderBtn = CCMenuItemSpriteExtra::create(buttonSprite, this, menu_selector(MyOptionsLayer::onTextureLdr));

            if (CCNode* howToPlayBtn = optionsMenu->getChildByID("how-to-play-button")) {
                howToPlayBtn->setVisible(false);
                optionsMenu->insertBefore(textureLoaderBtn, howToPlayBtn);
            }

            optionsMenu->updateLayout();
        }
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
        createQuickPopup("Woah!", "Texture Loader has been moved into <cb>Settings</c>, go to settings and click the <cg>Textures</c> button to manage your Texture Packs!", "I UNDERSTAND", nullptr, [] (FLAlertLayer*, bool selected) {
            Mod::get()->setSavedValue("shown-moved-alert", true);
        }, true);
    }
};


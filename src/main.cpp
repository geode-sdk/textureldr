#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include "PackSelectPopup.hpp"
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


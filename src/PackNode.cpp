#include "PackNode.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemToggler.hpp>
#include "PackManager.hpp"
#include "PackSelectLayer.hpp"
#include "PackInfoPopup.hpp"

bool PackNode::init(
    PackSelectLayer* layer,
    std::shared_ptr<Pack> pack,
    float width
) {
    if (!CCNode::init())
        return false;

    constexpr float HEIGHT = 35.f;
    constexpr float SPACE_FOR_MENU = 50.f;
    constexpr float MOVE_OFFSET = 20.f;
    constexpr float SPACE_FOR_LOGO = HEIGHT;
    constexpr float PADDING = 5.f;

    float labelWidth = width - SPACE_FOR_MENU - SPACE_FOR_LOGO;
    float menuPosX = width - SPACE_FOR_MENU + MOVE_OFFSET;
    
    m_pack = pack;
    m_layer = layer;

    this->setID("PackNode");

    this->setContentSize({ width, HEIGHT });

    auto menu = CCMenu::create();
    menu->setPosition(menuPosX, HEIGHT / 2);
    menu->setID("pack-button-menu");
    
    auto logo = CCSprite::createWithSpriteFrameName("geode.loader/no-logo.png");
    logo->setPosition({ SPACE_FOR_LOGO / 2 + PADDING, HEIGHT / 2 });
    limitNodeSize(logo, { HEIGHT - PADDING * 2, HEIGHT - PADDING * 2 }, 1.f, .1f);
    this->addChild(logo);

    auto nameLabel = CCLabelBMFont::create(
        m_pack->getDisplayName().c_str(), "bigFont.fnt"
    );
    nameLabel->limitLabelWidth(labelWidth, .65f, .1f);

    auto nameButton = CCMenuItemSpriteExtra::create(
        nameLabel, this, menu_selector(PackNode::onView)
    );
    nameButton->setPosition(
        PADDING + SPACE_FOR_LOGO + nameLabel->getScaledContentSize().width / 2 - menuPosX,
        0
    );
    nameButton->setID("pack-name-button");
    menu->addChild(nameButton);

    auto applyArrowSpr = CCSprite::createWithSpriteFrameName("navArrowBtn_001.png");
    applyArrowSpr->setScale(.45f);

    // add arrows to the left is the pack is applied
    if (PackManager::get()->isApplied(pack)) {
        applyArrowSpr->setFlipX(true);

        auto unapplyBtn = CCMenuItemSpriteExtra::create(
            applyArrowSpr, this, menu_selector(PackNode::onMoveToAvailable)
        );
        unapplyBtn->setID("unapply-pack-button");
        unapplyBtn->setPosition(-MOVE_OFFSET, 0.f);
        menu->addChild(unapplyBtn);

        nameButton->setPositionX(
            SPACE_FOR_LOGO + MOVE_OFFSET - PADDING +
            nameLabel->getScaledContentSize().width / 2
        );
        logo->setPositionX(SPACE_FOR_MENU + SPACE_FOR_LOGO / 2 - PADDING);
        menu->setPositionX(SPACE_FOR_MENU - MOVE_OFFSET);
    }
    // otherwise to the right
    else {
        auto applyBtn = CCMenuItemSpriteExtra::create(
            applyArrowSpr, this, menu_selector(PackNode::onMoveToApplied)
        );
        applyBtn->setID("apply-pack-button");
        applyBtn->setPosition(MOVE_OFFSET, 0.f);
        menu->addChild(applyBtn);
    }

    auto moveUpBtnSpr = CCSprite::createWithSpriteFrameName("navArrowBtn_001.png");
    moveUpBtnSpr->setRotation(-90.f);
    moveUpBtnSpr->setScale(.35f);

    auto moveUpBtn = CCMenuItemSpriteExtra::create(
        moveUpBtnSpr, this, menu_selector(PackNode::onMoveUp)
    );
    moveUpBtn->setID("move-pack-up-button");
    moveUpBtn->setPosition(0.f, 7.f);
    menu->addChild(moveUpBtn);

    auto moveDownBtnSpr = CCSprite::createWithSpriteFrameName("navArrowBtn_001.png");
    moveDownBtnSpr->setRotation(90.f);
    moveDownBtnSpr->setFlipY(true);
    moveDownBtnSpr->setScale(.35f);

    auto moveDownBtn = CCMenuItemSpriteExtra::create(
        moveDownBtnSpr, this, menu_selector(PackNode::onMoveDown)
    );
    moveDownBtn->setID("move-pack-down-button");
    moveDownBtn->setPosition(0.f, -7.f);
    menu->addChild(moveDownBtn);

    this->addChild(menu);

    return true;
}

void PackNode::onMoveUp(CCObject*) {
    PackManager::get()->movePackUp(m_pack);
    m_layer->updateLists();
}

void PackNode::onMoveDown(CCObject*) {
    PackManager::get()->movePackDown(m_pack);
    m_layer->updateLists();
}

void PackNode::onMoveToApplied(CCObject*) {
    PackManager::get()->moveToApplied(m_pack);
    m_layer->updateLists();
}

void PackNode::onMoveToAvailable(CCObject*) {
    PackManager::get()->moveToAvailable(m_pack);
    m_layer->updateLists();
}

void PackNode::onView(CCObject*) {
    PackInfoPopup::create(m_pack)->show();
}

PackNode* PackNode::create(
    PackSelectLayer* layer,
    std::shared_ptr<Pack> pack,
    float width
) {
    auto ret = new PackNode;
    if (ret && ret->init(layer, pack, width)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

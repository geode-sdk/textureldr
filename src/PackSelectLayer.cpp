#include "PackSelectLayer.hpp"
#include <Geode/ui/General.hpp>
#include <Geode/ui/ListView.hpp>
#include <Geode/utils/cocos.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/MenuLayer.hpp>
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/modify/CCScrollLayerExt.hpp>
#include "PackManager.hpp"
#include "PackNode.hpp"

static CCSize LAYER_SIZE = { 230.f, 190.f };

bool PackSelectLayer::init() {
    if (!CCLayer::init())
        return false;
    
    this->addChild(createLayerBG());

    auto winSize = CCDirector::get()->getWinSize();

    auto title = CCLabelBMFont::create("Texture Packs", "goldFont.fnt");
    title->setPosition(winSize.width / 2, winSize.height - 20.f);
    this->addChild(title);

    auto menu = CCMenu::create();
    menu->setZOrder(10);

    auto backBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png"),
        this, menu_selector(PackSelectLayer::onGoBack)
    );
    backBtn->setPosition(-winSize.width / 2 + 25.f, winSize.height / 2 - 25.f);
    menu->addChild(backBtn);

    auto applyBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Apply", "goldFont.fnt", "GJ_button_01.png", .8f),
        this, menu_selector(PackSelectLayer::onApply)
    );
    applyBtn->setPosition(0.f, -winSize.height / 2 + 25.f);
    menu->addChild(applyBtn);

    this->addChild(menu);

    // available packs list

    auto availableTitle = CCLabelBMFont::create("Available", "goldFont.fnt");
    availableTitle->setPosition(
        winSize / 2 + CCPoint { -120, LAYER_SIZE.height / 2 + 15.f }
    );
    availableTitle->setScale(.65f);
    this->addChild(availableTitle);

    auto availableListBG = CCScale9Sprite::create(
        "square02b_001.png", { 0, 0, 80, 80 }
    );
    availableListBG->setColor({ 0, 0, 0 });
    availableListBG->setOpacity(90);
    availableListBG->setContentSize(LAYER_SIZE);
    availableListBG->setPosition(winSize / 2 + CCPoint { -120, 0 });
    this->addChild(availableListBG);

    m_availableList = ScrollLayer::create(LAYER_SIZE);
    m_availableList->setPosition(
        winSize / 2 + CCPoint { -120, 0 } - LAYER_SIZE / 2
    );
    this->addChild(m_availableList);

    // applied packs list

    auto appliedTitle = CCLabelBMFont::create("Applied", "goldFont.fnt");
    appliedTitle->setPosition(
        winSize / 2 + CCPoint { 120, LAYER_SIZE.height / 2 + 15.f }
    );
    appliedTitle->setScale(.65f);
    this->addChild(appliedTitle);

    auto appliedListBG = CCScale9Sprite::create(
        "square02b_001.png", { 0, 0, 80, 80 }
    );
    appliedListBG->setColor({ 0, 0, 0 });
    appliedListBG->setOpacity(90);
    appliedListBG->setContentSize(LAYER_SIZE);
    appliedListBG->setPosition(winSize / 2 + CCPoint { 120, 0 });
    this->addChild(appliedListBG);

    m_appliedList = ScrollLayer::create(LAYER_SIZE);
    m_appliedList->setPosition(
        winSize / 2 + CCPoint { 120, 0 } - LAYER_SIZE / 2
    );
    this->addChild(m_appliedList);

    this->updateLists();
    
    return true;
}

void PackSelectLayer::updateList(
    ScrollLayer* list,
    std::vector<std::shared_ptr<Pack>> const& packs
) {
    list->m_contentLayer->removeAllChildren();
    float totalHeight = .0f;
    std::vector<PackNode*> rendered;
    for (auto& pack : packs) {
        auto node = PackNode::create(this, pack, LAYER_SIZE.width);
        totalHeight += node->getScaledContentSize().height;
        node->setPosition(0.f, -totalHeight);
        list->m_contentLayer->addChild(node);

        rendered.push_back(node);
    }
    if (totalHeight < LAYER_SIZE.height) {
        totalHeight = LAYER_SIZE.height;
    }
    for (auto& node : rendered) {
        node->setPositionY(node->getPositionY() + totalHeight);
    }
    list->m_contentLayer->setContentSize({ LAYER_SIZE.width, totalHeight });
    list->moveToTop();
}

void PackSelectLayer::updateLists() {
    this->updateList(m_availableList, PackManager::get()->getAvailablePacks());
    this->updateList(m_appliedList, PackManager::get()->getAppliedPacks());
}

void PackSelectLayer::keyBackClicked() {
    this->onGoBack(nullptr);
}

void PackSelectLayer::onGoBack(CCObject*) {
    CCDirector::get()->replaceScene(MenuLayer::scene(false));
}

void PackSelectLayer::onApply(CCObject*) {
    PackManager::get()->applyPacks(+[]() -> CCLayer* {
        return PackSelectLayer::create();
    });
}

PackSelectLayer* PackSelectLayer::create() {
    auto ret = new PackSelectLayer;
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

PackSelectLayer* PackSelectLayer::scene() {
    auto layer = PackSelectLayer::create();
    switchToScene(layer);
    return layer;
}

#include "PackSelectLayer.hpp"
#include <Geode/ui/General.hpp>
#include <Geode/ui/Notification.hpp>
#include <Geode/utils/cocos.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/MenuLayer.hpp>
#include <Geode/binding/ButtonSprite.hpp>
#include "PackManager.hpp"
#include "PackNode.hpp"

static CCSize LAYER_SIZE = { 230.f, 190.f };

bool PackSelectLayer::init() {
    if (!CCLayer::init())
        return false;

    GameManager::get()->fadeInMenuMusic();
    
    this->setID("PackSelectLayer");

    this->setKeypadEnabled(true);

    auto background = createLayerBG();
    background->setID("background");
    background->setColor({255, 255, 255});
    this->addChild(background);

    auto winSize = CCDirector::get()->getWinSize();

    auto title = CCLabelBMFont::create("Texture Packs", "goldFont.fnt");
    title->setPosition(winSize.width / 2, winSize.height - 20.f);
    title->setID("texture-packs-text");
    this->addChild(title);

    auto menu = CCMenu::create();
    menu->setID("menu");
    menu->setZOrder(10);

    auto backBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png"),
        this, menu_selector(PackSelectLayer::onGoBack)
    );
    backBtn->setPosition(-winSize.width / 2 + 25.f, winSize.height / 2 - 25.f);
    backBtn->setID("back-button");
    menu->addChild(backBtn);

    auto applyBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Apply", "goldFont.fnt", "GJ_button_01.png", .8f),
        this, menu_selector(PackSelectLayer::onApply)
    );
    applyBtn->setPosition(0.f, -winSize.height / 2 + 25.f);
    applyBtn->setID("apply-button");
    menu->addChild(applyBtn);

    auto folderBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("gj_folderBtn_001.png"),
        this, menu_selector(PackSelectLayer::onOpenFolder)
    );
    folderBtn->setPosition(winSize.width / 2.f - 25.f, -winSize.height / 2.f + 25.f);
    folderBtn->setID("folder-button");
    menu->addChild(folderBtn);

    auto reloadSpr = CCSprite::createWithSpriteFrameName("GJ_updateBtn_001.png");
    reloadSpr->setScale(.8f);
    auto reloadBtn = CCMenuItemSpriteExtra::create(reloadSpr, this, menu_selector(PackSelectLayer::onReloadPacks));
    reloadBtn->setPosition(-winSize.width / 2.f + 25.f, -winSize.height / 2.f + 25.f);
    reloadBtn->setID("reload-button");
    menu->addChild(reloadBtn);

    this->addChild(menu);

    // available packs list

    auto availableTitle = CCLabelBMFont::create("Available", "goldFont.fnt");
    availableTitle->setPosition(
        winSize / 2 + CCPoint { -120, LAYER_SIZE.height / 2 + 15.f }
    );
    availableTitle->setScale(.65f);
    availableTitle->setID("available-text");
    this->addChild(availableTitle);

    auto availableListBG = CCScale9Sprite::create(
        "square02b_001.png", { 0, 0, 80, 80 }
    );
    availableListBG->setColor({ 0, 0, 0 });
    availableListBG->setOpacity(90);
    availableListBG->setContentSize(LAYER_SIZE);
    availableListBG->setPosition(winSize / 2 + CCPoint { -120, 0 });
    availableListBG->setID("available-list-background");
    this->addChild(availableListBG);

    m_availableList = ScrollLayer::create(LAYER_SIZE);
    m_availableList->setPosition(
        winSize / 2 + CCPoint { -120, 0 } - LAYER_SIZE / 2
    );
    m_availableList->setID("available-list");
    this->addChild(m_availableList);

    // applied packs list

    auto appliedTitle = CCLabelBMFont::create("Applied", "goldFont.fnt");
    appliedTitle->setPosition(
        winSize / 2 + CCPoint { 120, LAYER_SIZE.height / 2 + 15.f }
    );
    appliedTitle->setScale(.65f);
    appliedTitle->setID("applied-text");
    this->addChild(appliedTitle);

    auto appliedListBG = CCScale9Sprite::create(
        "square02b_001.png", { 0, 0, 80, 80 }
    );
    appliedListBG->setColor({ 0, 0, 0 });
    appliedListBG->setOpacity(90);
    appliedListBG->setContentSize(LAYER_SIZE);
    appliedListBG->setPosition(winSize / 2 + CCPoint { 120, 0 });
    appliedListBG->setID("applied-list-background");
    this->addChild(appliedListBG);

    m_appliedList = ScrollLayer::create(LAYER_SIZE);
    m_appliedList->setPosition(
        winSize / 2 + CCPoint { 120, 0 } - LAYER_SIZE / 2
    );
    m_appliedList->setID("applied-list");
    this->addChild(m_appliedList);

    this->updateLists();
    
    return true;
}

void PackSelectLayer::updateList(
    ScrollLayer* list,
    std::vector<std::shared_ptr<Pack>> const& packs,
    bool resetPos
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
    if (resetPos) {
        list->moveToTop();
    }
}

void PackSelectLayer::updateLists(bool resetPos) {
    this->updateList(m_availableList, PackManager::get()->getAvailablePacks(), resetPos);
    this->updateList(m_appliedList, PackManager::get()->getAppliedPacks(), resetPos);
}

void PackSelectLayer::keyBackClicked() {
    this->onGoBack(nullptr);
}

void PackSelectLayer::onGoBack(CCObject*) {
    CCDirector::get()->replaceScene(CCTransitionFade::create(0.5, MenuLayer::scene(false)));
}

void PackSelectLayer::onApply(CCObject*) {
    PackManager::get()->applyPacks(+[]() -> CCLayer* {
        return PackSelectLayer::create();
    });
}

void PackSelectLayer::onOpenFolder(CCObject*) {
    utils::file::openFolder(PackManager::get()->getPackDir());
}

void PackSelectLayer::onReloadPacks(CCObject*) {
    size_t count = PackManager::get()->loadPacks();
    this->updateLists();
    Notification::create(fmt::format("Loaded {} packs", count), NotificationIcon::Success, 0.5f)->show();
}

void PackSelectLayer::startDragging(PackNode* node) {
    m_draggingNode = node;
    m_lastDragIdx = -1;
    auto const pos = node->getParent()->convertToWorldSpace(node->getPosition());

    // remove from the scroll layer
    node->retain();
    node->removeFromParentAndCleanup(false);
    this->addChild(node);
    node->release();
    
    node->setPosition(this->convertToNodeSpace(pos));

    m_dragListFrom = this->whereDragList();
}

PackListType PackSelectLayer::whereDragList() {
    if (!m_draggingNode) return PackListType::Available;
    // it has the anchor point on bot left for some reason
    auto x = m_draggingNode->getPosition().x + m_draggingNode->getContentSize().width / 2.f;
    if (x > CCDirector::get()->getWinSize().width / 2.f) {
        return PackListType::Applied;
    } else {
        return PackListType::Available;
    }
}

void PackSelectLayer::moveDrag(const CCPoint& offset) {
    m_draggingNode->setPosition(m_draggingNode->getPosition() + offset);
    this->reorderDragging();
}

void PackSelectLayer::reorderDragging() {
    auto const listTypeTo = this->whereDragList();

    auto appliedList = std::make_pair(m_appliedList, PackManager::get()->getAppliedPacks());
    auto availableList = std::make_pair(m_availableList, PackManager::get()->getAvailablePacks());
    
    auto& listTo = listTypeTo == PackListType::Applied ? appliedList : availableList;
    auto& listFrom = listTypeTo != PackListType::Applied ? appliedList : availableList;

    const auto listTop = listTo.first->convertToWorldSpace(
        listTo.first->m_contentLayer->getPosition() + listTo.first->m_contentLayer->getContentSize()).y;
    const auto nodeY = m_draggingNode->getPosition().y + m_draggingNode->getScaledContentSize().height / 2.f;

    auto targetIdx = static_cast<size_t>(std::max((listTop - nodeY) / PackNode::HEIGHT, 0.f));

    if (targetIdx == m_lastDragIdx && listTypeTo == m_dragListTo) return;

    m_lastDragIdx = targetIdx;

    if (listTypeTo != m_dragListTo) {
        this->reorderList(listFrom.first, listFrom.second, -1);
    }
    this->reorderList(listTo.first, listTo.second, targetIdx);

    m_dragListTo = listTypeTo;
}

void PackSelectLayer::reorderList(ScrollLayer* list, std::vector<std::shared_ptr<Pack>> const& packs, size_t skipIdx) {
    const auto childForPack = [list] (const std::shared_ptr<Pack>& pack) -> PackNode* {
        for (auto* child : CCArrayExt<PackNode*>(list->m_contentLayer->getChildren())) {
            if (child->getPack() == pack) return child;
        }
        return nullptr;
    };

    auto const totalHeight = list->m_contentLayer->getContentSize().height;
    
    float y = totalHeight;
    int visualIdx = 0;
    for (const auto& pack : packs) {
        if (visualIdx == skipIdx) {
            y -= PackNode::HEIGHT;
            ++visualIdx;
        }

        if (pack == m_draggingNode->getPack()) {
            continue;
        }

        auto* child = childForPack(pack);

        y -= PackNode::HEIGHT;

        child->setPositionY(y);

        ++visualIdx;
    }
}

void PackSelectLayer::stopDrag() {
    PackManager::get()->movePackToIdx(m_draggingNode->getPack(), m_dragListTo, m_lastDragIdx);

    m_draggingNode->removeFromParent();

    m_draggingNode = nullptr;

    this->updateLists(false);

    if (m_dragListFrom != m_dragListTo) {
        auto* from = m_dragListFrom == PackListType::Applied ? m_appliedList : m_availableList;
        auto* to = m_dragListTo == PackListType::Applied ? m_appliedList : m_availableList;
        // scroll source list up
        if (from->m_contentLayer->getPositionY() < 0.f) {
            from->m_contentLayer->setPositionY(from->m_contentLayer->getPositionY() + PackNode::HEIGHT);
        }
        // scroll destination list down
        if (to->m_contentLayer->getPositionY() + to->m_contentLayer->getContentSize().height > to->getContentSize().height + 1.f) {
            to->m_contentLayer->setPositionY(to->m_contentLayer->getPositionY() - PackNode::HEIGHT);
        }
    }
}

PackSelectLayer* PackSelectLayer::create() {
    auto ret = new PackSelectLayer;
    if (ret->init()) {
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

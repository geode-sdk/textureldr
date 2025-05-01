#include "PackSelectPopup.hpp"
#include <Geode/ui/General.hpp>
#include <Geode/ui/Notification.hpp>
#include <Geode/utils/cocos.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/MenuLayer.hpp>
#include <Geode/binding/ButtonSprite.hpp>
#include "PackManager.hpp"
#include "PackNode.hpp"

static CCSize LAYER_SIZE = { 230.f, 210.f };

bool PackSelectPopup::init() {
    if (!Popup<>::initAnchored(440.f, 290.f, "GJ_square01.png")) return false;

    GameManager::get()->fadeInMenuMusic();
    
    this->setID("PackSelectPopup");

    this->setKeypadEnabled(true);

    auto size = m_mainLayer->getContentSize();

    auto title = CCLabelBMFont::create("Texture Packs", "goldFont.fnt");
    title->setPosition(size.width / 2, size.height - 20.f);
    title->setID("texture-packs-text");
    title->setScale(.8f);
    m_mainLayer->addChild(title);

    auto menu = CCMenu::create();
    menu->setID("menu");
    menu->ignoreAnchorPointForPosition(false);
    menu->setPosition({size.width/2, 0});
    menu->setAnchorPoint({0.5, 0});
    menu->setContentSize({size.width - 10, 50});
    menu->setZOrder(10);
    menu->setLayout(
        SimpleRowLayout::create()
            ->setMainAxisAlignment(MainAxisAlignment::Between)
    );

    auto reloadSpr = CCSprite::createWithSpriteFrameName("GJ_updateBtn_001.png");
    reloadSpr->setScale(.8f);
    auto reloadBtn = CCMenuItemSpriteExtra::create(reloadSpr, this, menu_selector(PackSelectPopup::onReloadPacks));
    reloadBtn->setID("reload-button");
    menu->addChild(reloadBtn);


    auto applyBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Apply", "goldFont.fnt", "GJ_button_01.png", .8f),
        this, menu_selector(PackSelectPopup::onApply)
    );
    applyBtn->setID("apply-button");
    menu->addChild(applyBtn);

    auto folderBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("gj_folderBtn_001.png"),
        this, menu_selector(PackSelectPopup::onOpenFolder)
    );
    folderBtn->setID("folder-button");
    menu->addChild(folderBtn);

    menu->updateLayout();
    m_mainLayer->addChild(menu);

    // available packs list

    float distanceFromCenter = 105.f;
    float heightOffset = -5.f;
    float scale = .88f;

    auto availableTitle = CCLabelBMFont::create("Available", "goldFont.fnt");
    availableTitle->setPosition(
        size / 2 + CCPoint { -distanceFromCenter, LAYER_SIZE.height / 2 + heightOffset }
    );
    availableTitle->setScale(.65f);
    availableTitle->setID("available-text");
    m_mainLayer->addChild(availableTitle);

    auto availableListBG = CCScale9Sprite::create(
        "square02b_001.png", { 0, 0, 80, 80 }
    );
    availableListBG->setColor({ 0, 0, 0 });
    availableListBG->setOpacity(90);
    availableListBG->setScale(scale);
    availableListBG->setContentSize(LAYER_SIZE);
    availableListBG->setPosition(size / 2 + CCPoint { -distanceFromCenter, heightOffset });
    availableListBG->setID("available-list-background");
    m_mainLayer->addChild(availableListBG);

    m_availableList = ScrollLayer::create(LAYER_SIZE * scale);
    m_availableList->setPosition(
        size / 2 + CCPoint { -distanceFromCenter, heightOffset } - LAYER_SIZE * scale / 2
    );
    m_availableList->setID("available-list");
    m_mainLayer->addChild(m_availableList);

    // applied packs list

    auto appliedTitle = CCLabelBMFont::create("Applied", "goldFont.fnt");
    appliedTitle->setPosition(
        size / 2 + CCPoint { distanceFromCenter, LAYER_SIZE.height / 2 + heightOffset }
    );
    appliedTitle->setScale(.65f);
    appliedTitle->setID("applied-text");
    m_mainLayer->addChild(appliedTitle);

    auto appliedListBG = CCScale9Sprite::create(
        "square02b_001.png", { 0, 0, 80, 80 }
    );
    appliedListBG->setColor({ 0, 0, 0 });
    appliedListBG->setOpacity(90);
    appliedListBG->setScale(scale);
    appliedListBG->setContentSize(LAYER_SIZE);
    appliedListBG->setPosition(size / 2 + CCPoint { distanceFromCenter, + heightOffset });
    appliedListBG->setID("applied-list-background");
    m_mainLayer->addChild(appliedListBG);

    m_appliedList = ScrollLayer::create(LAYER_SIZE * scale);
    m_appliedList->setPosition(
        size / 2 + CCPoint { distanceFromCenter, heightOffset } - LAYER_SIZE * scale / 2
    );
    m_appliedList->setID("applied-list");
    m_mainLayer->addChild(m_appliedList);

    this->updateLists();
    
    return true;
}

void PackSelectPopup::updateList(
    ScrollLayer* list,
    std::vector<std::shared_ptr<Pack>> const& packs,
    bool resetPos
) {
    list->m_contentLayer->removeAllChildren();
    float totalHeight = .0f;
    std::vector<PackNode*> rendered;
    for (auto& pack : packs) {
        auto node = PackNode::create(this, pack, LAYER_SIZE.width);
        node->setScale(.88f);
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

void PackSelectPopup::updateLists(bool resetPos) {
    this->updateList(m_availableList, PackManager::get()->getAvailablePacks(), resetPos);
    this->updateList(m_appliedList, PackManager::get()->getAppliedPacks(), resetPos);
}

class $modify(MyMenuLayer, MenuLayer) {

    struct Fields {
        bool m_openPackSelectPopup = false;
    };

    void openOptions(bool videoOptions) {
        auto fields = m_fields.self();
        if (fields->m_openPackSelectPopup) {
            MenuLayer::openOptions(false);
            PackSelectPopup::create()->show();
            fields->m_openPackSelectPopup = false;
            return;
        }
        MenuLayer::openOptions(videoOptions);
    }
};

void PackSelectPopup::onApply(CCObject*) {
    PackManager::get()->applyPacks(+[]() -> CCLayer* {
        CCScene* scene = MenuLayer::scene(true);
        MyMenuLayer* menuLayer = static_cast<MyMenuLayer*>(scene->getChildByType<MenuLayer*>(0));
        menuLayer->m_fields->m_openPackSelectPopup = true;
        return menuLayer;
    });
}

void PackSelectPopup::onOpenFolder(CCObject*) {
    utils::file::openFolder(PackManager::get()->getPackDir());
}

void PackSelectPopup::onReloadPacks(CCObject*) {
    size_t count = PackManager::get()->loadPacks();
    this->updateLists();
    Notification::create(fmt::format("Loaded {} packs", count), NotificationIcon::Success, 0.5f)->show();
}

void PackSelectPopup::startDragging(PackNode* node) {
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

PackListType PackSelectPopup::whereDragList() {
    if (!m_draggingNode) return PackListType::Available;
    // it has the anchor point on bot left for some reason
    auto x = m_draggingNode->getPosition().x + m_draggingNode->getScaledContentSize().width / 2.f;
    if (x > CCDirector::get()->getWinSize().width / 2.f) {
        return PackListType::Applied;
    } else {
        return PackListType::Available;
    }
}

void PackSelectPopup::moveDrag(const CCPoint& offset) {
    m_draggingNode->setPosition(m_draggingNode->getPosition() + offset);
    this->reorderDragging();
}

void PackSelectPopup::reorderDragging() {
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

void PackSelectPopup::reorderList(ScrollLayer* list, std::vector<std::shared_ptr<Pack>> const& packs, size_t skipIdx) {
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
            y -= PackNode::HEIGHT * .88f;
            ++visualIdx;
        }

        if (pack == m_draggingNode->getPack()) {
            continue;
        }

        auto* child = childForPack(pack);

        y -= PackNode::HEIGHT * .88f;

        child->setPositionY(y);

        ++visualIdx;
    }
}

void PackSelectPopup::stopDrag() {
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

PackSelectPopup* PackSelectPopup::create() {
    auto ret = new PackSelectPopup();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

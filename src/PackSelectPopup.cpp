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
#include "BoundedScrollLayer.hpp"

// go back to options after apply
class $modify(ReloadMenuLayer, MenuLayer) {

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

    int availCount = PackManager::get()->getAvailablePacks().size();
    int appliedCount = PackManager::get()->getAppliedPacks().size();

    m_infoLabel = CCLabelBMFont::create(fmt::format("Available: {}\nApplied: {}\nTotal: {}", availCount, appliedCount, availCount + appliedCount).c_str(), "chatFont.fnt");
    m_infoLabel->setPosition({70, 25});
    m_infoLabel->setID("info-text");
    m_infoLabel->setScale(.45f);
    m_infoLabel->setOpacity(200.f);
    m_mainLayer->addChild(m_infoLabel);

    auto reloadSpr = CCSprite::createWithSpriteFrameName("GJ_updateBtn_001.png");
    reloadSpr->setScale(.7f);
    auto reloadBtn = CCMenuItemSpriteExtra::create(reloadSpr, this, menu_selector(PackSelectPopup::onReloadPacks));
    reloadBtn->setID("reload-button");
    reloadBtn->setPosition({30, 25});

    m_buttonMenu->addChild(reloadBtn);
    auto applySpr = ButtonSprite::create("Apply", "goldFont.fnt", "GJ_button_01.png", .8f);
    applySpr->setScale(0.9f);
    auto applyBtn = CCMenuItemSpriteExtra::create(
        applySpr,
        this, menu_selector(PackSelectPopup::onApply)
    );
    applyBtn->setID("apply-button");
    applyBtn->setPosition({size.width/2, 25});
    m_buttonMenu->addChild(applyBtn);

    auto folderBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("gj_folderBtn_001.png"),
        this, menu_selector(PackSelectPopup::onOpenFolder)
    );
    folderBtn->setID("folder-button");
    folderBtn->setPosition({size.width - 30, 25});
    m_buttonMenu->addChild(folderBtn);

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

    m_availableList = BoundedScrollLayer::create(LAYER_SIZE * scale);
    m_availableList->m_contentLayer->setLayout(
        SimpleColumnLayout::create()
        ->setMainAxisDirection(AxisDirection::TopToBottom)
            ->setMainAxisAlignment(MainAxisAlignment::Start)
            ->setMainAxisScaling(AxisScaling::Grow)
    );
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

    m_appliedList = BoundedScrollLayer::create(LAYER_SIZE * scale);
    m_appliedList->m_contentLayer->setLayout(
        SimpleColumnLayout::create()
            ->setMainAxisDirection(AxisDirection::TopToBottom)
            ->setMainAxisAlignment(MainAxisAlignment::Start)
            ->setMainAxisScaling(AxisScaling::Grow)
    );
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
    for (auto& pack : packs) {
        auto node = PackNode::create(this, pack, LAYER_SIZE.width);
        node->setScale(.88f);
        list->m_contentLayer->addChild(node);
    }
    
    list->m_contentLayer->setContentHeight(list->getContentHeight());
    list->m_contentLayer->updateLayout();
    if (resetPos) {
        list->moveToTop();
    }

    int availCount = PackManager::get()->getAvailablePacks().size();
    int appliedCount = PackManager::get()->getAppliedPacks().size();
    m_infoLabel->setString(fmt::format("Available: {}\nApplied: {}\nTotal: {}", availCount, appliedCount, availCount + appliedCount).c_str());
}

void PackSelectPopup::updateLists(bool resetPos) {
    this->updateList(m_availableList, PackManager::get()->getAvailablePacks(), resetPos);
    this->updateList(m_appliedList, PackManager::get()->getAppliedPacks(), resetPos);
}

void PackSelectPopup::onApply(CCObject*) {
    PackManager::get()->applyPacks(+[]() -> CCLayer* {
        CCScene* scene = MenuLayer::scene(true);
        ReloadMenuLayer* menuLayer = static_cast<ReloadMenuLayer*>(scene->getChildByType<MenuLayer*>(0));
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

std::pair<PackListType, size_t> PackSelectPopup::getPackListTypeAndIndex(const std::shared_ptr<Pack>& pack) {
    auto manager = PackManager::get();
    const auto& applied = manager->getAppliedPacks();
    const auto& available = manager->getAvailablePacks();

    auto it = std::find(applied.begin(), applied.end(), pack);
    if (it != applied.end()) {
        return { PackListType::Applied, static_cast<size_t>(std::distance(applied.begin(), it)) };
    }

    it = std::find(available.begin(), available.end(), pack);
    return { PackListType::Available, static_cast<size_t>(std::distance(available.begin(), it)) };
}
void PackSelectPopup::startDragging(PackNode* node) {
    m_draggingNode = node;

    // set initial information, else clicking with no drag can cause it to move to available
    auto packData = getPackListTypeAndIndex(node->getPack());
    m_dragListTo = packData.first;
    m_lastDragIdx = packData.second;
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
    auto x = m_draggingNode->getPosition().x;
    if (x > getContentWidth() / 2.f) {
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
    const auto nodeY = m_draggingNode->getPosition().y;

    auto count = static_cast<float>(listTo.first->m_contentLayer->getChildrenCount());
    auto targetIdx = static_cast<size_t>(std::clamp((listTop - nodeY) / PackNode::HEIGHT, 0.f, count));

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
            y -= PackNode::HEIGHT;
            ++visualIdx;
        }

        if (pack == m_draggingNode->getPack()) {
            continue;
        }

        auto* child = childForPack(pack);

        y -= PackNode::HEIGHT;

        child->stopAllActions();
        child->runAction(CCEaseInOut::create(CCMoveTo::create(0.3, {child->getPositionX(), y + PackNode::HEIGHT / 2}), 2.f));

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

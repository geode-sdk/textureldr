#pragma once

#include <Geode/DefaultInclude.hpp>
#include <cocos2d.h>
#include <Geode/ui/ScrollLayer.hpp>
#include "Pack.hpp"
#include "PackManager.hpp"

class PackNode;

using namespace geode::prelude;

class PackSelectPopup : public Popup<> {
protected:
    ScrollLayer* m_availableList = nullptr;
    ScrollLayer* m_appliedList = nullptr;
    CCLabelBMFont* m_infoLabel = nullptr;
    PackNode* m_draggingNode = nullptr;
    size_t m_lastDragIdx = size_t(-1);
    PackListType m_dragListFrom, m_dragListTo;

    bool setup() override { return true; }
    bool init() override;

    void updateList(
        ScrollLayer* list,
        std::vector<std::shared_ptr<Pack>> const& packs,
        bool resetPos
    );

    void reorderDragging();
    void reorderList(
        ScrollLayer* list,
        std::vector<std::shared_ptr<Pack>> const& packs,
        size_t skipIdx
    );

    PackListType whereDragList();
    std::pair<PackListType, size_t> getPackListTypeAndIndex(const std::shared_ptr<Pack>& pack);

public:
    static PackSelectPopup* create();
    
    void updateLists(bool resetPos = true);

    void onApply(CCObject*);

    void onOpenFolder(CCObject*);
    void onReloadPacks(CCObject*);

    void startDragging(PackNode* node);
    void moveDrag(const CCPoint& offset);
    void stopDrag();
};
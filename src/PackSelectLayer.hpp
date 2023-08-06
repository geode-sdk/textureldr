#pragma once

#include <Geode/DefaultInclude.hpp>
#include <cocos2d.h>
#include <Geode/ui/ScrollLayer.hpp>
#include "Pack.hpp"
#include "PackManager.hpp"

class PackNode;

using namespace geode::prelude;

class PackSelectLayer : public CCLayer {
protected:
    ScrollLayer* m_availableList = nullptr;
    ScrollLayer* m_appliedList = nullptr;
    PackNode* m_draggingNode = nullptr;
    size_t m_lastDragIdx = size_t(-1);
    PackListType m_dragListFrom, m_dragListTo;

    bool init();

    void keyBackClicked() override;

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

public:
    static PackSelectLayer* create();
    static PackSelectLayer* scene();
    
    void updateLists(bool resetPos = true);

    void onGoBack(CCObject*);
    void onApply(CCObject*);

    void onOpenFolder(CCObject*);

    void startDragging(PackNode* node);
    void moveDrag(CCPoint offset);
    void stopDrag();
};

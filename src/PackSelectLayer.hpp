#pragma once

#include <Geode/DefaultInclude.hpp>
#include <cocos2d.h>
#include <Geode/ui/ScrollLayer.hpp>
#include "Pack.hpp"

using namespace geode::prelude;

class PackSelectLayer : public CCLayer {
protected:
    ScrollLayer* m_availableList = nullptr;
    ScrollLayer* m_appliedList = nullptr;

    bool init();

    void keyBackClicked() override;

    void updateList(
        ScrollLayer* list,
        std::vector<std::shared_ptr<Pack>> const& packs
    );

public:
    static PackSelectLayer* create();
    static PackSelectLayer* scene();
    
    void updateLists();

    void onGoBack(CCObject*);
    void onApply(CCObject*);
};

#include <Geode/Geode.hpp>
#include "Tutorial.hpp"

using namespace geode::prelude;

Task<void> awaitAction(CCNode* target, CCFiniteTimeAction* action) {
    auto [task, finish, _, isCancelled] = Task<void>::spawn();
    auto* func = CallFuncExt::create([finish = std::move(finish), isCancelled = std::move(isCancelled)] {
        if (!isCancelled()) {
            finish(true);
        }
    });
    auto* seq = CCSequence::createWithTwoActions(action, func);
    target->runAction(seq);
    return task;
}

template <class T>
class MyObjWrapper : public CCObject {
protected:
    T m_value;

    MyObjWrapper(T&& value) : m_value(std::forward<T>(value)) {
        this->autorelease();
    }
public:
    static MyObjWrapper* create(T&& value) {
        return new MyObjWrapper(std::forward<T>(value));
    }
    T& getValue() {
        return m_value;
    }
};

void playNewLocationTutorial() {
    auto* layer = MenuLayer::get();
    if (!layer) return;
    auto* button = layer->getChildByIDRecursive("texture-loader-button"_spr);
    if (!button) return;

    auto task = [](CCNode* button, CCLayer* layer) -> Task<void> {
        co_await awaitAction(button, CCEaseIn::create(CCFadeTo::create(0.5f, 0.f), 0.5f));
        button->setVisible(false);

        const auto showButton = [](CCMenuItemSpriteExtra* button) -> Task<void> {
            auto pos = button->getParent()->convertToWorldSpace(button->getPosition());
            int z = CCScene::get()->getHighestChildZ() + 10;

            Ref<CCSprite> arrow = CCSprite::createWithSpriteFrameName("navArrowBtn_001.png");
            arrow->setRotation(90);
            arrow->setPosition(pos + ccp(0, 20.f));
            arrow->runAction(CCRepeatForever::create(
                CCSequence::createWithTwoActions(
                    CCMoveBy::create(0.2f, ccp(0, 10.f)),
                    CCMoveBy::create(0.2f, ccp(0, -10.f))
                )
            ));
            arrow->runAction(CCSequence::createWithTwoActions(
                CCDelayTime::create(2.f),
                CCRemoveSelf::create()
            ));
            arrow->setZOrder(z + 10);
            CCScene::get()->addChild(arrow);

            // dont want the user pressing it before we do
            button->setEnabled(false);

            // two circles
            for (int i = 0; i < 2; ++i) {
                auto* circle = CCCircleWave::create(0.f, 100.f, 0.5f, false, true);
                circle->setPosition(pos);
                circle->setZOrder(z);
                CCScene::get()->addChild(circle);
                co_await awaitAction(button, CCDelayTime::create(0.5f));
            }

            button->setEnabled(true);
            button->selected();

            co_await awaitAction(button, CCDelayTime::create(0.2f));

            arrow->removeFromParentAndCleanup(true);
            button->activate();
            button->unselected();
        };

        auto* settingsBtn = typeinfo_cast<CCMenuItemSpriteExtra*>(layer->getChildByIDRecursive("settings-button"));
        if (!settingsBtn) co_return;
        co_await showButton(settingsBtn);

        co_await awaitAction(layer, CCDelayTime::create(0.6f));

        auto* optionsLayer = layer->getChildByType<OptionsLayer*>(0);
        // oh happy textures..
        if (!optionsLayer) optionsLayer = CCScene::get()->getChildByType<OptionsLayer*>(0);
        if (!optionsLayer) co_return;

        auto* texturesButton = typeinfo_cast<CCMenuItemSpriteExtra*>(optionsLayer->getChildByIDRecursive("texture-loader-button"_spr));

        // gotta go through the graphics button
        if (!texturesButton) {
            auto* graphicsBtn = typeinfo_cast<CCMenuItemSpriteExtra*>(optionsLayer->getChildByIDRecursive("graphics-button"));
            if (!graphicsBtn) co_return;
            co_await showButton(graphicsBtn);

            co_await awaitAction(layer, CCDelayTime::create(0.6f));

            auto* videoLayer = CCScene::get()->getChildByType<VideoOptionsLayer*>(0);
            if (!videoLayer) co_return;
            texturesButton = typeinfo_cast<CCMenuItemSpriteExtra*>(videoLayer->getChildByIDRecursive("texture-loader-button"_spr));
            if (!texturesButton) co_return;
        }

        co_await showButton(texturesButton);
    }(button, layer);

    // this would make an hjfod faint
    auto* obj = MyObjWrapper<EventListener<Task<void>>>::create({});
    obj->getValue().setFilter(task);
    layer->setUserObject("tutorial"_spr, obj);
}
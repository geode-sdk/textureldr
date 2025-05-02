#include "PackNode.hpp"
#include <Geode/binding/CCMenuItemToggler.hpp>
#include "PackManager.hpp"
#include "PackSelectPopup.hpp"
#include "PackInfoPopup.hpp"
#include "DragThingy.hpp"

bool PackNode::init(
    PackSelectPopup* layer,
    const std::shared_ptr<Pack>& pack,
    float width
) {
    if (!CCNode::init())
        return false;

    constexpr float HEIGHT = PackNode::HEIGHT / .88f;
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
    menu->setContentSize(this->getContentSize());
    menu->setPosition({0, 0});

    auto logo = CCSprite::create((pack->getResourcesPath() / "pack.png").string().c_str());

    if (!logo || logo->getUserObject("fallback"_spr)) {
        logo = CCSprite::create("noLogo.png"_spr);
        if (logo)
            logo->setOpacity(100);
    }
    if (logo) {
        logo->setPosition({ SPACE_FOR_LOGO / 2 + PADDING, HEIGHT / 2 });
        limitNodeSize(logo, { HEIGHT - PADDING * 2, HEIGHT - PADDING * 2 }, 1.f, .1f);
        this->addChild(logo);
    }

    auto nameLabel = CCLabelBMFont::create(
        m_pack->getDisplayName().c_str(), "bigFont.fnt"
    );
    nameLabel->limitLabelWidth(125.f, 0.40f, 0.1f);
    nameLabel->setPositionX(0);
    nameLabel->setAnchorPoint({0, 0.5f});
    nameLabel->setID("pack-name-text");

    auto nameButton = CCMenuItemSpriteExtra::create(
        nameLabel, this, menu_selector(PackNode::onView)
    );
    nameButton->setPosition(
        PADDING + SPACE_FOR_LOGO + nameLabel->getScaledContentSize().width / 2 - menuPosX,
        0
    );

    nameButton->setID("pack-name-button");
    nameButton->setContentWidth(nameLabel->getScaledContentWidth());
    nameButton->setEnabled(false);
    menu->addChild(nameButton);

    auto applyArrowSpr = CCSprite::create("dragIcon.png"_spr);
    applyArrowSpr->setScale(.6f);

    DragThingy* dragHandle = DragThingy::create(
        [=, this] {
            m_draggingBg->setVisible(true);
            m_layer->startDragging(this);
        },
        [=, this] (const CCPoint& offset) { m_layer->moveDrag(offset); },
        [=, this] {
            m_draggingBg->setVisible(false);
            m_layer->stopDrag();
        }
    );

    if (!m_pack->getInfo().has_value()) {
        nameButton->setPosition({40 + nameButton->getContentWidth()/2, this->getContentHeight()/2});
    }
    else {
        PackInfo packInfo = m_pack->getInfo().value();
        CCLabelBMFont* extraInfoLabel = CCLabelBMFont::create(fmt::format("{} | {}", packInfo.m_version.toNonVString(), packInfo.m_id).c_str(), "bigFont.fnt");
        extraInfoLabel->setColor({127, 127, 127});
        extraInfoLabel->setScale(0.15f);
        extraInfoLabel->setAnchorPoint({0, 0.5f});
        extraInfoLabel->setOpacity(127);
        extraInfoLabel->setPosition({40, 8});
        extraInfoLabel->setZOrder(-1);
        extraInfoLabel->setID("extra-info-text");

        this->addChild(extraInfoLabel);

        CCLabelBMFont* authorLabel = CCLabelBMFont::create(packInfo.m_authors.at(0).c_str(), "goldFont.fnt");
        authorLabel->setAnchorPoint({0, 0.5f});
        authorLabel->setPosition({40.2, 15});
        authorLabel->setZOrder(-1);
        authorLabel->setScale(0.25f);
        authorLabel->setID("author-text");
        this->addChild(authorLabel);

        nameButton->setPosition({40 + nameButton->getContentWidth()/2, this->getContentHeight()-10});
    }

    applyArrowSpr->setAnchorPoint(ccp(0, 0));
    dragHandle->addChild(applyArrowSpr);
    dragHandle->setContentSize(applyArrowSpr->getScaledContentSize());
    dragHandle->setID("apply-pack-button");
    dragHandle->setPosition(width - MOVE_OFFSET, HEIGHT / 2.f);
    dragHandle->setTouchPriority(-130);

    this->addChild(dragHandle);

    m_draggingBg = CCScale9Sprite::create(
        "square02b_001.png"
    );
    m_draggingBg->setCapInsets({10, 10, 50, 50});
    m_draggingBg->setColor({ 0, 0, 0 });
    m_draggingBg->setOpacity(90);
    m_draggingBg->setContentSize(this->getContentSize());
    m_draggingBg->setPosition({ width / 2.f, HEIGHT / 2.f });
    m_draggingBg->setZOrder(-10);
    m_draggingBg->setVisible(false);
    
    this->addChild(m_draggingBg);

    this->addChild(menu);

    return true;
}

void PackNode::onView(CCObject*) {
    PackInfoPopup::create(m_pack)->show();
}

PackNode* PackNode::create(
    PackSelectPopup* layer,
    const std::shared_ptr<Pack>& pack,
    float width
) {
    auto ret = new PackNode;
    if (ret->init(layer, pack, width)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

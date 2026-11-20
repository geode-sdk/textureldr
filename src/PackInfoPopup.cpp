#include "PackInfoPopup.hpp"
#include <Geode/loader/Loader.hpp>
#include <Geode/loader/Dirs.hpp>
#include <Geode/binding/GameManager.hpp>

class WackyBypassFont : public CCLabelBMFont {
protected:
    void setFntFile(asp::fs::Path fnt) {
        auto fntString = fnt.string();
        auto conf = FNTConfigLoadFile(fntString.c_str());
        m_sFntFile = fntString;
        if (!conf) {
            log::error("!conf ?????");
            return;
        }
        conf->retain();
        if (m_pConfiguration)
            m_pConfiguration->release();
        m_pConfiguration = conf;
        conf->m_sAtlasName = fnt.replaceExtension("png").string();

        this->setTexture(
            CCTextureCache::sharedTextureCache()->addImage(conf->getAtlasName(), false)
        );
        this->createFontChars();
    }

public:
    static CCLabelBMFont* create(
        const char* text,
        asp::fs::Path const& fnt
    ) {
        auto label = CCLabelBMFont::create();
        static_cast<WackyBypassFont*>(label)->setFntFile(fnt); // NOLINT(*-pro-type-static-cast-downcast)
        label->setString(text);
        return label;
    }
};

asp::fs::Path PackInfoPopup::getPathInPack(const char* filename) const {
    std::string suffix;
    switch (CCDirector::get()->getLoadedTextureQuality()) {
        case kTextureQualityHigh: {
            suffix = "-uhd";
        } break;

        case kTextureQualityMedium: {
            suffix = "-hd";
        } break;

        default: break;
    }

    auto fname = asp::fs::Path(filename);
    fname.replaceFilename(
        fmt::format("{}{}{}", fname.stem().value_or(""), suffix, fname.extension().value_or(""))
    );

    if ((m_pack->getResourcesPath() / fname).exists()) {
        return m_pack->getResourcesPath() / fname;
    } else {
        return dirs::getGameDir() / "Resources" / fname;
    }
}

bool PackInfoPopup::init(const std::shared_ptr<Pack>& pack) {
    if (!Popup::init(320.f, 200.f, this->getPathInPack("GJ_square01.png").string().c_str())) return false;

    m_pack = pack;

    auto title = WackyBypassFont::create(
        m_pack->getDisplayName().c_str(),
        this->getPathInPack("goldFont.fnt")
    );
    title->setPosition(
        m_size.width / 2,
        m_size.height / 2 + m_size.height / 2 - 30.f
    );
    m_mainLayer->addChild(title);

    // bigFont.fnt

    auto bigFontLabel = WackyBypassFont::create(
        "This is what a Label looks like!",
        this->getPathInPack("bigFont.fnt")
    );
    bigFontLabel->limitLabelWidth(m_size.width, .5f, .1f);
    bigFontLabel->setPosition(m_size / 2 + CCPoint { 0.f, 10.f });
    m_mainLayer->addChild(bigFontLabel);

    // GJ_button_01.png

    auto defaultBtnLabel = WackyBypassFont::create(
        "Button!",
        this->getPathInPack("goldFont.fnt")
    );
    defaultBtnLabel->setScale(.8f);

    auto defaultBtnSize =
        defaultBtnLabel->getScaledContentSize() + CCSize { 20.f, 15.f };

    auto defaultBtnSpr = NineSlice::create(
        this->getPathInPack("GJ_button_01.png").string(),
        { 0, 0, 40, 40 }
    );
    defaultBtnSpr->setContentSize(defaultBtnSize);
    defaultBtnSpr->setScale(.85f);
    defaultBtnSpr->addChild(defaultBtnLabel);

    defaultBtnLabel->setPosition(defaultBtnSize / 2);

    auto defaultBtn = CCMenuItemSpriteExtra::create(
        defaultBtnSpr, this, nullptr
    );
    defaultBtn->setPosition(m_size / 2 + CCPoint{-60.f, -50.f});
    m_buttonMenu->addChild(defaultBtn);

    // GJ_button_02.png

    auto altBtnLabel = WackyBypassFont::create(
        "Button!",
        this->getPathInPack("goldFont.fnt")
    );
    altBtnLabel->setScale(.8f);

    auto altBtnSize =
        altBtnLabel->getScaledContentSize() + CCSize { 20.f, 15.f };

    auto altBtnSpr = NineSlice::create(
        this->getPathInPack("GJ_button_02.png").string(),
        { 0, 0, 40, 40 }
    );
    altBtnSpr->setContentSize(altBtnSize);
    altBtnSpr->setScale(.85f);
    altBtnSpr->addChild(altBtnLabel);

    altBtnLabel->setPosition(altBtnSize / 2);

    auto altBtn = CCMenuItemSpriteExtra::create(
        altBtnSpr, this, nullptr
    );
    altBtn->setPosition(m_size / 2 + CCPoint{60.f, -50.f});
    m_buttonMenu->addChild(altBtn);

    return true;
}

PackInfoPopup* PackInfoPopup::create(const std::shared_ptr<Pack>& pack) {
    auto ret = new PackInfoPopup;
    if (ret->init(pack)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

#include "PackInfoPopup.hpp"
#include <asp/fs/std.hpp>
#include <Geode/loader/Loader.hpp>
#include <Geode/loader/Dirs.hpp>
#include <Geode/binding/GameManager.hpp>

class WackyBypassFont : public CCLabelBMFont {
protected:
    void setFntFile(std::filesystem::path fnt) {
        auto fntString = asp::fs::Path::fromStdPath(fnt).string();
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
        conf->m_sAtlasName = asp::fs::Path::fromStdPath(fnt.replace_extension("png")).string();

        this->setTexture(
            CCTextureCache::sharedTextureCache()->addImage(conf->getAtlasName(), false)
        );
        this->createFontChars();
    }

public:
    static CCLabelBMFont* create(
        const char* text,
        std::filesystem::path const& fnt
    ) {
        auto label = CCLabelBMFont::create();
        static_cast<WackyBypassFont*>(label)->setFntFile(fnt); // NOLINT(*-pro-type-static-cast-downcast)
        label->setString(text);
        return label;
    }
};

std::filesystem::path PackInfoPopup::getPathInPack(const char* filename) const {
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

    auto fname = std::filesystem::path(filename);
    fname.replace_filename(
        asp::fs::Path::fromStdPath(fname.stem()).string() + suffix + asp::fs::Path::fromStdPath(fname.extension()).string()
    );

    if (std::filesystem::exists(m_pack->getResourcesPath() / fname)) {
        return m_pack->getResourcesPath() / fname;
    } else {
        return std::filesystem::path(dirs::getGameDir().native()) / "Resources" / fname;
    }
}

bool PackInfoPopup::init(const std::shared_ptr<Pack>& pack) {
    m_pack = pack;
    if (!Popup::init(320.f, 200.f, asp::fs::Path::fromStdPath(this->getPathInPack("GJ_square01.png")).string().c_str())) return false;

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
        asp::fs::Path::fromStdPath(this->getPathInPack("GJ_button_01.png")).string(),
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
        asp::fs::Path::fromStdPath(this->getPathInPack("GJ_button_02.png")).string(),
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

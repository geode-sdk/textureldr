#include "Pack.hpp"
#include <Geode/utils/JsonValidation.hpp>
#include <Geode/loader/Loader.hpp>
#include <Geode/utils/file.hpp>

Result<PackInfo> PackInfo::from(matjson::Value const& json) {
    auto info = PackInfo();

    auto copyJson = json;
    auto checker = JsonChecker(copyJson);
    auto root = checker.root("[pack.json]").obj();

    auto target = root.needs("textureldr").get<VersionInfo>();

    if (checker.isError()) {
        return Err(checker.getError());
    }
    auto current = Mod::get()->getVersion();
    if (target > VersionInfo(current.getMajor(), current.getMinor(), 99999999)) {
        return Err("Pack targets newer version of TextureLdr");
    }
    info.m_textureldr = target;
    root.needs("name").into(info.m_name);
    root.needs("id").into(info.m_id);
    root.needs("version").into(info.m_version);

    // has single "author" key?
    if (auto author = root.has("author").as<matjson::Type::String>()) {
        info.m_authors = { author.get<std::string>() };
    }
    // otherwise use "authors" key
    else {
        root.needs("authors").into(info.m_authors);
    }

    if (checker.isError()) {
        return Err(checker.getError());
    }

    return Ok(info);
}

ghc::filesystem::path Pack::getPath() const {
    return m_path;
}

std::string Pack::getID() const {
    return m_info.has_value() ? 
        m_info.value().m_id :
        m_path.filename().string();
}

std::string Pack::getDisplayName() const {
    return m_info.has_value() ?
        m_info.value().m_name :
        m_path.filename().string();
}

Result<> Pack::apply() {
    CCFileUtils::get()->addTexturePack(CCTexturePack {
        .m_id = this->getID(),
        .m_paths = { m_path.string() }
    });
    return Ok();
}

Result<> Pack::unapply() const {
    CCFileUtils::get()->removeTexturePack(this->getID());
    return Ok();
}

Result<> Pack::parsePackJson() {
    try {
        auto data = file::readString(m_path / "pack.json");
        if (!data) {
            return Err(data.error());
        }
        auto res = PackInfo::from(matjson::Value::from_str(data.value()));
        if (!res) {
            return Err(res.unwrapErr());
        }
        m_info = res.unwrap();
        return Ok();
    } catch(std::exception& e) {
        return Err("Unable to parse pack.json: " + std::string(e.what()));
    }
}

Pack::~Pack() {
    (void)this->unapply();
}

Result<std::shared_ptr<Pack>> Pack::from(ghc::filesystem::path const& dir) {
    auto pack = std::make_shared<Pack>();
    pack->m_path = dir;
    if (ghc::filesystem::exists(dir / "pack.json")) {
        auto res = pack->parsePackJson();
        if (!res) {
            return Err(res.unwrapErr());
        }
    }
    return Ok(pack);
}

matjson::Value matjson::Serialize<std::shared_ptr<Pack>>::to_json(std::shared_ptr<Pack> const& pack) {
    return matjson::Object({
        { "path", pack->getPath() }
    });
}

std::shared_ptr<Pack> matjson::Serialize<std::shared_ptr<Pack>>::from_json(matjson::Value const& value) {
    return Pack::from(value["path"].as<ghc::filesystem::path>()).unwrap();
}

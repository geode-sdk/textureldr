#include "Pack.hpp"
#include <Geode/utils/JsonValidation.hpp>
#include <Geode/loader/Loader.hpp>
#include <Geode/utils/file.hpp>

Result<PackInfo> PackInfo::from(json::Value const& json) {
    auto info = PackInfo();

    auto copyJson = json;
    auto checker = JsonChecker(copyJson);
    auto root = checker.root("[pack.json]").obj();

    auto target = root.needs("textureldr").get<VersionInfo>();

    if (checker.isError()) {
        return Err(checker.getError());
    }
    if (target > VersionInfo(1, 0, 99999999)) {
        return Err("Pack targets newer version of TextureLdr");
    }
    info.m_textureldr = target;
    root.needs("name").into(info.m_name);
    root.needs("id").into(info.m_id);
    root.needs("version").into(info.m_version);

    // has single "creator" key?
    if (auto creator = root.has("creator").as<json::Type::String>()) {
        info.m_creators = { creator.get<std::string>() };
    }
    // otherwise use "creators" key
    else {
        json::Value().as<std::vector<std::string>>();
        json::Value().as<std::set<std::string>>();
        json::Value().as<std::unordered_set<std::string>>();
        json::Value().as<std::map<std::string, size_t>>();
        json::Value().as<std::unordered_map<std::string, int>>();
        root.needs("creators").into(info.m_creators);
    }

    root.has("edits").into(info.m_edits);

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
    if (m_info) {
        // clear old edits
        m_edits.clear();

        // add new edits
        for (auto& edit : m_info.value().m_edits) {
            if (ghc::filesystem::exists(m_path / edit)) {
                auto res = m_edits.addFrom(m_path / edit);
                if (!res) {
                    log::warn(
                        "Unable to load edits from {}: {}",
                        m_path / edit, res.unwrapErr()
                    );
                }
            } else {
                log::warn("Unable to find edit {}", m_path / edit);
            }
        }
    }
    return Ok();
}

Result<> Pack::unapply() {
    CCFileUtils::get()->removeTexturePack(this->getID());
    m_edits.clear();
    return Ok();
}

Result<> Pack::parsePackJson() {
    try {
        auto data = file::readString(m_path / "pack.json");
        if (!data) {
            return Err(data.error());
        }
        auto res = PackInfo::from(json::Value::from_str(data.value()));
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

json::Value json::Serialize<std::shared_ptr<Pack>>::to_json(std::shared_ptr<Pack> const& pack) {
    return json::Object({
        { "path", pack->getPath() }
    });
}

std::shared_ptr<Pack> json::Serialize<std::shared_ptr<Pack>>::from_json(json::Value const& value) {
    return Pack::from(value["path"].as<ghc::filesystem::path>()).unwrap();
}

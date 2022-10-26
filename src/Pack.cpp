#include "Pack.hpp"
#include <Geode/utils/JsonValidation.hpp>
#include <Geode/loader/Loader.hpp>
#include <Geode/utils/file.hpp>

NewResult<PackInfo> PackInfo::from(nlohmann::json const& json) {
    auto info = PackInfo();

    auto checker = JsonChecker(json);
    auto root = checker.root("[pack.json]").obj();

    auto target = root.needs("textureldr").get<VersionInfo>();

    if (checker.isError()) {
        return NewErr(checker.getError());
    }
    if (target > VersionInfo(1, 0, 99999999)) {
        return NewErr("Pack targets newer version of TextureLdr");
    }
    info.m_textureldr = target;
    root.needs("name").into(info.m_name);
    root.needs("id").into(info.m_id);
    root.needs("version").into(info.m_version);

    // has single "creator" key?
    if (auto creator = root.has("creator").as<value_t::string>()) {
        info.m_creators = { creator.get<std::string>() };
    }
    // otherwise use "creators" key
    else {
        root.needs("creators").into(info.m_creators);
    }

    root.has("edits").into(info.m_edits);

    if (checker.isError()) {
        return NewErr(checker.getError());
    }

    return NewOk(info);
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

NewResult<> Pack::apply() {
    Loader::get()->addTexturePath(m_path);
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
    return NewOk();
}

NewResult<> Pack::unapply() {
    Loader::get()->removeTexturePath(m_path);
    m_edits.clear();
    return NewOk();
}

NewResult<> Pack::parsePackJson() {
    try {
        auto data = file::readString(m_path / "pack.json");
        if (!data) {
            return NewErr(data.error());
        }
        auto res = PackInfo::from(nlohmann::json::parse(data.value()));
        if (!res) {
            return NewErr(res.unwrapErr());
        }
        m_info = res.unwrap();
        return NewOk();
    } catch(std::exception& e) {
        return NewErr("Unable to parse pack.json: " + std::string(e.what()));
    }
}

Pack::~Pack() {
    (void)this->unapply();
}

NewResult<std::shared_ptr<Pack>> Pack::from(ghc::filesystem::path const& dir) {
    auto pack = std::make_shared<Pack>();
    pack->m_path = dir;
    if (ghc::filesystem::exists(dir / "pack.json")) {
        auto res = pack->parsePackJson();
        if (!res) {
            return NewErr(res.unwrapErr());
        }
    }
    return NewOk(pack);
}

void to_json(nlohmann::json& json, std::shared_ptr<Pack> const& pack) {
    json["path"] = pack->getPath();
}

void from_json(nlohmann::json const& json, std::shared_ptr<Pack>& pack) {
    pack = Pack::from(json["path"]).unwrap();
}

#include "Pack.hpp"

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

NewResult<std::shared_ptr<Pack>> Pack::from(ghc::filesystem::path const& dir) {
    auto pack = std::make_shared<Pack>();
    pack->m_path = dir;
    return NewOk(pack);
}

void to_json(nlohmann::json& json, std::shared_ptr<Pack> const& pack) {
    json["path"] = pack->getPath();
}

void from_json(nlohmann::json const& json, std::shared_ptr<Pack>& pack) {
    pack = Pack::from(json["path"]).unwrap();
}

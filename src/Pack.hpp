#pragma once

#include <string>
#include <optional>
#include <fs/filesystem.hpp>
#include <memory>
#include <Geode/utils/Result.hpp>
#include <Geode/utils/json.hpp>

USE_GEODE_NAMESPACE();

struct PackInfo {
    std::string m_id;
    std::string m_name;
};

class Pack {
protected:
    ghc::filesystem::path m_path;
    std::optional<PackInfo> m_info;

public:
    ghc::filesystem::path getPath() const;
    std::string getDisplayName() const;
    std::string getID() const;

    static NewResult<std::shared_ptr<Pack>> from(ghc::filesystem::path const& dir);
};

void to_json(nlohmann::json& json, std::shared_ptr<Pack> const& pack);
void from_json(nlohmann::json const& json, std::shared_ptr<Pack>& pack);

#pragma once

#include <string>
#include <optional>
#include <ghc/filesystem.hpp>
#include <memory>
#include <Geode/utils/Result.hpp>
#include <Geode/utils/VersionInfo.hpp>
#include <Geode/ui/EnterLayerEvent.hpp>
#include "Edit.hpp"

USE_GEODE_NAMESPACE();

struct PackInfo {
    VersionInfo m_textureldr;
    std::string m_id;
    std::string m_name;
    VersionInfo m_version;
    std::vector<std::string> m_creators;
    std::vector<ghc::filesystem::path> m_edits;

    static Result<PackInfo> from(nlohmann::json const& json);
};

class Pack {
protected:
    ghc::filesystem::path m_path;
    std::optional<PackInfo> m_info;
    EditCollection m_edits;

    Result<> parsePackJson();

public:
    ghc::filesystem::path getPath() const;
    std::string getDisplayName() const;
    std::string getID() const;

    Result<> apply();
    Result<> unapply();

    ~Pack();

    static Result<std::shared_ptr<Pack>> from(ghc::filesystem::path const& dir);
};

void to_json(nlohmann::json& json, std::shared_ptr<Pack> const& pack);
void from_json(nlohmann::json const& json, std::shared_ptr<Pack>& pack);

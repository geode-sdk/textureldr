#pragma once

#include <string>
#include <optional>
#include <ghc/filesystem.hpp>
#include <memory>
#include <Geode/utils/Result.hpp>
#include <Geode/utils/VersionInfo.hpp>
#include <Geode/ui/EnterLayerEvent.hpp>
#include <matjson/stl_serialize.hpp>

using namespace geode::prelude;

struct PackInfo {
    VersionInfo m_textureldr;
    std::string m_id;
    std::string m_name;
    VersionInfo m_version;
    std::vector<std::string> m_authors;

    static Result<PackInfo> from(matjson::Value const& json);
};

class Pack {
protected:
    ghc::filesystem::path m_path;
    std::optional<PackInfo> m_info;

    Result<> parsePackJson();

public:
    [[nodiscard]] ghc::filesystem::path getPath() const;
    [[nodiscard]] std::string getDisplayName() const;
    [[nodiscard]] std::string getID() const;

    [[nodiscard]] Result<> apply();
    [[nodiscard]] Result<> unapply() const;

    ~Pack();

    static Result<std::shared_ptr<Pack>> from(ghc::filesystem::path const& dir);
};

template <>
struct matjson::Serialize<std::shared_ptr<Pack>> {
    static matjson::Value to_json(std::shared_ptr<Pack> const& pack);
    static std::shared_ptr<Pack> from_json(matjson::Value const& value);
};

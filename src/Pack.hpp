#pragma once

#include <string>
#include <optional>
#include <memory>
#include <Geode/Result.hpp>
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
    asp::fs::Path m_path;
    asp::fs::Path m_unzippedPath;
    asp::fs::Path m_resourcesPath;
    std::optional<PackInfo> m_info;

    Result<> parsePackJson();

    Result<> setup();
    Result<> extract();
    std::optional<asp::fs::Path> findResourcesPath(asp::fs::Path targetPath);

public:
    // Returns where the pack comes from. typically the folder, but will also point to the .zip files
    // for zipped packs
    [[nodiscard]] asp::fs::Path getOriginPath() const;
    // Returns the path where the resources are, whether the pack is a folder or it was unzipped by the mod
    [[nodiscard]] asp::fs::Path getResourcesPath() const;
    [[nodiscard]] std::string getDisplayName() const;
    [[nodiscard]] std::string getID() const;
    [[nodiscard]] std::optional<PackInfo> getInfo() const;

    [[nodiscard]] Result<> apply();
    [[nodiscard]] Result<> unapply() const;

    ~Pack();

    static Result<std::shared_ptr<Pack>> from(asp::fs::Path const& dir);
};

template <>
struct matjson::Serialize<std::shared_ptr<Pack>> {
    static matjson::Value toJson(std::shared_ptr<Pack> const& pack);
    static Result<std::shared_ptr<Pack>> fromJson(matjson::Value const& value);
};

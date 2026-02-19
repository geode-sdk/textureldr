#pragma once

#include <Geode/loader/Dispatch.hpp>
#include <Geode/loader/Loader.hpp>
#include <Geode/utils/VersionInfo.hpp>

#include <filesystem>
#include <vector>
#include <string>

#ifdef MY_MOD_ID
    #undef MY_MOD_ID
#endif
#define MY_MOD_ID "geode.texture-loader"

namespace geode::texture_loader {

inline bool isLoaded() {
    return geode::Loader::get()->isModLoaded(MY_MOD_ID);
}

struct Pack {
    std::string id;
    std::string name;
    VersionInfo version;
    std::vector<std::string> authors;

    /// Path from where the pack originates from. May be a file (apk, zip) or a folder
    std::filesystem::path path;
    /// Path where the resources are located. This is what is added to the search path
    std::filesystem::path resourcesPath;
};

inline std::vector<Pack> getAvailablePacks() GEODE_EVENT_EXPORT_NORES(&getAvailablePacks, ());
inline std::vector<Pack> getAppliedPacks() GEODE_EVENT_EXPORT_NORES(&getAppliedPacks, ());

}

#undef MY_MOD_ID

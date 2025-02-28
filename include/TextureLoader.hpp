#pragma once

#include <Geode/loader/Dispatch.hpp>
#include <Geode/loader/Loader.hpp>
#include <Geode/utils/VersionInfo.hpp>

#include <filesystem>
#include <vector>
#include <string>

namespace geode::texture_loader {

inline bool isLoaded() {
    return geode::Loader::get()->isModLoaded("geode.texture-loader");
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

namespace impl {
    using EventGetAvailablePacks = geode::DispatchEvent<std::vector<Pack>*>;
    using EventGetAppliedPacks = geode::DispatchEvent<std::vector<Pack>*>;
}

inline std::vector<Pack> getAvailablePacks() {
    std::vector<Pack> result;
    impl::EventGetAvailablePacks("geode.texture-loader/v1/get-available-packs", &result).post();
    return result;
}

inline std::vector<Pack> getAppliedPacks() {
    std::vector<Pack> result;
    impl::EventGetAppliedPacks("geode.texture-loader/v1/get-applied-packs", &result).post();
    return result;
}

}

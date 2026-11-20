#include "Pack.hpp"
#include <Geode/utils/JsonValidation.hpp>
#include <Geode/loader/Loader.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/utils/file.hpp>
#include <asp/fs.hpp>

Result<PackInfo> PackInfo::from(matjson::Value const& json) {
    auto info = PackInfo();

    auto copyJson = json;
    auto root = checkJson(copyJson, "[pack.json]");

    auto target = root.needs("textureldr").get<VersionInfo>();

    GEODE_UNWRAP(root.ok());

    auto current = Mod::get()->getVersion();
    if (target > VersionInfo(current.getMajor(), current.getMinor(), 99999999)) {
        return Err("Pack targets newer version of TextureLdr");
    }
    info.m_textureldr = target;
    root.needs("name").into(info.m_name);
    root.needs("id").into(info.m_id);
    root.needs("version").into(info.m_version);

    // has single "author" key?
    if (auto author = root.has("author")) {
        std::string temp;
        author.into(temp);
        info.m_authors = { temp };
    }
    // otherwise use "authors" key
    else {
        root.needs("authors").into(info.m_authors);
    }

    GEODE_UNWRAP(root.ok());

    return Ok(info);
}

asp::fs::Path Pack::getOriginPath() const {
    return m_path;
}

asp::fs::Path Pack::getResourcesPath() const {
    return m_resourcesPath;
}

std::string Pack::getID() const {
    return m_info.has_value() ? 
        m_info.value().m_id :
        m_path.filename() ? m_path.filename()->string() : "Unknown";
}

std::string Pack::getDisplayName() const {
    return m_info.has_value() ?
        m_info.value().m_name :
        m_path.filename() ? m_path.filename()->string() : "Unknown";
}

std::optional<PackInfo> Pack::getInfo() const {
    return m_info;
}

Result<> Pack::apply() {
    CCFileUtils::get()->addTexturePack(CCTexturePack {
        .m_id = this->getID(),
        .m_paths = { this->getResourcesPath().string() }
    });
    return Ok();
}

Result<> Pack::unapply() const {
    CCFileUtils::get()->removeTexturePack(this->getID());
    return Ok();
}

Result<> Pack::parsePackJson() {
    try {
        GEODE_UNWRAP_INTO(auto json, file::readJson(m_resourcesPath / "pack.json"));
        GEODE_UNWRAP_INTO(m_info, PackInfo::from(json));
        return Ok();
    } catch(std::exception& e) {
        return Err("Unable to parse pack.json: {}", e.what());
    }
}

Result<> Pack::setup() {
    m_unzippedPath = m_path;
    m_resourcesPath = m_path;
    GEODE_UNWRAP(this->extract());
    auto optPath = this->findResourcesPath(m_unzippedPath);
    if (optPath) {
        m_resourcesPath = *optPath;
    }
    // TODO: read this from the zip before extracting.. somehow
    if ((m_resourcesPath / "pack.json").exists()) {
        GEODE_UNWRAP(this->parsePackJson());
    }
    return Ok();
}

Result<> Pack::extract() {
    // this method is only for zips and stuff
    if (m_path.isDir()) return Ok();

    auto const fileExt = m_path.extension();
    // TODO: we dont support rar, lol
    if (fileExt != ".zip" && fileExt != ".apk") {
        return Err("Expected zip or apk");
    }

    auto extractPath = Mod::get()->getSaveDir() / "unzipped" / this->getID();
    (void) utils::file::createDirectoryAll(extractPath);

    auto datePath = extractPath / "modified-at";
    std::string currentHash = file::readString(datePath).unwrapOr("");

    auto metadata = asp::fs::metadata(m_path);
    if (!metadata) {
        return Err("Unable get metadata: {}", metadata.unwrapErr());
    }

    auto modifiedDate = metadata.unwrap().modifiedAt;
    auto modifiedHash = modifiedDate.has_value() ? fmt::to_string(modifiedDate->timeSinceEpoch()) : "Unknown";
    if (currentHash == modifiedHash) {
        m_unzippedPath = extractPath;
        return Ok();
    }
    log::debug("Hash mismatch detected, unzipping {}", this->getID());

    auto removeRes = asp::fs::removeDirAll(extractPath, false);
    if(!removeRes) {
        return Err("Unable to delete temp dir: {}", removeRes.unwrapErr());
    }

    (void) utils::file::createDirectoryAll(extractPath);
    auto res = file::writeString(datePath, modifiedHash);
    if (!res) {
        log::warn("Failed to write modified date of extracted pack: {}", res.unwrapErr());
    }

    GEODE_UNWRAP_INTO(auto unzip, file::Unzip::create(m_path));
    GEODE_UNWRAP(unzip.extractAllTo(extractPath));

    m_unzippedPath = extractPath;
    m_resourcesPath = extractPath;

    return Ok();
}

std::optional<asp::fs::Path> Pack::findResourcesPath(asp::fs::Path targetPath) {
    // Packs are often distributed in weird ways, this code tries to find where the resources actually are..

    if (m_path.extension() == ".apk") {
        // resources can only be in one place! very easy
        return m_unzippedPath / "assets";
    }

    if ((targetPath / "pack.json").exists() || (targetPath / "pack.png").exists()) {
        // this pack is made for texture loader, so it should be correct already
        return targetPath;
    }

    const auto existsDir = [](auto path) {
        return path.exists() && path.isDir();
    };

    if (existsDir(targetPath / "Resources")) {
        // its probably there, i hope
        return targetPath / "Resources";
    }

    // 2.2 icons folder
    if (existsDir(targetPath / "icons")) {
        return targetPath;
    }

    // Look for any plist files, or png files ending in -uhd -hd or starting in GJ_

    auto pathIterRes = asp::fs::iterDir(targetPath);
    if(pathIterRes) for (auto fileRes : pathIterRes.unwrap()) {
        if(!fileRes) continue;
        auto file = fileRes.unwrap();
        if (!file.isFile()) continue;

        auto const path = file.path;
        auto const name = path.stem().value_or("");
        auto const ext = path.extension().value_or("");

        if (ext == ".plist") {
            return targetPath;
        } else if (ext == ".png" && (name.starts_with("GJ_") || name.ends_with("-hd") || name.ends_with("-uhd"))) {
            return targetPath;
        } else if (ext == ".ogg") {
            return targetPath;
        } else if (ext == ".mp3") {
            return targetPath;
        }
    }

    // ok, look recursively through the folders then
    auto pathIterRes2 = asp::fs::iterDir(targetPath);
    if(pathIterRes2) for (auto dirRes : pathIterRes2.unwrap()) {
        if(!dirRes) continue;
        auto dir = dirRes.unwrap();
        if (!dir.isDir()) continue;

        auto const path = dir.path;

        // TODO: this might skip over texture packs that set geode mod textures..
        // though, they should be using pack.json or pack.png anyways!
        auto const opt = this->findResourcesPath(path);
        if (opt) {
            return *opt;
        }
    }

    return std::nullopt;
}

Pack::~Pack() {
    (void)this->unapply();
}

Result<std::shared_ptr<Pack>> Pack::from(asp::fs::Path const& dir) {
    if (!dir.exists()) {
        return Err("Path does not exist");
    }
    auto pack = std::make_shared<Pack>();
    pack->m_path = dir;
    GEODE_UNWRAP(pack->setup());
    return Ok(pack);
}

matjson::Value matjson::Serialize<std::shared_ptr<Pack>>::toJson(std::shared_ptr<Pack> const& pack) {
    return matjson::makeObject({
        { "path", pack->getOriginPath() }
    });
}

Result<std::shared_ptr<Pack>> matjson::Serialize<std::shared_ptr<Pack>>::fromJson(matjson::Value const& value) {
    GEODE_UNWRAP_INTO(auto path, value["path"].as<asp::fs::Path>());
    GEODE_UNWRAP_INTO(auto pack, Pack::from(path));
    return Ok(pack);
}

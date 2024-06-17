#include "Pack.hpp"
#include <Geode/utils/JsonValidation.hpp>
#include <Geode/loader/Loader.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/utils/file.hpp>

Result<PackInfo> PackInfo::from(matjson::Value const& json) {
    auto info = PackInfo();

    auto copyJson = json;
    auto checker = JsonChecker(copyJson);
    auto root = checker.root("[pack.json]").obj();

    auto target = root.needs("textureldr").get<VersionInfo>();

    if (checker.isError()) {
        return Err(checker.getError());
    }
    auto current = Mod::get()->getVersion();
    if (target > VersionInfo(current.getMajor(), current.getMinor(), 99999999)) {
        return Err("Pack targets newer version of TextureLdr");
    }
    info.m_textureldr = target;
    root.needs("name").into(info.m_name);
    root.needs("id").into(info.m_id);
    root.needs("version").into(info.m_version);

    // has single "author" key?
    if (auto author = root.has("author").as<matjson::Type::String>()) {
        info.m_authors = { author.get<std::string>() };
    }
    // otherwise use "authors" key
    else {
        root.needs("authors").into(info.m_authors);
    }

    if (checker.isError()) {
        return Err(checker.getError());
    }

    return Ok(info);
}

std::filesystem::path Pack::getOriginPath() const {
    return m_path;
}

std::filesystem::path Pack::getResourcesPath() const {
    return m_resourcesPath;
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
        auto data = file::readString(m_resourcesPath / "pack.json");
        if (!data) {
            return Err(data.error());
        }
        auto res = PackInfo::from(matjson::Value::from_str(data.value()));
        if (!res) {
            return Err(res.unwrapErr());
        }
        m_info = res.unwrap();
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
    if (std::filesystem::exists(m_resourcesPath / "pack.json")) {
        GEODE_UNWRAP(this->parsePackJson());
    }
    return Ok();
}

Result<> Pack::extract() {
    // this method is only for zips and stuff
    if (std::filesystem::is_directory(m_path)) return Ok();

    auto const fileExt = m_path.extension().string();
    // TODO: we dont support rar, lol
    if (fileExt != ".zip" && fileExt != ".apk") {
        return Err("Expected zip or apk");
    }

    auto extractPath = Mod::get()->getSaveDir() / "unzipped" / this->getID();
    (void) utils::file::createDirectoryAll(extractPath);

    auto datePath = extractPath / "modified-at";
    std::string currentHash = file::readString(datePath).unwrapOr("");

    std::error_code ec;
    auto modifiedDate = std::filesystem::last_write_time(m_path, ec);
    if (ec) {
        return Err("Unable get last_write_time: {}", ec.message());
    }
    auto modifiedCount = std::chrono::duration_cast<std::chrono::milliseconds>(modifiedDate.time_since_epoch());
    auto modifiedHash = std::to_string(modifiedCount.count());
    if (currentHash == modifiedHash) {
        m_unzippedPath = extractPath;
        return Ok();
    }
    log::debug("Hash mismatch detected, unzipping {}", this->getID());

    std::filesystem::remove_all(extractPath, ec);
    if (ec) {
        return Err("Unable to delete temp dir: {}", ec.message());
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

std::optional<std::filesystem::path> Pack::findResourcesPath(std::filesystem::path targetPath) {
    // Packs are often distributed in weird ways, this code tries to find where the resources actually are..

    if (m_path.extension().string() == ".apk") {
        // resources can only be in one place! very easy
        return m_unzippedPath / "assets";
    }

    if (std::filesystem::exists(targetPath / "pack.json") || std::filesystem::exists(targetPath / "pack.png")) {
        // this pack is made for texture loader, so it should be correct already
        return targetPath;
    }

    const auto existsDir = [](auto path) {
        return std::filesystem::exists(path) && std::filesystem::is_directory(path);
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

    for (auto const& file : std::filesystem::directory_iterator(targetPath, std::filesystem::directory_options::skip_permission_denied)) {
        if (!file.is_regular_file()) continue;

        auto const path = file.path();
        auto const name = path.stem().string();
        auto const ext = path.extension().string();

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
    for (auto const& dir : std::filesystem::directory_iterator(targetPath, std::filesystem::directory_options::skip_permission_denied)) {
        if (!dir.is_directory()) continue;

        auto const path = dir.path();

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

Result<std::shared_ptr<Pack>> Pack::from(std::filesystem::path const& dir) {
    #ifdef GEODE_IS_WINDOWS
    try {
        auto test = dir.filename().string();
    } catch(const std::exception& e) {
        return Err("Invalid path");
    }
    #endif

    if (!std::filesystem::exists(dir)) {
        return Err("Path does not exist");
    }
    auto pack = std::make_shared<Pack>();
    pack->m_path = dir;
    GEODE_UNWRAP(pack->setup());
    return Ok(pack);
}

matjson::Value matjson::Serialize<std::shared_ptr<Pack>>::to_json(std::shared_ptr<Pack> const& pack) {
    return matjson::Object({
        { "path", pack->getOriginPath() }
    });
}

std::shared_ptr<Pack> matjson::Serialize<std::shared_ptr<Pack>>::from_json(matjson::Value const& value) {
    return Pack::from(value["path"].as<std::filesystem::path>()).unwrap();
}

#include "mod/MyMod.h"

#include <filesystem>
#include <system_error>

#include <pl/Mod.hpp>

namespace horion_android {

HorionAndroidMod &HorionAndroidMod::instance() {
    static HorionAndroidMod mod;
    return mod;
}

HorionAndroidMod::HorionAndroidMod()
    : mSelf(*ll::mod::NativeMod::current()) {}

bool HorionAndroidMod::load() {
    auto &self = getSelf();

    try {
        std::error_code ec;
        std::filesystem::create_directories(self.getDataDir(), ec);
        if (ec) {
            self.getLogger().error("Cannot create mod data directory: {}", ec.message());
            return false;
        }

        std::filesystem::create_directories(self.getConfigDir(), ec);
        if (ec) {
            self.getLogger().error("Cannot create mod config directory: {}", ec.message());
            return false;
        }

        mConfigFile.emplace();
        if (!mConfigFile->load()) {
            self.getLogger().error("Failed to load Horion Android configuration");
            mConfigFile.reset();
            return false;
        }

        mConfig = mConfigFile->value();
        self.getLogger().info(
            "Loaded {}. This scaffold registers no gameplay hooks or patches.",
            self.getName()
        );
        return true;
    } catch (const std::exception &e) {
        self.getLogger().error("Horion Android load failed: {}", e.what());
        mConfigFile.reset();
        return false;
    } catch (...) {
        self.getLogger().error("Horion Android load failed with an unknown error");
        mConfigFile.reset();
        return false;
    }
}

bool HorionAndroidMod::enable() {
    auto &logger = getSelf().getLogger();

    if (mConfig.enabled) {
        logger.info("Diagnostic message: {}", mConfig.message);
    } else {
        logger.info("Diagnostic logging is disabled by config");
    }

    return true;
}

bool HorionAndroidMod::disable() {
    getSelf().getLogger().info("Horion Android scaffold disabled");
    return true;
}

bool HorionAndroidMod::unload() {
    getSelf().getLogger().info("Horion Android scaffold unloaded");
    mConfigFile.reset();
    return true;
}

} // namespace horion_android

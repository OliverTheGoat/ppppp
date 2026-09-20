#include "mod/MyMod.h"

#include "mod/GameBridge.h"

#include <filesystem>

#include <pl/Mod.hpp>
#include <pl/ModMenu.hpp>

namespace clange_me {

ClangeMeMod &ClangeMeMod::instance() {
    static ClangeMeMod instance;
    return instance;
}

ClangeMeMod::ClangeMeMod() : mSelf(*ll::mod::NativeMod::current()) {}

bool ClangeMeMod::load() {
    auto &self = getSelf();
    self.getLogger().debug("Loading...");

    std::error_code ec;
    std::filesystem::create_directories(self.getDataDir(), ec);
    if (ec) {
        self.getLogger().error("Failed to create data directory {}: {}", self.getDataDir().string(),
                               ec.message());
        return false;
    }

    std::filesystem::create_directories(self.getConfigDir(), ec);
    if (ec) {
        self.getLogger().error("Failed to create config directory {}: {}",
                               self.getConfigDir().string(), ec.message());
        return false;
    }

    mConfigFile.emplace();
    if (!mConfigFile->load()) {
        self.getLogger().warn("Failed to load typed config");
        return false;
    }
    mConfig = mConfigFile->value();

    if (!game::initialize()) {
        self.getLogger().warn("Game bridge could not resolve the 1.26.50 client constructor yet");
    }

    self.getLogger().info("Loaded {} from {}", self.getName(), self.getModDir().string());
    return true;
}

bool ClangeMeMod::enable() {
    auto &self = getSelf();
    self.getLogger().debug("Enabling...");
    if (!mConfig.enabled) {
        self.getLogger().info("clange_me is disabled by config");
        return true;
    }

    pl::modmenu::ModuleBuilder("lords_shulker", "Lords Shulker")
        .description("Load the Lords Shulker action from the Mod Menu.")
        .modId(self.getId())
        .defaultEnabled(true)
        .registerModule();

    pl::modmenu::ButtonBuilder("lords_shulker.give", "Lords Shulker")
        .modId(self.getId())
        .moduleId("lords_shulker")
        .label("Give")
        .behavior(pl::modmenu::ButtonBehavior::Click)
        .onEvent([this](std::string_view, pl::modmenu::ButtonEvent event, float) {
            if (event != pl::modmenu::ButtonEvent::Click) {
                return;
            }

            if (!game::runCommand("/give @s red_shulker_box 1")) {
                getSelf().getLogger().warn(
                    "Lords Shulker button pressed before the client screen model was available");
            }
        })
        .registerButton();

    self.getLogger().info("Config message: {}", mConfig.message);
    return true;
}

bool ClangeMeMod::disable() {
    auto &self = getSelf();
    self.getLogger().debug("Disabling...");
    pl::modmenu::unregisterButton("lords_shulker.give");
    pl::modmenu::unregisterModule("lords_shulker");
    return true;
}

bool ClangeMeMod::unload() {
    getSelf().getLogger().debug("Unloading...");
    mConfigFile.reset();
    return true;
}

} // namespace clange_me

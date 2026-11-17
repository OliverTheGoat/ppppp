#pragma once

#include <string>
#include <string_view>

#include <pl/Config.hpp>

namespace horion_android {

struct ModConfig {
    int version = 1;
    bool enabled = true;
    std::string message = "Horion Android native scaffold loaded.";
};

nlohmann::json makeDefaultConfigJson();
nlohmann::json makeConfigSchemaJson();

} // namespace horion_android

template <> struct pl::config::Schema<horion_android::ModConfig> {
    static constexpr std::string_view title = "Horion Android Settings";
    static constexpr std::string_view description =
        "Diagnostic settings for the native package scaffold.";

    static constexpr FieldSchema field(std::string_view name) {
        if (name == "version")
            return {.title = "Version", .readOnly = true};
        if (name == "enabled")
            return {.title = "Diagnostic logging",
                    .description = "Enables or disables the startup diagnostic message."};
        if (name == "message")
            return {.title = "Startup message",
                    .description = "Text written to the mod log when enabled."};
        return {};
    }
};

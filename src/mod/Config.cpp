#include "mod/Config.h"

namespace horion_android {

nlohmann::json makeDefaultConfigJson() {
    return pl::config::defaultJson(ModConfig{});
}

nlohmann::json makeConfigSchemaJson() {
    return pl::config::schema(ModConfig{});
}

} // namespace horion_android

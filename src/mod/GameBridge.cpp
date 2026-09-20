#include "mod/GameBridge.h"

#include <algorithm>
#include <cmath>
#include <dlfcn.h>
#include <memory>
#include <mutex>
#include <string>
#include <utility>

#include <pl/memory/Hook.hpp>

// Minimal ABI-compatible declarations for the 26.50 client-side MCAPI types
// used by this bridge. These are resolved from libminecraftpe.so at runtime.
namespace Core {
template <typename T>
class PathBuffer {
public:
    T value;
};

class Path : public PathBuffer<std::string> {
public:
    Path() = default;
    explicit Path(std::string const& value) : PathBuffer<std::string>{value} {}
    explicit Path(std::string&& value) : PathBuffer<std::string>{std::move(value)} {}
    explicit Path(char const* value) : PathBuffer<std::string>{std::string(value)} {}
};
} // namespace Core

struct Vec3 {
    float x;
    float y;
    float z;
};

class StructureTemplate;

class ClientInstanceScreenModel {
public:
    void sendChatMessage(std::string const& message);
    StructureTemplate* importStructureBlock(
        std::string const& structureName,
        Core::Path const& filePath
    );
    void insertStructureBlockRequest(
        std::string const& structureName,
        StructureTemplate const& structureTemplate
    );
    Vec3 getPlayerPosition() const;
};

using ModelPtr = std::shared_ptr<ClientInstanceScreenModel>;
using CtorFn = void* (*)(void*, ModelPtr);

std::mutex gMutex;
ClientInstanceScreenModel* gModel = nullptr;
CtorFn gOriginalCtor = nullptr;
bool gInitialized = false;

constexpr const char* kCtorSymbol =
    "_ZN30ClientInstanceScreenController5$ctorESt10shared_ptrI25ClientInstanceScreenModelE";

void* findGameSymbol(const char* name) {
    if (void* p = dlsym(RTLD_DEFAULT, name)) {
        return p;
    }

    void* handle = dlopen("libminecraftpe.so", RTLD_NOW | RTLD_NOLOAD);
    if (!handle) {
        return nullptr;
    }

    void* p = dlsym(handle, name);
    dlclose(handle);
    return p;
}

void* hookedCtor(void* self, ModelPtr model) {
    ClientInstanceScreenModel* captured = model.get();
    void* result = gOriginalCtor ? gOriginalCtor(self, std::move(model)) : self;

    if (captured) {
        std::lock_guard lock(gMutex);
        gModel = captured;
    }

    return result;
}

std::string makePosition(int x, int y, int z) {
    return std::to_string(x) + " " + std::to_string(y) + " " + std::to_string(z);
}

} // namespace

bool initialize() {
    std::lock_guard lock(gMutex);
    if (gInitialized) {
        return true;
    }

    auto ctor = findGameSymbol(kCtorSymbol);
    if (!ctor) {
        return false;
    }

    const auto rc = pl::memory::hook(
        ctor,
        reinterpret_cast<pl::memory::FuncPtr>(&hookedCtor),
        reinterpret_cast<pl::memory::FuncPtr*>(&gOriginalCtor)
    );
    if (rc != 0) {
        return false;
    }

    gInitialized = true;
    return true;
}

bool isReady() {
    std::lock_guard lock(gMutex);
    return gInitialized && gModel != nullptr;
}

bool giveLordsShulker(const std::string& structurePath) {
    ClientInstanceScreenModel* model = nullptr;
    {
        std::lock_guard lock(gMutex);
        if (!gInitialized || !gModel) {
            return false;
        }
        model = gModel;
    }

    const std::string structureName = "mystructure:Lords_Shulker";
    Core::Path filePath(structurePath);
    StructureTemplate* structure = model->importStructureBlock(structureName, filePath);
    if (!structure) {
        return false;
    }

    // Send the uploaded structure template to the connected server.
    model->insertStructureBlockRequest(structureName, *structure);

    // Work well away from the player, then loot the shulker block that exists
    // at relative (2, 3, 2) inside the uploaded 5x5x5 template.
    const Vec3 player = model->getPlayerPosition();
    const int baseX = static_cast<int>(std::floor(player.x)) + 10000;
    const int baseY = static_cast<int>(std::floor(player.y));
    const int baseZ = static_cast<int>(std::floor(player.z)) + 10000;
    const int shulkerX = baseX + 2;
    const int shulkerY = baseY + 3;
    const int shulkerZ = baseZ + 2;

    model->sendChatMessage(
        "/structure load " + structureName + " " + makePosition(baseX, baseY, baseZ)
    );
    model->sendChatMessage(
        "/loot give @s mine " + makePosition(shulkerX, shulkerY, shulkerZ)
    );
    model->sendChatMessage(
        "/fill " + makePosition(baseX, baseY, baseZ) + " "
        + makePosition(baseX + 4, baseY + 4, baseZ + 4) + " air"
    );

    return true;
}

} // namespace clange_me::game

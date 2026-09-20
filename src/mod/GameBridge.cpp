#include "mod/GameBridge.h"

#include <dlfcn.h>
#include <memory>
#include <mutex>
#include <string>
#include <utility>

#include <pl/memory/Hook.hpp>

namespace clange_me::game {
namespace {

class ClientInstanceScreenModel {
public:
    void sendChatMessage(std::string const& message);
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

bool runCommand(const std::string& command) {
    std::lock_guard lock(gMutex);
    if (!gModel) {
        return false;
    }

    gModel->sendChatMessage(command);
    return true;
}

} // namespace clange_me::game

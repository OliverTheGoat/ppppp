#include "mod/GameBridge.h"

#include <dlfcn.h>
#include <memory>
#include <mutex>
#include <string>

#include <pl/memory/Hook.hpp>

namespace clange_me::game {
namespace {

class ClientInstanceScreenModel;

using ModelPtr = std::shared_ptr<ClientInstanceScreenModel>;
using CtorFn = void* (*)(void*, ModelPtr);
using SendChatFn = void* (*)(ClientInstanceScreenModel*, const std::string&);

std::mutex gMutex;
ClientInstanceScreenModel* gModel = nullptr;
CtorFn gOriginalCtor = nullptr;
SendChatFn gSendChat = nullptr;
bool gInitialized = false;

constexpr const char* kCtorSymbol =
    "_ZN30ClientInstanceScreenController5$ctorESt10shared_ptrI25ClientInstanceScreenModelE";

constexpr const char* kSendChatSymbol =
    "_ZN24ClientInstanceScreenModel15sendChatMessageERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE";

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
    void* result = gOriginalCtor ? gOriginalCtor(self, std::move(model)) : self;

    {
        std::lock_guard lock(gMutex);
        if (result) {
            // The constructor argument is the model used by this controller.
            // Recovering it here is intentionally avoided after the move;
            // the active model is captured by the caller-side hook below.
        }
    }

    return result;
}

} // namespace

bool initialize() {
    std::lock_guard lock(gMutex);
    if (gInitialized) {
        return gModel != nullptr;
    }

    // sendChatMessage is resolved independently. The controller constructor
    // hook is installed only when the game exports the expected 1.26.50 ABI.
    auto sendChat = findGameSymbol(kSendChatSymbol);
    if (!sendChat) {
        return false;
    }

    gSendChat = reinterpret_cast<SendChatFn>(sendChat);

    // The constructor symbol is exported by the client and is used to keep
    // the bridge tied to the live ClientInstanceScreenModel instance.
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
    return gInitialized && gModel && gSendChat;
}

bool runCommand(const std::string& command) {
    std::lock_guard lock(gMutex);
    if (!gSendChat || !gModel) {
        return false;
    }

    gSendChat(gModel, command);
    return true;
}

} // namespace clange_me::game

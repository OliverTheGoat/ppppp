#pragma once

#include <string>

namespace clange_me::game {

bool initialize();
bool isReady();

// Imports the bundled .mcstructure into the current client and uses the
// resulting server-side structure to produce the configured Lords Shulker.
bool giveLordsShulker(const std::string& structurePath);

} // namespace clange_me::game

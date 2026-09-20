#pragma once

#include <string>

namespace clange_me::game {

bool initialize();
bool isReady();
bool runCommand(const std::string& command);

} // namespace clange_me::game

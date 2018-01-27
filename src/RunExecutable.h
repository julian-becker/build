#pragma once
#include "Events.h"
#include <string>
#include <vector>

namespace System {

Events::Event<char> runExecutable(std::string exe, std::vector<std::string> args);

}

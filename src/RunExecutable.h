#pragma once
#include "Events.h"
#include <string>

namespace System {

Events::Event<char> runExecutable(std::string exe);

}

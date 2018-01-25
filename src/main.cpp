#include <build/build.h>
#include <memory>
#include <type_traits>
#include <map>
#include <functional>
#include <iostream>
#include <thread>
#include <chrono>
#include <future>
#include "Events.h"
#include "RunExecutable.h"

using namespace Events;
using namespace System;

int main() {
    auto canceler1 = runExecutable("/usr/bin/gcc").subscribe([](char c){ std::cout << c; });
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    canceler1.cancel();
}

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
#include "ElasticQueue.h"

using namespace Events;
using namespace System;
using namespace Connectors;


int main() {
    auto canceler1 = runExecutable(
        "/usr/bin/g++",
        {
            "--std=c++1z", "-g", "-o", "exe",
            "-Iinclude",
            "src/main.cpp", "src/Events.cpp", "src/RunExecutable.cpp"
        }
    ).subscribe([&](char c){ std::cout << c; });
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    canceler1.cancel();
    std::cout << "Hello newer world!" << std::endl;
}

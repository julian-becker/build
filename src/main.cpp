#include <build/build.h>
#include <memory>
#include <type_traits>
#include <map>
#include <functional>
#include <iostream>
#include <thread>
#include <chrono>
#include <future>
#include <set>
#include "Events.h"
#include "RunExecutable.h"
#include "ElasticQueue.h"

using namespace Events;
using namespace System;
using namespace Connectors;

class CompilerSettings {
    std::set<std::string> m_includePaths;

public:
    CompilerSettings(std::set<std::string> includePaths)
        : m_includePaths{std::move(includePaths)}
    {}

    std::set<std::string> const& getIncludePaths() const { return m_includePaths; }
};


class Compiler {
    CompilerSettings m_settings;
    std::string m_compiler;

public:
    Compiler(std::string compiler, CompilerSettings settings)
        : m_compiler{std::move(compiler)}
        , m_settings{std::move(settings)}
    {}

    auto compile(std::string outputExecutable, std::set<std::string> sources) {
        std::vector<std::string> args;
        args.push_back("-g");
        args.push_back("--std=c++1z");
        args.push_back("-o");
        args.push_back(std::move(outputExecutable));

        for(auto const& path : m_settings.getIncludePaths())
            args.push_back("-I" + path);

        for(auto&& source : sources)
            args.push_back(std::move(source));

        return runExecutable(m_compiler,args);
    }
};


int main() {
    try {
        CompilerSettings settings{{"include"}};
        Compiler compiler{"/usr/bin/g++", std::move(settings)};
        auto canceler = compiler.compile("exe",
            {"src/frp.cpp"}
        ).subscribe([&](char c){ std::cout << c; });;
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        canceler.cancel();
    }
    catch(std::exception const& ex) {
        std::cerr << "EXCEPTION: " << ex.what() << std::endl;
    }
}

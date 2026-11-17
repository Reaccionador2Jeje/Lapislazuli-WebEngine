#include "lapislazuli/core.hpp"

#include <cassert>
#include <iostream>

int main() {
    using lapislazuli::core::EngineRuntime;
    using lapislazuli::core::ModuleStatus;

    const auto version = EngineRuntime::engineVersion();
    assert(version.major == 0);
    assert(version.minor == 9);
    assert(version.patch == 0);

    const auto api = EngineRuntime::publicApiVersion();
    assert(api.major == 0);
    assert(api.minor == 1);
    assert(!EngineRuntime::engineName().empty());

    const auto modules = EngineRuntime::modules();
    assert(modules.size() == 10);
    assert(EngineRuntime::findModule("dom") != nullptr);
    assert(EngineRuntime::findModule("storage") != nullptr);
    assert(EngineRuntime::findModule("security") != nullptr);
    assert(EngineRuntime::findModule("render") != nullptr);
    assert(EngineRuntime::findModule("browser") != nullptr);
    assert(EngineRuntime::findModule("does-not-exist") == nullptr);
    assert(EngineRuntime::findModule("net")->status == ModuleStatus::Foundation);
    assert(EngineRuntime::findModule("js")->status == ModuleStatus::Experimental);
    assert(lapislazuli::core::moduleStatusName(ModuleStatus::Foundation) == "foundation");

    std::cout << "Core runtime metadata tests passed.\n";
}

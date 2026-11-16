#include <lapislazuli/lapislazuli.hpp>

#include <iostream>

int main() {
    const auto version = lapislazuli::core::EngineRuntime::engineVersion();
    const auto modules = lapislazuli::core::EngineRuntime::modules();

    std::cout << lapislazuli::core::EngineRuntime::engineName()
              << " " << version.major << "." << version.minor << "." << version.patch
              << " (" << modules.size() << " modules)\n";

    return modules.empty() ? 1 : 0;
}

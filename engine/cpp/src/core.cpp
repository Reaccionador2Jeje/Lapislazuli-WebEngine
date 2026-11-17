#include "lapislazuli/core.hpp"

#include <array>

namespace lapislazuli::core {
namespace {

constexpr std::array<ModuleDescriptor, 10> kModules{{
    {"dom", "HTML and DOM", ModuleStatus::Experimental,
     "HTML tokenization, document parsing, and mutable DOM primitives."},
    {"css", "CSS", ModuleStatus::Experimental,
     "A limited stylesheet, selector, cascade, and value-processing implementation."},
    {"layout", "Layout", ModuleStatus::Experimental,
     "Initial box-model and block-flow layout calculations."},
    {"render", "Rendering", ModuleStatus::Experimental,
     "Backend-neutral display lists for backgrounds, borders, text, and image placement."},
    {"js", "JavaScript", ModuleStatus::Experimental,
     "A limited interpreter runtime with a mutable host DOM bridge."},
    {"net", "Networking", ModuleStatus::Foundation,
     "HTTP transport with HTTPS-first defaults, TLS verification, and response limits."},
    {"storage", "Persistent storage", ModuleStatus::Foundation,
     "SQLite-backed key/value storage partitioned by a canonical HTTP(S) origin."},
    {"security", "Origin security", ModuleStatus::Foundation,
     "Constrained canonical tuple-origin parsing and same-origin comparisons for HTTP(S) URLs."},
    {"browser", "Browser integration", ModuleStatus::Experimental,
     "An initial navigation facade with HTML parsing and bounded resource loading."},
    {"core", "Runtime and module registry", ModuleStatus::Foundation,
     "Version metadata and discoverable module capability information."}
}};

} // namespace

std::string_view EngineRuntime::engineName() noexcept {
    return "Lapislazuli WebEngine";
}

std::string_view EngineRuntime::projectWebsite() noexcept {
    return "https://github.com/Reaccionador2Jeje/Lapislazuli-WebEngine";
}

std::span<const ModuleDescriptor> EngineRuntime::modules() noexcept {
    return kModules;
}

const ModuleDescriptor* EngineRuntime::findModule(std::string_view id) noexcept {
    for (const auto& module : kModules) {
        if (module.id == id) return &module;
    }
    return nullptr;
}

std::string_view moduleStatusName(ModuleStatus status) noexcept {
    switch (status) {
        case ModuleStatus::Foundation: return "foundation";
        case ModuleStatus::Experimental: return "experimental";
    }
    return "unknown";
}

} // namespace lapislazuli::core

#pragma once

#include "lapislazuli/export.hpp"

#include <array>
#include <cstdint>
#include <span>
#include <string_view>

namespace lapislazuli::core {

struct Version {
    std::uint16_t major;
    std::uint16_t minor;
    std::uint16_t patch;
};

enum class ModuleStatus : std::uint8_t {
    Foundation,
    Experimental
};

struct ModuleDescriptor {
    std::string_view id;
    std::string_view name;
    ModuleStatus status;
    std::string_view summary;
};

// Runtime metadata for embedders. This API reports what is compiled into this
// distribution; it does not imply complete web-standard compatibility.
class LAPISLAZULI_API EngineRuntime final {
public:
    [[nodiscard]] static constexpr Version engineVersion() noexcept {
        return {0, 9, 0};
    }

    [[nodiscard]] static constexpr Version publicApiVersion() noexcept {
        return {0, 1, 0};
    }

    [[nodiscard]] static std::string_view engineName() noexcept;
    [[nodiscard]] static std::string_view projectWebsite() noexcept;
    [[nodiscard]] static std::span<const ModuleDescriptor> modules() noexcept;
    [[nodiscard]] static const ModuleDescriptor* findModule(std::string_view id) noexcept;
};

[[nodiscard]] LAPISLAZULI_API std::string_view moduleStatusName(ModuleStatus status) noexcept;

} // namespace lapislazuli::core

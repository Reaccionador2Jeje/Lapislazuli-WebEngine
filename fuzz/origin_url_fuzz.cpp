#include "lapislazuli/browser/engine.hpp"
#include "lapislazuli/security/origin.hpp"

#include <cstddef>
#include <cstdint>
#include <string_view>

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size) {
    const std::string_view input(reinterpret_cast<const char*>(data), size);
    try { (void)lapislazuli::security::Origin::fromUrl(input); } catch (...) {}
    try {
        (void)lapislazuli::browser::Engine::resolveUrl(
            "https://fuzz.example.test/a/b/index.html", input);
    } catch (...) {}
    return 0;
}

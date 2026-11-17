#include "lapislazuli/security/origin.hpp"

#include <cassert>
#include <iostream>
#include <string_view>

using lapislazuli::security::Origin;
using lapislazuli::security::SecurityError;

template <typename Function>
void rejects(Function&& function) {
    bool caught = false;
    try { function(); } catch (const SecurityError&) { caught = true; }
    assert(caught);
}

int main() {
    const auto secure = Origin::fromUrl("HTTPS://Example.COM:443/path?q=1#top");
    const auto same = Origin::fromUrl("https://example.com/other");
    const auto custom = Origin::fromUrl("https://example.com:8443/");
    const auto plain = Origin::fromUrl("http://example.com:80/");

    assert(secure.valid());
    assert(secure.serialize() == "https://example.com");
    assert(secure.sameOrigin(same));
    assert(!secure.sameOrigin(custom));
    assert(!secure.sameOrigin(plain));
    assert(custom.serialize() == "https://example.com:8443");
    assert(plain.serialize() == "http://example.com");
    assert(Origin::fromUrl("https://example.com./").serialize() == "https://example.com");
    assert(Origin::fromUrl("https://127.0.0.1:443/").serialize() == "https://127.0.0.1");
    assert(Origin::fromUrl("https://[2001:0db8:0:0:0:0:0:1]:443/").serialize() == "https://[2001:db8::1]");
    assert(Origin::fromUrl("https://[::1]/").sameOrigin(Origin::fromUrl("https://[0:0:0:0:0:0:0:1]:443/")));

    rejects([] { (void)Origin::fromUrl("javascript:alert(1)"); });
    rejects([] { (void)Origin::fromUrl("file:///etc/passwd"); });
    rejects([] { (void)Origin::fromUrl("https://user@example.com/"); });
    rejects([] { (void)Origin::fromUrl("https://example.com:/"); });
    rejects([] { (void)Origin::fromUrl("https://example.com:65536/"); });
    rejects([] { (void)Origin::fromUrl("https://example.com:abc/"); });
    rejects([] { (void)Origin::fromUrl("https://127.000.0.1/"); });
    rejects([] { (void)Origin::fromUrl("https://[::::]/"); });
    rejects([] { (void)Origin::fromUrl("https://bad_host.example/"); });
    rejects([] { (void)Origin::fromUrl("https://example.com/a b"); });

    std::cout << "Canonical HTTP(S) origin tests passed.\n";
}

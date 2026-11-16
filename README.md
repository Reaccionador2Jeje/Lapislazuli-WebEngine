# Lapislazuli WebEngine

**An independent, open web-engine project by AtDev Dev.**

Lapislazuli WebEngine aims to give browser developers a modular foundation for building new browsing experiences. It is intended for independent developers, small teams, experimental browsers, and established projects that need an engine they can study, adapt, and integrate.

Gold & Diamond Browser is the first planned integration target — not the only intended user.

> A new foundation for the web experience.

## Project goals

- **Independent:** develop our own engine components instead of requiring Chromium or Qt WebEngine as the core.
- **Modular:** keep subsystems understandable and reusable so browser projects can integrate them progressively.
- **Portable:** design APIs with desktop, mobile, and specialized platforms in mind.
- **Open:** make the code available for inspection, learning, contributions, and adaptation.
- **Standards-oriented:** implement web standards incrementally and document compatibility honestly.
- **Developer-friendly:** provide clear documentation, examples, tests, and integration guidance.

## Intended users

- Developers creating a new browser.
- Small teams that need a starting point for browser-engine development.
- Experimental and educational browser projects.
- Existing browser projects exploring an alternative engine architecture.
- Developers working on specialized browsing environments.

Lapislazuli is not currently a drop-in replacement for mature browser engines. It is an early-stage project, and web compatibility will grow through clearly defined milestones.

## Architecture

The project is being developed with a combination of **C++ and Rust**. Planned subsystems include HTML tokenization and parsing, DOM, CSS, layout and rendering, JavaScript integration, URL handling, networking, HTTPS, cookies, storage, and browser-facing APIs.

## Current status

**Milestone 0.8 — modular HTTP transport with HTTPS-first security policy.**

The C++ CSS module currently provides:

- CSS tokenization and a limited stylesheet parser.
- Basic type, class, ID, compound, descendant, and child selectors.
- Selector specificity, source ordering, important declarations, and inline styles.
- Basic inheritance for common text-related properties, including color and font size.
- Initial handling of inherit, initial, and unset for the supported property subset.
- Length parsing for unitless numbers, px, %, em, rem, and auto.
- Initial margin and padding shorthand expansion.
- A queryable box-model structure for width, height, margins, padding, borders, and box-sizing.

The C++ JavaScript module introduces a small interpreter-like runtime with let, const, and var declarations (stored in a shared global environment), boolean/null/undefined/numeric/string values, arithmetic and comparison operators, logical operators, parenthesized and unary expressions, registered host functions, built-in print, String, Number, and Boolean functions, an output callback, and a read-only DOM bridge through getElementText(id).

Example:

    let price = 12;
    let quantity = 3;
    let total = price * quantity;
    print('Total:', total);

This is not a complete ECMAScript implementation. Function declarations, closures, arrays, objects, prototypes, modules, promises, exceptions, browser event loops, and most standard built-ins are not implemented. Variable declarations currently share one global environment; const immutability and JavaScript's full coercion and short-circuit rules are not yet enforced. The DOM bridge is read-only, and HTML script elements are not executed automatically. Earlier modules provide only experimental subsets of HTML, CSS, and layout behavior.

## Networking (0.8)

The C++ networking module (lapislazuli_net) uses libcurl as a transport backend and exposes a browser-engine-facing API without exposing libcurl types. It currently provides:

- GET, HEAD, POST, PUT, PATCH, and DELETE requests.
- HTTPS-only requests by default; plaintext HTTP requires explicit opt-in.
- TLS peer and hostname verification enabled by default, with TLS 1.2 as the minimum.
- HTTPS-only redirect policy by default and a configurable redirect limit.
- Connection and total request timeouts.
- A configurable maximum response-body size.
- Response status, normalized response headers, content type, effective URL, redirect metadata, and body.
- Request-header validation that rejects CR/LF injection.
- Basic URL policy checks that reject non-HTTPS schemes and embedded user-info.
- Automatic response decompression through libcurl.

Example:

    #include "lapislazuli/net/http_client.hpp"

    lapislazuli::net::HttpClient client;
    lapislazuli::net::Request request;
    request.url = "https://example.com/";
    const auto response = client.perform(request);

    if (response.statusCode >= 200 && response.statusCode < 300) {
        // The response body can be passed to a later HTML integration layer.
    }

This is an initial transport layer, not a complete browser networking stack. It does not yet implement an origin/security model, DNS rebinding defenses, a complete WHATWG URL parser, cookies, cache, proxy configuration, certificate pinning, HSTS, CSP, service workers, streaming APIs, cancellation, or guaranteed HTTP/2 and HTTP/3 support. URL checks are deliberately limited and must be combined with robust URL parsing and address-resolution policy before untrusted arbitrary navigation is exposed. The allowInsecureHttp option is for controlled development/testing only. Responses are not automatically parsed as HTML and scripts are never executed by the transport layer.

## Build

Requirements: CMake 3.20+, a C++20 compiler, and libcurl development files.

    sudo apt-get install libcurl4-openssl-dev
    cmake -S . -B build
    cmake --build build
    ctest --test-dir build --output-on-failure

### Rust components

Requirements: Rust and Cargo.

    cd engine/rust
    cargo test --workspace

## Roadmap

| Milestone | Focus |
| --- | --- |
| 0.1 | Core foundation and HTML tokenizer |
| 0.2 | HTML parser and DOM foundation |
| 0.3 | HTML attributes, entities, and parser improvements |
| 0.4 | CSS parsing, selectors, and initial cascade |
| 0.5 | CSS values, inheritance, and box-model foundation |
| 0.6 | Initial layout tree and block-flow foundation |
| 0.7 | Experimental JavaScript runtime and DOM bridge |
| 0.8 | HTTP transport, HTTPS defaults, and network security foundations |
| 0.9 | Storage and external browser integrations |
| 1.0 | Stable, documented engine release |

A milestone does not imply complete support for every feature in that area. Compatibility claims will be based on implemented and tested behavior.

## Contributing

Contributions, bug reports, architecture discussions, and documentation improvements are welcome. See [CONTRIBUTING.md](CONTRIBUTING.md) for the project workflow.

When reporting a compatibility issue, include a minimal HTML/CSS/JS example, expected behavior, actual behavior, and your build environment where possible.

## Project philosophy

Lapislazuli WebEngine is intended to lower the barrier to experimenting with browser development. It should remain modular, transparent, and accessible to developers with different levels of resources.

The goal is not to promise an instant replacement for existing engines. The goal is to build a capable engine carefully, subsystem by subsystem, and make that work useful to the wider browser-development community.

## License

Lapislazuli WebEngine is distributed under the MIT License. See [LICENSE](LICENSE).

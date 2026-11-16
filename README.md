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

## 1.0 architecture work — Stage 1: Core Rebuild

The 1.0 preparation work begins by establishing a public native SDK boundary before expanding the browser features. The current repository remains version 0.9.0 while this foundation is developed and validated.

### Runtime and module registry

The new `lapislazuli_core` target provides:

- Engine and public API version metadata.
- A discoverable registry of compiled modules.
- Explicit module maturity labels (`foundation` or `experimental`).
- Stable module identifiers for embedder capability checks.
- A public convenience header: `<lapislazuli/lapislazuli.hpp>`.
- An opt-in symbol visibility macro for future shared-library builds.

The module registry is descriptive metadata. A `foundation` label means the subsystem provides a reusable base API; it is not a certification of security or complete standards compliance. HTML/DOM, CSS, layout, JavaScript, and browser navigation remain experimental.

### Downstream CMake SDK package

CMake now builds an aggregate `Lapislazuli::core` target and exports individual subsystem targets. The install step provides public headers and a relocatable CMake package discoverable through:

    find_package(Lapislazuli CONFIG REQUIRED)
    target_link_libraries(my_browser PRIVATE Lapislazuli::core)

The package declares libcurl and SQLite as dependencies. CI now installs the SDK into a temporary prefix, configures a separate consumer project with `find_package`, builds it, and runs the consumer executable. This checks that the public package works outside the engine's own build tree.

The runtime API is intentionally small at this stage. It does not yet promise ABI stability, shared-library distribution, a complete browser lifecycle, or full web-platform compatibility. Those are separate release criteria for the eventual 1.0.

## 1.0 architecture work — Stage 2: Web Standards foundations

The HTML, CSS, and layout modules now have additional tested behavior:

### HTML tokenizer and parser

- Start-tag attribute parsing for single-quoted, double-quoted, and unquoted values.
- Boolean attributes and first-duplicate-wins handling.
- Self-closing syntax recognition and void-element handling.
- Comments and declaration skipping.
- Common named character references (`amp`, `lt`, `gt`, `quot`, `apos`) and decimal/hex numeric references in text and attribute values.
- Basic recovery for nested paragraphs and list items.

### CSS selectors

- Attribute-presence selectors such as `[disabled]`.
- Exact attribute-value selectors such as `[data-kind="primary"]`.
- Attribute selectors contribute to selector specificity.

### Layout

- Initial flex container layout for row and column directions.
- Basic main-axis start/center/end/space-between distribution.
- Initial cross-axis start/center/end alignment.
- Basic gap spacing and reverse-direction ordering.

These are deliberately limited foundations, not full HTML5 parsing, complete CSS selector support, or a standards-complete Flexbox implementation. The parser does not yet implement the full WHATWG tree-construction algorithm, all character references, raw-text/RCDATA states, foster parenting, or automatic document structure synthesis. CSS attribute matching currently supports presence and exact-value operators only; pseudo-classes and the remaining attribute operators are not implemented. Flex sizing, wrapping, flex-grow/shrink/basis, min/max constraints, and the full alignment specification remain future work.


## Current status

**Milestone 0.9 — persistent origin-partitioned storage and an initial browser-facing navigation facade.**

### Existing experimental modules

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

This is an initial transport layer, not a complete browser networking stack. It does not yet implement a complete WHATWG URL parser, DNS rebinding defenses, cookies, cache, proxy configuration, certificate pinning, HSTS, CSP, service workers, streaming APIs, cancellation, or guaranteed HTTP/2 and HTTP/3 support. URL checks are deliberately limited and must be combined with robust URL parsing and address-resolution policy before untrusted arbitrary navigation is exposed. The allowInsecureHttp option is for controlled development/testing only.

## Persistent storage (0.9)

The lapislazuli_storage module provides a persistent, SQLite-backed key/value area partitioned by an origin identifier:

- getItem, setItem, removeItem, clear, key, length, and usageBytes.
- Per-area configurable byte quota, with transactional quota checks.
- Persistent data across StorageArea instances and process restarts when they use the same database path and origin identifier.
- Separate origin namespaces in the same database.
- Parameterized SQL statements, SQLite busy timeout, and WAL journal mode.
- Empty keys and values are supported; values are accounted for as UTF-8 byte sequences supplied by the embedder.

The embedder must provide a canonical, trusted origin identifier derived from its URL/security layer. StorageArea treats this identifier as an opaque partition key; it does not parse URLs, decide same-origin policy, or establish a security boundary. The current API is a native storage primitive, not a JavaScript localStorage implementation. Storage encryption, eviction policy, private-browsing lifecycle, cross-process quota coordination, and browser privacy controls are not implemented yet.

Example:

    #include "lapislazuli/storage/storage_area.hpp"

    lapislazuli::storage::StorageArea storage(
        "profile/lapislazuli.sqlite", "https://example.com");
    storage.setItem("theme", "dark");
    const auto theme = storage.getItem("theme");

## Browser integration facade (0.9)

The lapislazuli_browser target exposes a small C++ integration point:

    #include "lapislazuli/browser/engine.hpp"

    lapislazuli::browser::Engine engine;
    lapislazuli::browser::NavigationRequest request;
    request.url = "https://example.com/";
    const auto page = engine.navigate(request);

The facade performs an HTTP GET through lapislazuli_net, accepts HTML/XHTML media types, and parses the response body with the current experimental DocumentParser. The result includes the HTTP status, requested and final URL, content type, redirect flag, and a DOM document.

This is an initial integration seam, not a production browser navigation implementation. It does not yet resolve relative subresources, apply CSS, execute document scripts, render pixels, manage history, implement cookies/cache, enforce a complete origin policy, or handle all HTML parsing rules. The parser supports a growing subset of HTML attributes and character references, but still does not implement the full WHATWG parsing algorithm. Do not expose arbitrary untrusted navigation as secure until URL parsing, DNS/address policy, origin enforcement, and the browser security model are implemented.

## Build

Requirements: CMake 3.20+, a C++20 compiler, libcurl development files, and SQLite 3 development files.

Ubuntu/Debian:

    sudo apt-get install libcurl4-openssl-dev libsqlite3-dev
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
| 0.9 | Persistent origin-partitioned storage and browser integration facade |
| 1.0 preparation | Core runtime, public SDK package, integration contracts, and release validation |\n| 1.0 preparation — Stage 2 | HTML attributes/entities, CSS attribute selectors, and initial flex layout |\n| 1.0 | Stable, documented engine release after all acceptance criteria pass |

A milestone does not imply complete support for every feature in that area. Compatibility claims will be based on implemented and tested behavior.

## Contributing

Contributions, bug reports, architecture discussions, and documentation improvements are welcome. See [CONTRIBUTING.md](CONTRIBUTING.md) for the project workflow.

When reporting a compatibility issue, include a minimal HTML/CSS/JS example, expected behavior, actual behavior, and your build environment where possible.

## Project philosophy

Lapislazuli WebEngine is intended to lower the barrier to experimenting with browser development. It should remain modular, transparent, and accessible to developers with different levels of resources.

The goal is not to promise an instant replacement for existing engines. The goal is to build a capable engine carefully, subsystem by subsystem, and make that work useful to the wider browser-development community.

## License

Lapislazuli WebEngine is distributed under the MIT License. See [LICENSE](LICENSE).

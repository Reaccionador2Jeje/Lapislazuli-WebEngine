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
- Basic row main-axis start/center/end/space-between distribution; column containers currently use sequential placement with gap.
- Initial cross-axis start/center/end alignment.
- Basic gap spacing and reverse-direction ordering.

These are deliberately limited foundations, not full HTML5 parsing, complete CSS selector support, or a standards-complete Flexbox implementation. The parser does not yet implement the full WHATWG tree-construction algorithm, all character references, raw-text/RCDATA states, foster parenting, or automatic document structure synthesis. CSS attribute matching currently supports presence and exact-value operators only; pseudo-classes and the remaining attribute operators are not implemented. Flex sizing, wrapping, flex-grow/shrink/basis, min/max constraints, and the full alignment specification remain future work.


## 1.0 architecture work — Stage 3: JavaScript integration

The browser facade now connects the experimental JavaScript runtime to parsed HTML documents:

- The HTML tokenizer treats `script`, `style`, `textarea`, and `title` contents as raw/RCDATA-like text until their matching end tag, so comparison operators and markup-like text inside scripts are not tokenized as HTML.
- Inline classic scripts without a `type` attribute, or with `text/javascript` / `application/javascript`, execute in document order during browser navigation.
- A page-level runtime is shared by the inline scripts in one document, preserving their supported global variables.
- The native DOM bridge supports `getElementText`, `setElementText`, `setElementAttribute`, `getElementAttribute`, `removeElementAttribute`, and `clickElement`.
- `clickElement(id)` dispatches the element's inline `onclick` attribute using the same limited interpreter.
- Script execution results report the number of inline scripts found, successful executions, and per-script errors. A failing script does not prevent later scripts from running.
- At the end of Stage 3, external scripts were not yet fetched; Stage 4 adds ordered loading for supported external classic scripts. JavaScript modules remain unsupported.

The bridge functions are Lapislazuli host APIs, not standard JavaScript `document` objects or standards-compliant DOM methods. The interpreter still has no function declarations, closures, objects, arrays, promises, event loop, timers, or general-purpose event listener API. There is no JavaScript sandbox or standards-complete browser security model yet; this feature must be treated as experimental and must not be presented as safe for arbitrary hostile pages.

## 1.0 architecture work — Stage 4: Resource System

The browser facade now performs an initial document resource pass after parsing HTML:

- Resolves relative, root-relative, query-only, fragment-only, protocol-relative, and absolute HTTPS resource URLs against the final document URL.
- Honors the first usable `base[href]` when resolving resources.
- Loads linked stylesheets from `link[rel~="stylesheet"]` and parses their CSS into `NavigationResult::styleSheets`.
- Parses inline `style` elements into the same stylesheet collection.
- Loads `img[src]` resources and retains their response bytes and MIME metadata in the navigation result.
- Loads external classic JavaScript files in document order and executes them in the same page runtime as inline classic scripts.
- Checks successful HTTP status codes and expected CSS, image, and JavaScript MIME types.
- Keeps per-resource status, URL, content type, success state, body, and error information.
- Applies document-wide resource count and aggregate byte budgets in addition to the HTTP client's per-response limit.
- Rejects malformed or unsupported URL schemes such as `javascript:` in resource references.

This is an initial resource loader, not a complete browser fetch algorithm. URL resolution is a deliberately limited HTTP(S) resolver rather than a complete WHATWG URL implementation. It does not decode image formats or paint images, and downloaded stylesheets are exposed to the embedder but are not yet wired into a full style/layout/paint lifecycle. `srcset`, responsive image selection, media queries, stylesheet alternate/disabled behavior, script modules, async/defer scheduling, dynamic resource insertion, caching, CSP, and a complete origin/security policy remain future work. External scripts run in the experimental interpreter and must not be considered safe for arbitrary untrusted pages.

## 1.0 architecture work — Stage 5: Security and origin foundations

The new `lapislazuli_security` module introduces a constrained HTTP(S) tuple-origin API:

- Parses absolute HTTP and HTTPS URLs into scheme, host, and effective port.
- Normalizes scheme and ASCII DNS host casing, removes a trailing DNS dot, and omits default ports when serializing.
- Validates canonical dotted-decimal IPv4 hosts and bracketed IPv6 literals; IPv6 zero compression is normalized for same-origin comparisons.
- Rejects user-info, unsupported schemes, malformed authorities, invalid ports, control characters, whitespace, and unsupported host syntax.
- Provides `Origin::sameOrigin()` for comparing validated tuple origins.
- Adds the final response origin to `NavigationResult`, so redirects use the final URL's origin.
- Adds a preferred `StorageArea` constructor that accepts a validated `security::Origin` and uses its canonical serialization as the SQLite partition key.
- Registers the security module in the public SDK module registry and exports it as `Lapislazuli::security`.

The original string-based storage constructor remains available as a low-level compatibility API. Callers using it are still responsible for supplying a canonical, trusted partition key.

This is a security foundation, not a complete browser security architecture. The parser is intentionally constrained: it is not a complete WHATWG URL parser, does not implement IDNA conversion, and does not cover every IPv6 address form. Same-origin comparison is available but is not yet enforced across all DOM, script, storage, and resource APIs. DNS resolution/address pinning, rebinding protection, mixed-content policy, CSP, sandboxing, cookie policy, permissions, and process isolation remain future work. Do not treat the current engine as safe for arbitrary untrusted pages.

## 1.0 architecture work — Stage 6: Rendering pipeline foundations

The browser navigation result now contains a computed style tree, a layout tree, and a backend-neutral display list:

- The navigation request accepts a viewport size.
- Linked and inline stylesheets are combined for the current style-resolution pass.
- The style resolver computes the supported cascade for the parsed document.
- The layout engine produces the initial box tree for that viewport.
- The rendering module converts layout boxes and computed styles into ordered drawing commands.
- Supported commands include rectangle fills, rectangle strokes, text drawing, and image placement.
- Initial color handling includes a small named-color subset and hexadecimal RGB/RGBA forms.
- Text commands carry color, font size, bold, and italic metadata.
- Image commands carry their resource URL and layout bounds.
- The new `lapislazuli_render` module is exported as `Lapislazuli::render` and registered in the runtime module list.

The display list is a renderer-independent handoff for native embedders; it does not itself draw pixels. There is no GPU or software raster backend yet. Font shaping, glyph measurement, advanced line breaking, stacking contexts, z-index, clipping, transforms, opacity, gradients, shadows, full border styles, image decoding, and final pixel output remain future work. The current text layout still estimates glyph widths, and the supported CSS color/paint properties are intentionally limited.

## 1.0 architecture work — Stage 7: Browser Integration SDK

The public browser SDK now provides a reusable `browser::Browser` context and a move-only `browser::Page` result:

- `BrowserConfig` groups network configuration, an optional profile database path, and the viewport.
- `Browser::navigate()` performs navigation through the existing HTML, resource, style, layout, and display-list pipeline.
- `Page` provides access to the navigation result, validated origin, DOM document, layout tree, display list, and resource records.
- When a profile database path is configured, each returned page receives a persistent `StorageArea` partitioned by its validated canonical origin.
- `Browser::openStorage(origin)` lets embedders open an origin partition directly.
- Omitting the profile database path disables page storage; callers do not get an implicit profile file created.
- The convenience SDK header now includes the Browser and Page APIs.

Example:

    #include <lapislazuli/lapislazuli.hpp>

    lapislazuli::browser::BrowserConfig config;
    config.storageDatabasePath = "profile/browser.sqlite";
    config.viewport = {1280, 800, 16};

    lapislazuli::browser::Browser browser(config);
    auto page = browser.navigate("https://example.com/");

    const auto& origin = page.origin();
    const auto& drawing = page.displayList();
    if (auto* storage = page.storage()) {
        storage->setItem("last-visited", page.navigation().finalUrl);
    }

This is an initial native embedder SDK, not a complete browser application framework. It does not yet provide tabs, navigation history, back/forward, downloads, permission prompts, profile encryption, cache management, private sessions, or a stable 1.0 ABI. Mutating the returned DOM does not automatically rerun style calculation, layout, or display-list generation; embedders should treat the generated rendering data as a snapshot.

## 1.0 architecture work — Stage 8: Compatibility foundations

The browser engine now exposes a local-source processing path and a curated end-to-end compatibility test:

- Engine::loadHtml(source, documentUrl, viewport) processes already available HTML without requiring a top-level network request.
- Browser::loadHtml(source, documentUrl) exposes the same pipeline through the higher-level SDK and attaches origin-partitioned storage when a profile is configured.
- Both local and network navigation share the same parse, resource, script, style, layout, and display-list pipeline.
- Non-rendered document elements such as head, style, script, meta, link, title, base, and template are excluded from layout.
- A compatibility smoke test verifies HTML parsing, inline JavaScript DOM changes, CSS parsing, computed styles, viewport layout, and display-list generation as one flow.
- docs/COMPATIBILITY.md records supported subsets and major gaps for each subsystem.

The compatibility suite is a project-owned smoke-test set, not the official Web Platform Tests suite. Passing it proves only the behaviors covered by those tests; it is not a browser compatibility score or standards certification.

## 1.0 architecture work — Stage 9: Release-candidate preparation

The downstream CMake consumer test now exercises the installed SDK rather than only reading version metadata. The C++ workflow also has separate AddressSanitizer/UndefinedBehaviorSanitizer and Clang libFuzzer jobs. HTML parsing and origin/resource URL handling receive 1,000-iteration fuzz smoke runs from committed seed corpora. URL and request-header validation rejects embedded NUL and other forbidden control characters. Local HTML processing rejects documents larger than 16 MiB before parsing.

- Links the exported `Lapislazuli::core`, `Lapislazuli::browser`, `Lapislazuli::render`, and `Lapislazuli::security` targets.
- Includes the public convenience header from the install prefix.
- Creates a Browser context and processes a local HTML document.
- Verifies a supported inline JavaScript DOM mutation.
- Verifies the canonical page origin, generated display list, and profile-backed storage.
- Removes its temporary SQLite profile after the test.

A new `docs/RELEASE_CHECKLIST.md` records what is implemented and what must still be completed before a 1.0 release. Configure a local sanitizer build with `-DLAPISLAZULI_ENABLE_SANITIZERS=ON` to run the suite with ASan and UBSan. Clang fuzz targets can be enabled with `-DLAPISLAZULI_BUILD_FUZZERS=ON` alongside the sanitizer option. Stage 9 is preparation only: this project remains version 0.9.0, and the checklist explicitly blocks a production-ready 1.0 claim while rasterization, broad compatibility, and critical security work remain incomplete.

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

The C++ JavaScript module introduces a small interpreter-like runtime with let, const, and var declarations (stored in a shared global environment), boolean/null/undefined/numeric/string values, arithmetic and comparison operators, logical operators, parenthesized and unary expressions, registered host functions, built-in print, String, Number, and Boolean functions, an output callback, and a mutable host DOM bridge. The browser facade can execute supported inline and external classic scripts and dispatch basic inline onclick handlers.

Example:

    let price = 12;
    let quantity = 3;
    let total = price * quantity;
    print('Total:', total);

This is not a complete ECMAScript implementation. Function declarations, closures, arrays, objects, prototypes, modules, promises, exceptions, browser event loops, and most standard built-ins are not implemented. Variable declarations currently share one global environment; const immutability and JavaScript's full coercion and short-circuit rules are not yet enforced. The DOM bridge is limited to named host functions rather than the standard JavaScript document API. Script execution supports classic inline and external scripts only through the documented interpreter subset. Earlier modules provide only experimental subsets of HTML, CSS, and layout behavior.

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

The lapislazuli_browser target exposes the lower-level Engine facade and the higher-level Browser/Page SDK:

    #include "lapislazuli/browser/engine.hpp"

    lapislazuli::browser::Engine engine;
    lapislazuli::browser::NavigationRequest request;
    request.url = "https://example.com/";
    const auto page = engine.navigate(request);

The facade performs an HTTP GET through lapislazuli_net, accepts HTML/XHTML media types, parses the response body, executes supported inline and external classic scripts, discovers linked stylesheets and images, computes styles and layout for the requested viewport, and builds a backend-neutral display list. The result includes navigation metadata, the canonical origin, the DOM, script diagnostics, resource records, parsed stylesheets, computed styles, layout boxes, and drawing commands.

This is an initial integration seam, not a production browser navigation implementation. It does not yet rasterize pixels, decode image bytes, provide a GPU/window backend, manage history, implement cookies/cache, enforce a complete origin policy, or handle all HTML parsing rules. The parser supports a growing subset of HTML attributes and character references, but still does not implement the full WHATWG parsing algorithm. Do not expose arbitrary untrusted navigation as secure until URL parsing, DNS/address policy, origin enforcement, and the browser security model are implemented.

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
| 1.0 preparation | Core runtime, public SDK package, integration contracts, and release validation |
| 1.0 preparation — Stage 2 | HTML attributes/entities, CSS attribute selectors, and initial flex layout |
| 1.0 preparation — Stage 3 | Inline JavaScript execution and mutable DOM bridge |
| 1.0 preparation — Stage 4 | Relative resource URLs, linked CSS, images, and external classic scripts |
| 1.0 preparation — Stage 5 | Canonical HTTP(S) origins and origin-keyed storage integration |
| 1.0 preparation — Stage 6 | Computed styles, viewport layout, and backend-neutral display lists |
| 1.0 preparation — Stage 7 | Public Browser/Page SDK and profile-backed origin storage |
| 1.0 preparation — Stage 8 | Local document processing, end-to-end compatibility tests, and support matrix |
| 1.0 preparation — Stage 9 | Installed SDK consumer validation and release-readiness checklist |
| 1.0 | Stable, documented engine release after all acceptance criteria pass |

A milestone does not imply complete support for every feature in that area. Compatibility claims will be based on implemented and tested behavior.

## Contributing

Contributions, bug reports, architecture discussions, and documentation improvements are welcome. See [CONTRIBUTING.md](CONTRIBUTING.md) for the project workflow.

When reporting a compatibility issue, include a minimal HTML/CSS/JS example, expected behavior, actual behavior, and your build environment where possible.

## Project philosophy

Lapislazuli WebEngine is intended to lower the barrier to experimenting with browser development. It should remain modular, transparent, and accessible to developers with different levels of resources.

The goal is not to promise an instant replacement for existing engines. The goal is to build a capable engine carefully, subsystem by subsystem, and make that work useful to the wider browser-development community. See [docs/COMPATIBILITY.md](docs/COMPATIBILITY.md) for the tested subset and known gaps, and [docs/RELEASE_CHECKLIST.md](docs/RELEASE_CHECKLIST.md) for the 1.0 acceptance gates.

## License

Lapislazuli WebEngine is distributed under the MIT License. See [LICENSE](LICENSE).

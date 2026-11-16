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

The project is being developed with a combination of **C++ and Rust**.

Planned subsystems include HTML tokenization and parsing, DOM tree and document APIs, CSS tokenization and parsing, selectors and cascade, layout and rendering, JavaScript integration, URL handling and networking, HTTPS, cookies, storage, and browser-facing APIs.

## Current status

**Milestone 0.4 — experimental CSS engine foundation.**

The C++ CSS module currently provides:
- CSS tokenization for identifiers, hashes, strings, punctuation, whitespace, and comments.
- Stylesheet parsing and declaration extraction.
- Basic type, class, ID, compound, descendant, and child selectors.
- Selector specificity and comma-separated selector lists.
- Initial cascade ordering, !important, and inline style declarations.
- Computed-style lookup for individual elements and whole DOM trees.

This is an intentionally small CSS subset, not a complete CSS Syntax or Selectors specification implementation. Pseudo-classes, attribute selectors, namespaces, custom-property substitution, inheritance, shorthand expansion, media queries, and the full cascade/layer model are not implemented. Computed values are retained as strings; this milestone does not perform layout or paint.

## Build

Requirements: CMake 3.20+ and a C++20 compiler.

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
| 0.5 | CSS values, inheritance, and box-model layout |
| 0.6 | Rendering pipeline |
| 0.7 | JavaScript integration |
| 0.8 | Networking and secure transport |
| 0.9 | Storage and external browser integrations |
| 1.0 | Stable, documented engine release |

A milestone does not imply complete support for every feature in that area. Compatibility claims will be based on implemented and tested behavior.

## Contributing

Contributions, bug reports, architecture discussions, and documentation improvements are welcome. See CONTRIBUTING.md for the project workflow.

When reporting a compatibility issue, include a minimal HTML/CSS/JS example, expected behavior, actual behavior, and your build environment where possible.

## Project philosophy

Lapislazuli WebEngine is intended to lower the barrier to experimenting with browser development. It should remain modular, transparent, and accessible to developers with different levels of resources.

The goal is not to promise an instant replacement for existing engines. The goal is to build a capable engine carefully, subsystem by subsystem, and make that work useful to the wider browser-development community.

## License

Lapislazuli WebEngine is distributed under the MIT License. See LICENSE.

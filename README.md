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

Planned subsystems include:

- HTML tokenization and parsing
- DOM tree and document APIs
- CSS parsing, cascade, and layout
- Rendering and compositing
- JavaScript integration
- URL handling and networking
- HTTPS and certificate validation
- Cookies and web storage
- Browser-facing integration APIs

The exact boundaries between components may evolve as the engine develops.

## Current status

**Early prototype — not production-ready.**

The initial foundation includes:

- A basic C++ HTML tokenizer prototype.
- A minimal Rust URL parser prototype.
- Initial CMake and Cargo workspace configuration.
- Basic C++ and Rust unit-test scaffolding.

These prototypes intentionally cover only a small subset of web behavior. They are not yet standards-compliant implementations and must not be treated as security boundaries.

## Build

### C++ components

Requirements: a C++20 compiler and CMake.

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

### Rust components

Requirements: Rust and Cargo.

```sh
cd engine/rust
cargo test --workspace
```

## Roadmap

The roadmap is directional and may change as technical work progresses.

| Milestone | Focus |
| --- | --- |
| 0.1 | Core foundation and HTML tokenizer |
| 0.2 | HTML parser and DOM |
| 0.3 | CSS parsing, cascade, and initial layout |
| 0.4 | Rendering pipeline |
| 0.5 | JavaScript integration |
| 0.6 | Networking and secure transport |
| 0.7 | Cookies, storage, and browser integration APIs |
| 0.8 | First external browser integrations and compatibility work |
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

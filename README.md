# Lapislazuli WebEngine

**An independent, open web-engine project by AtDev Dev.**

Lapislazuli provides a modular foundation for independent developers, small teams, experimental browsers, and established projects. Gold & Diamond Browser is the first planned integration target, not the only intended user.

> A new foundation for the web experience.

## Goals
- Independent engine components, without Chromium or Qt WebEngine as the core.
- Modular, portable APIs for desktop, mobile, and specialized browsers.
- Open development, documentation, tests, and community contributions.
- Standards-oriented implementation with honest compatibility reporting.

## Current status

**Milestone 0.3 — experimental HTML parser improvements.**

The C++ implementation now includes:
- Tag tokenization with quoted, unquoted, and boolean attributes.
- Correct handling of greater-than characters inside quoted attribute values.
- Basic doctype/declaration skipping.
- DOM construction with parsed attributes.
- Character-reference decoding for amp, lt, gt, quot, apos, nbsp, and numeric decimal/hex references.
- UTF-8 encoding for numeric references and first-duplicate-attribute-wins behavior.
- Self-closing and void-element handling.
- Expanded tokenizer and DOM/parser tests.

This is not a complete WHATWG HTML parser. The full named character-reference table, complete tree-construction algorithm, raw-text/RCDATA states, and browser-grade error recovery are not implemented. Unknown named references remain literal. Do not use this prototype as a security boundary or production parser.

## Build

Requirements: CMake 3.20+ and a C++20 compiler.

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Rust workspace tests:

```sh
cd engine/rust
cargo test --workspace
```

## Roadmap
| Milestone | Focus |
| --- | --- |
| 0.1 | Core foundation and HTML tokenizer |
| 0.2 | DOM tree and document APIs |
| 0.3 | HTML attributes, character references, parser improvements |
| 0.4 | CSS parsing, cascade, and initial layout |
| 0.5 | Rendering pipeline |
| 0.6 | JavaScript integration |
| 0.7 | Networking and secure transport |
| 0.8 | Cookies, storage, browser integration APIs |
| 0.9 | External browser integrations and compatibility |
| 1.0 | Stable, documented engine release |

## Contributing
See CONTRIBUTING.md. Bug reports should include a minimal reproduction, expected and actual behavior, and build environment.

## License
MIT. See LICENSE.

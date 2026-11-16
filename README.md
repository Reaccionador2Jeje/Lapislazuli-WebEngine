# Lapislazuli WebEngine

An independent experimental web engine by AtDev Dev, designed for Gold & Diamond Browser.

## Status
**Version:** 0.1.0-dev · **Stage:** Foundation · **Languages:** C++20 and Rust.

Lapislazuli is a modular engine prototype. This milestone introduces a basic HTML tokenizer in C++ and an initial absolute-URL parser in Rust. It is not yet suitable for rendering modern websites.

## Build C++
Requirements: CMake 3.20+ and a C++20 compiler.

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## Test Rust
Requirements: stable Rust and Cargo.

```sh
cd engine/rust
cargo test --workspace
```

## Current foundations
- Basic HTML tokenization: text, start tags, end tags, comments.
- Minimal absolute URL parsing.
- CMake and Cargo build/test foundations.

## Not implemented
CSS parsing/layout, DOM construction, JavaScript execution, networking, TLS, cookies, storage, image decoding, accessibility, and page painting.

**Security notice:** This is experimental software. Do not use it for sensitive browsing. The URL parser is not WHATWG-compliant and is not a security boundary.

## License
MIT. See [LICENSE](LICENSE).

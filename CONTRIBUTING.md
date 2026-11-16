# Contributing to Lapislazuli WebEngine

Thank you for your interest in contributing.

Lapislazuli WebEngine is an early-stage, independent web-engine project by AtDev Dev. Contributions from browser developers, systems programmers, students, testers, and documentation writers are welcome.

## Before opening a change

1. Read the project README and understand the current prototype status.
2. Check existing issues and pull requests to avoid duplicate work.
3. Keep changes focused. Prefer small, reviewable contributions.
4. Do not claim standards compliance unless the relevant behavior is implemented and tested.

## Development guidelines

- Keep C++ code compatible with the project's configured C++ standard.
- Keep Rust code formatted with `cargo fmt` and free of avoidable warnings.
- Add or update tests for behavior changes.
- Document limitations and unsupported cases.
- Avoid introducing Chromium or Qt WebEngine as a required engine dependency.
- Treat parsing, URL handling, networking, and security-sensitive behavior conservatively.
- Do not describe an experimental parser as a security boundary.

## Testing

Run the relevant checks before submitting a pull request:

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure

cd engine/rust
cargo fmt --check
cargo test --workspace
```

If a check cannot be run in your environment, mention that clearly in the pull request.

## Pull requests

A useful pull request should include:

- A concise explanation of the problem and proposed change.
- The affected subsystem.
- Tests or a reason tests are not applicable.
- Known limitations or compatibility implications.

## Issues and bug reports

For bugs, provide a minimal reproducible example whenever possible, along with the operating system, compiler/toolchain versions, build steps, and relevant output.

Please be respectful and constructive. The project aims to be a welcoming place for people learning about and building browser technology.

# 1.0 release-candidate checklist

**Current line:** 0.9.0 development / Stage 9 preparation  
**Release status:** Not a release candidate yet

This checklist is an acceptance gate, not a claim that 1.0 is complete.

## SDK and packaging

- [x] C++20 CMake package with exported subsystem targets.
- [x] Relocatable install tree and generated package version metadata.
- [x] Dependency discovery for libcurl and SQLite.
- [x] Public convenience header.
- [x] Downstream consumer build in CI.
- [ ] ABI policy and shared-library compatibility guarantees.
- [ ] Install and consumer validation on Linux, Windows, and macOS.

## Core web pipeline

- [x] HTML source to DOM for the implemented parser subset.
- [x] Inline classic JavaScript execution and limited host DOM bridge.
- [x] Initial CSS parsing, cascade, and computed styles.
- [x] Initial viewport layout and flex foundations.
- [x] Resource loading for linked CSS, images, and external classic scripts.
- [x] Backend-neutral display-list generation.
- [ ] Pixel rasterizer and native rendering backend.
- [ ] Broader HTML/CSS/ECMAScript compatibility coverage.
- [ ] Official WPT subset integration with documented expected failures.

## Security and storage

- [x] HTTPS-first transport defaults and TLS verification.
- [x] Bounded response and document-resource budgets.
- [x] Canonical HTTP(S) tuple-origin helper.
- [x] Origin-partitioned persistent storage API.
- [ ] Full URL parser and robust address-resolution/rebinding defenses.
- [ ] Enforced same-origin policy across all browser APIs.
- [ ] CSP, sandboxing, cookie policy, and permission model.
- [ ] Security review before arbitrary untrusted browsing is recommended.

## Quality and release operations

- [x] Unit and end-to-end smoke tests in CI.
- [x] Installed SDK consumer test.
- [x] AddressSanitizer and UndefinedBehaviorSanitizer CI build and 14-test suite.
- [x] Clang libFuzzer harnesses for HTML and URL parsing, with 1,000-iteration CI smoke runs each.
- [ ] Long-duration scheduled fuzzing, crash-corpus retention, and replay automation.
- [ ] Cross-platform CI.
- [ ] Documented compatibility profile and migration notes.
- [ ] Release notes, version/tag policy, and final API review.

## Release rule

Do not publish version 1.0 or describe Lapislazuli as a production-ready general-purpose browser engine until the remaining critical security, rendering, compatibility, and release-operation items have been addressed and verified.

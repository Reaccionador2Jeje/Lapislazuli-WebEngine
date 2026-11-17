# Lapislazuli WebEngine compatibility profile

This document describes the implemented subset as it exists in the current 0.9.0 development line. It is an honest feature inventory, not a claim of standards certification.

## Compatibility summary

| Area | Current support | Important gaps |
| --- | --- | --- |
| HTML | Partial tokenizer/parser, common attributes/entities, void elements, basic paragraph/list recovery, raw text for script/style | Full WHATWG tree construction, full named entity table, foster parenting, implied document structure |
| DOM | Element/text/comment nodes, attributes, traversal, text content, append/remove child | Full DOM Standard, ranges, mutation observers, namespaces, shadow DOM |
| CSS | Basic type/class/ID/descendant/child/attribute selectors, limited cascade and inherited values | Pseudo-classes/elements, most attribute operators, full cascade layers, variables, media queries |
| Layout | Initial block flow, box model, basic row/column flex layout | Complete Flexbox, Grid, inline formatting, intrinsic sizing, fragmentation, advanced positioning |
| JavaScript | Small interpreter subset, inline/external classic scripts, host DOM bridge | Full ECMAScript, standard document object, modules, async/defer scheduling, event loop, promises |
| Networking | libcurl transport, HTTPS defaults, TLS verification, redirects and response limits | Complete WHATWG URL/fetch, DNS rebinding defense, cookies, cache, CSP, service workers |
| Resources | Relative HTTPS URL resolution, linked CSS, image bytes, external classic scripts | srcset, full URL algorithm, @import, dynamic loading, image decoding, caching |
| Security | Canonical HTTP(S) tuple origins and same-origin comparison helper, origin-keyed storage constructor | Full same-origin enforcement, IDNA, DNS pinning, CSP, sandboxing, process isolation |
| Rendering | Backend-neutral display list for fills, strokes, text and image placement | Pixel rasterizer, GPU backend, font shaping, full paint/compositing model |
| Browser SDK | Browser/Page context, navigation, local HTML processing, profile-backed origin storage | Tabs, history, downloads, permissions, private profiles, stable 1.0 ABI |

## Compatibility test strategy

The C++ test suite includes a curated end-to-end compatibility smoke test. It feeds HTML through the parser, executes a supported inline script, parses inline CSS, computes styles, creates layout boxes, and generates a display list. Separate tests cover URL resolution, origins, resource parsing, storage partitioning, and the public Browser SDK.

These are project-owned tests for the supported subset. They are not imported Web Platform Tests (WPT), do not measure browser market compatibility, and do not establish complete standards conformance.

## Safe compatibility claims

Use phrases such as:

- "Supports a tested subset of HTML, CSS, and JavaScript."
- "Can generate a backend-neutral display list for the implemented layout subset."
- "Provides an initial HTTP(S) resource loader and origin-partitioned storage primitive."

Do not claim:

- Full HTML5/CSS3/JavaScript support.
- Full WPT or Acid test compliance.
- A production-secure browser engine.
- Pixel-identical rendering to Chromium, Firefox, or WebKit.

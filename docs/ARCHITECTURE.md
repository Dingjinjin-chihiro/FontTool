# Architecture

## Layering

```text
Consumer application
        │
        ▼
include/fonttool/fonttool.hpp       Stable public C++17 API
        │
        ▼
src/fonttool.cpp                    Validation, status mapping, PImpl adapter
        │
        ▼
src/internal/podofo/main            Font loading and facade
        │
        ├── FreeType                Face parsing, charmaps, metrics
        ├── TrueType subsetter      glyf/loca/hmtx rebuild
        └── AFDKO subsetter         Type1/CFF conversion and CID-keyed CFF output
```

Private headers are never installed. Consumers do not depend on the internal `PoDoFo` namespace or AFDKO APIs.

## Data ownership

Both file and memory loading end in an owned byte buffer inside `FontMetricsFreetype`. FreeType faces are created over that owned storage, so caller buffers need not outlive `load_memory()`.

## TrueType subset path

1. Resolve requested Unicode/CID/GID inputs.
2. Add `.notdef` and recursively discover components of compound glyphs.
3. Choose compact or retained GID layout.
4. Rebuild `glyf`, `loca`, `hmtx`, `maxp`, `hhea` and `head`.
5. Preserve selected safe tables such as `name`, `post` and optional hinting tables.
6. Recalculate table checksums and `head.checkSumAdjustment`.

The source cmap can only be copied when all referenced glyph slots remain valid.

## CFF/Type1 subset path

AFDKO parses Type1 or CFF input and emits a CID-keyed CFF font program for requested glyphs. This is a raw CFF program, not an OTF wrapper. The result includes CID-to-subset-GID mappings required by PDF consumers.

## Error boundary

The public layer converts internal failures into `ErrorCode` plus diagnostic text. Exceptions are not propagated through the public API. UTF-8 decoding is isolated so decoding failures are returned as `InvalidUtf8`.

## Extension points

New backends should be implemented behind the internal facade, then mapped to existing public result structures. Public API additions should preserve source compatibility within the same major version.

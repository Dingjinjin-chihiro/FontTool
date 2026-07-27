# Contributing

Contributions should keep the public API small, non-throwing and independent of PDF object models.

## Local checks

```bash
cmake --preset release -DFONTTOOL_WARNINGS_AS_ERRORS=ON
cmake --build --preset release
ctest --preset release
```

Changes to font parsing or subsetting should include a regression test. Do not commit proprietary fonts; tests must use system fonts or redistributable test fixtures with their license included.

## Source layout

- `include/fonttool`: stable public API
- `src/fonttool.cpp`: public-to-internal adapter
- `src/internal/podofo`: font-only PoDoFo-derived implementation
- `third_party`: bundled source dependencies
- `src/cli`: optional command-line application

Run `clang-format` on new public, CLI, example and test code. Preserve existing SPDX identifiers and third-party notices.

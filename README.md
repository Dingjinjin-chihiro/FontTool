# FontTool

FontTool is an embeddable C++17 library and optional CLI for font inspection and subsetting. It is extracted from a larger PDF-oriented codebase, but has no dependency on PDF document objects, XML, or Fontconfig. FreeType 2 is the only required external dependency.

## Features

- Load fonts from files or memory.
- Enumerate faces in TTC/OTC collections.
- Inspect family, style, PostScript name, format, glyph count and units-per-em.
- Resolve Unicode code points to glyph IDs and query normalized advances.
- Subset from UTF-8 text, Unicode code points, source GIDs or explicit CIDs.
- TrueType `glyf/loca` and Type1/CFF subsetting.
- Source-to-subset GID and CID mappings.
- Static/shared library builds, optional CLI, tests and examples.
- Integration through `add_subdirectory`, installed CMake packages or pkg-config.

CFF and Type1 inputs produce a raw CID-keyed CFF font program. They are not repackaged as OpenType files. CFF2, WOFF/WOFF2 output, variable-font instancing and GSUB/GPOS rewriting are not currently supported.

## Build

Requirements: CMake 3.20+, a C++17 compiler and FreeType 2 development files.

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Shared build:

```bash
cmake -S . -B build-shared -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_SHARED_LIBS=ON
cmake --build build-shared --parallel
```

A `vcpkg.json` manifest and CMake presets are included.

## Embed with CMake

```cmake
add_subdirectory(external/FontTool)
target_link_libraries(your_target PRIVATE FontTool::FontTool)
```

When embedded, the CLI, tests, examples and install rules default to `OFF`.

## Install and consume

```bash
cmake --install build --prefix /opt/fonttool
```

```cmake
find_package(FontTool 1 CONFIG REQUIRED)
add_executable(app main.cpp)
target_link_libraries(app PRIVATE FontTool::FontTool)
```

Configure the consumer with `-DCMAKE_PREFIX_PATH=/opt/fonttool` when the prefix is non-standard.

## Minimal example

```cpp
#include <fonttool/fonttool.hpp>

#include <iostream>

int main() {
    fonttool::FontTool tool;
    auto status = tool.load_file("input.ttf");
    if (!status) {
        std::cerr << status.message << '\n';
        return 1;
    }

    fonttool::SubsetResult subset;
    status = tool.subset_utf8("Hello, FontTool!", {}, subset);
    if (!status) {
        std::cerr << status.message << '\n';
        return 1;
    }

    status = fonttool::FontTool::write_file("subset.ttf", subset);
    return status ? 0 : 1;
}
```

## CLI

```bash
fonttool info input.ttf
fonttool faces collection.ttc --json
fonttool subset input.ttf -o subset.ttf --text "Hello world"
fonttool subset input.otf -o subset.cff --text-file characters.txt
fonttool subset input.ttf -o subset.ttf --gids 1,2,36 --retain-gids
```

See [README_ZH.md](README_ZH.md), [API documentation](docs/API.md), [integration guide](docs/INTEGRATION.md) and [architecture notes](docs/ARCHITECTURE.md) and [migration guide](docs/MIGRATION_FROM_COMPRESS.md).

## License

New FontTool wrapper code is MPL-2.0. PoDoFo-derived files retain `LGPL-2.0-or-later OR MPL-2.0`. Bundled third-party sources retain their upstream licenses. See `LICENSE`, `NOTICE`, `THIRD_PARTY_NOTICES.md` and `LICENSES/`.

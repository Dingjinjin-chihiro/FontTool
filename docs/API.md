# Public API

The installed API consists only of headers under `include/fonttool`. Everything under `src/internal` is private and may change without an API-version change.

## Status

Every fallible operation returns `fonttool::Status`:

```cpp
auto status = tool.load_file(path);
if (!status) {
    log(fonttool::to_string(status.code), status.message);
}
```

`message` is diagnostic text and should not be parsed. Program logic should use `ErrorCode`.

## FontTool lifetime

`FontTool` is move-only and owns the loaded font data. Calling another `load_*` operation or `reset()` discards the previous face. A moved-from object is valid only for destruction or move assignment.

## Loading and inspection

- `load_file(path, face_index)` loads one face.
- `load_memory(data, size, face_index)` copies the font bytes.
- `inspect_file` and `inspect_memory` enumerate all faces without keeping one loaded.
- `find_face_index` matches names exposed by FreeType.
- `info()` returns an empty `FaceInfo` when no face is loaded.

## Glyph queries

`glyph_id()` accepts Unicode scalar values only. Surrogates and values above U+10FFFF return `InvalidArgument`. A valid scalar not present in the selected charmap returns `MissingGlyph`.

`glyph_advance()` returns horizontal advance divided by units-per-em. For example, `0.5` means half an em.

## Subset inputs

### UTF-8

```cpp
fonttool::SubsetResult result;
auto status = tool.subset_utf8("Hello 世界", {}, result);
```

Invalid UTF-8 returns `InvalidUtf8`; characters absent from the font return `MissingGlyph`.

### Unicode code points

```cpp
std::vector<char32_t> codepoints = {U'A', U'B', U'中'};
tool.subset_codepoints(codepoints, {}, result);
```

### Source glyph IDs

```cpp
std::vector<std::uint32_t> gids = {36, 37, 38};
tool.subset_glyphs(gids, {}, result);
```

The current font formats use 16-bit source glyph IDs. Values above 65535 are rejected.

### Explicit CIDs

```cpp
std::vector<fonttool::CidGlyphMapping> mappings = {
    {1, 36, U"A"},
    {2, 37, U"B"},
};
fonttool::CidSystemInfo ros{"Adobe", "Identity", 0};
tool.subset_cids(mappings, {}, result, ros);
```

CID 0 is reserved for `.notdef` and must not be supplied.

## Subset output

- `font_program`: generated binary font program.
- `format`: actual output format; do not infer it from the input extension.
- `glyph_mappings`: source GID to subset GID.
- `cid_mappings`: CID to subset GID.
- `identity_cid_to_gid`: whether a PDF consumer may use `/CIDToGIDMap /Identity`.
- `retains_original_gids`: source and output GIDs are equal.
- `retains_original_glyph_count`: the output retains source glyph slots.

Always use returned mappings unless the corresponding identity/retention flag explicitly permits omission.

## Option invariants

| Option | Required companion options |
|---|---|
| `retain_gids` | none |
| `retain_glyph_count` | `retain_gids` |
| `include_cmap` | `retain_gids`, `retain_glyph_count` |
| `include_hinting` | none |

`include_cmap` preserves the source cmap rather than synthesizing a new one. It is therefore only safe when source GID references remain valid.

## Thread safety

Separate `FontTool` instances can be used from separate threads. A single instance is stateful and requires external synchronization for concurrent access.

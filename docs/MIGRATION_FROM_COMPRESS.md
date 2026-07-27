# Migration from the Compress-integrated FontTool

The extracted project intentionally removes the public `PoDoFo` namespace and all PDF-oriented headers. Applications should include only:

```cpp
#include <fonttool/fonttool.hpp>
```

## Target migration

Old integration commonly linked an internal target or manually listed source files. Replace it with:

```cmake
add_subdirectory(external/FontTool)
target_link_libraries(Compress PRIVATE FontTool::FontTool)
```

Do not add `src/internal`, AFDKO, utf8proc or FreeType include directories to the consumer target. FontTool manages those privately.

## API mapping

| Old internal call | New public call |
|---|---|
| `PoDoFo::FontToolsFacade::LoadFontFile` | `fonttool::FontTool::load_file` |
| `LoadFontBuffer` | `load_memory` |
| `Reset` / `IsLoaded` | `reset` / `is_loaded` |
| `GetAllFontFaces` | `FontTool::inspect_file` or `inspect_memory` |
| `GetFontIndexByName` | `FontTool::find_face_index` |
| `SubsetUtf8` | `subset_utf8` |
| `SubsetUnicode` | `subset_codepoints` |
| `SubsetGids` | `subset_glyphs` |
| `SubsetCids` | `subset_cids` |
| `package.Subset.FontProgram` | `result.font_program` |
| `OriginalToSubsetGid` | `result.glyph_mappings` |
| `CidToSubsetGid` | `result.cid_mappings` |

## Error handling

Old calls returned `bool` and optionally wrote an error string. New calls return a structured status:

```cpp
auto status = tool.subset_utf8(text, options, result);
if (!status) {
    report(fonttool::to_string(status.code), status.message);
}
```

## PDF integration

For TrueType output, write `result.font_program` as the embedded font stream and use `glyph_mappings` when GIDs were compacted.

For Type1/OpenType-CFF input, the output is CID-keyed CFF. Use `cid_mappings` to construct the PDF CID-to-GID relationship. `identity_cid_to_gid` explicitly reports whether `/CIDToGIDMap /Identity` is valid.

The independent library deliberately does not create PDF dictionaries, descriptors, widths arrays or ToUnicode CMaps. Those remain responsibilities of the PDF layer.

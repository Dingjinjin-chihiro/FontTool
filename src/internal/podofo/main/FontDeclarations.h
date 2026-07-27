// Font-only adaptation derived from PoDoFo.
// SPDX-License-Identifier: LGPL-2.0-or-later OR MPL-2.0
// Font-only declarations extracted from PoDoFo's PdfDeclarations/PdfFontMetrics types.
// No PDF parser, object model, writer or exception dependency.
#pragma once
#include <podofo/auxiliary/baseincludes.h>
#include <cstdint>
#include <map>
#include <string>
#include <vector>
namespace PoDoFo {
enum class FontFileType : uint8_t { Unknown=0, Type1, TrueType, OpenTypeCFF, Type1CFF, CIDKeyedCFF, CFF2 };
struct FontGID final { unsigned Id=0; unsigned MetricsId=0; };
struct FontCharGIDInfo final { char32_t CodePoint=0; uint16_t Cid=0; FontGID Gid; };
struct CIDSystemInfo final { std::string Registry="Adobe"; std::string Ordering="Identity"; int Supplement=0; };
struct UnicodeRange final { char32_t First=0; char32_t Last=0; };
struct FontSubsetOptions final {
    bool RetainGids=false;
    // Matches HarfBuzz HB_SUBSET_FLAGS_RETAIN_NUM_GLYPHS when used together
    // with RetainGids: preserve maxp.numGlyphs instead of trimming trailing
    // empty glyph slots after the highest retained GID.
    bool RetainNumGlyphs=false;
    bool IncludeCmap=false;
    bool IncludeHintingTables=true;
};
struct FontSubsetResult final {
    charbuff FontProgram;
    FontFileType FileType=FontFileType::Unknown;
    bool RetainsOriginalGids=false;
    bool RetainsOriginalGlyphCount=false;
    std::map<unsigned,unsigned> OriginalToSubsetGid;
    std::map<uint16_t,unsigned> CidToSubsetGid;
};
struct CIDGlyphMapping final { uint16_t Cid=0; unsigned SourceGid=0; std::u32string Unicode; };
}

// Adapted from PoDoFo private/FreetypePrivate.h. PDF error dependencies removed.
// SPDX-License-Identifier: LGPL-2.0-or-later OR MPL-2.0
#pragma once
#include <ft2build.h>
#include FT_FREETYPE_H
#include <podofo/auxiliary/basetypes.h>
#include <podofo/main/FontDeclarations.h>
#include <map>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
namespace FT {
struct FontFaceInfo final {
    unsigned Index=0;
    unsigned FaceCount=0;
    std::string FamilyName;
    std::string StyleName;
    std::string PostScriptName;
    PoDoFo::FontFileType FileType=PoDoFo::FontFileType::Unknown;
    bool IsScalable=false;
    bool HasUnicodeCharmap=false;
    bool IsCidKeyed=false;
    unsigned GlyphCount=0;
    unsigned UnitsPerEm=0;
};
void FreeFace(FT_Face face) noexcept;
using FT_FacePtr=std::unique_ptr<FT_FaceRec_,decltype(&FreeFace)>;
FT_Library GetLibrary() noexcept;
FT_FacePtr CreateFaceFromFile(std::string_view filepath,unsigned faceIndex,PoDoFo::charbuff& retainedBuffer) noexcept;
FT_FacePtr CreateFaceFromBuffer(const PoDoFo::bufferview& view,unsigned faceIndex,PoDoFo::charbuff& retainedBuffer) noexcept;
FT_FacePtr CreateFaceFromBuffer(const PoDoFo::bufferview& view) noexcept;
FT_FacePtr ExtractCFFFont(FT_Face face,PoDoFo::charbuff& retainedBuffer) noexcept;
bool GetDataFromFace(FT_Face face,PoDoFo::charbuff& output) noexcept;
bool TryGetFontFileFormat(FT_Face face,PoDoFo::FontFileType& format) noexcept;
bool IsSupported(FT_Face face) noexcept;
std::unordered_map<std::string,unsigned> GetPostMap(FT_Face face) noexcept;
bool IsTTCFont(FT_Face face) noexcept;
bool IsTTCFont(const PoDoFo::bufferview& view) noexcept;
bool TryExtractDataFromTTC(FT_Face face,PoDoFo::charbuff& output) noexcept;
std::optional<unsigned> GetFontFaceCount(const PoDoFo::bufferview& view) noexcept;
std::optional<FontFaceInfo> GetFontFaceInfo(const PoDoFo::bufferview& view,unsigned faceIndex) noexcept;
std::vector<FontFaceInfo> GetAllFontFaces(const PoDoFo::bufferview& view) noexcept;
std::map<std::string,unsigned> GetFontNamesWithIndex(const PoDoFo::bufferview& view) noexcept;
std::optional<unsigned> GetFontIndexByName(const PoDoFo::bufferview& view,std::string_view targetName) noexcept;
std::optional<std::string> GetPostScriptName(FT_Face face) noexcept;
std::optional<FT_UShort> GetFsTypeFlags(FT_Face face) noexcept;
std::optional<unsigned> GetGlyphIndex(FT_Face face,char32_t codePoint) noexcept;
bool TryLoadSfntTable(FT_Face face,FT_ULong tag,PoDoFo::charbuff& output) noexcept;
std::vector<PoDoFo::UnicodeRange> GetUnicodeRanges(FT_Face face) noexcept;
}
// Other legacy TrueType tables retained from PoDoFo.
#define TTAG_acnt FT_MAKE_TAG('a','c','n','t')
#define TTAG_ankr FT_MAKE_TAG('a','n','k','r')
#define TTAG_kerx FT_MAKE_TAG('k','e','r','x')
#define TTAG_fdsc FT_MAKE_TAG('f','d','s','c')
#define TTAG_fmtx FT_MAKE_TAG('f','m','t','x')
#define TTAG_fond FT_MAKE_TAG('f','o','n','d')
#define TTAG_gcid FT_MAKE_TAG('g','c','i','d')
#define TTAG_ltag FT_MAKE_TAG('l','t','a','g')
#define TTAG_meta FT_MAKE_TAG('m','e','t','a')
#define TTAG_xref FT_MAKE_TAG('x','r','e','f')
#define TTAG_Zapf FT_MAKE_TAG('Z','a','p','f')

// Font-only adaptation derived from PoDoFo.
// SPDX-License-Identifier: LGPL-2.0-or-later OR MPL-2.0
// PDFium-facing but PDF-neutral intermediate layer.
// It contains no PDF parser, writer, dictionary or object model.
#pragma once
#include <podofo/main/FontDeclarations.h>
#include <podofo/main/FontMetrics.h>
#include <string>
#include <vector>
namespace PoDoFo {
class OutputStream;
struct FontSubsetPackage final {
    FontSubsetResult Subset;
    std::vector<FontCharGIDInfo> Infos;
    bool CanUseIdentityCidToGidMap=false;
};
class FontSubsetAdapter final {
public:
    static bool BuildFromUnicode(const FontMetrics& metrics,const cspan<char32_t>& codePoints,const FontSubsetOptions& options,FontSubsetPackage& output,std::string* error=nullptr) noexcept;
    static bool BuildFromCids(const FontMetrics& metrics,const cspan<CIDGlyphMapping>& mappings,const FontSubsetOptions& options,FontSubsetPackage& output,const CIDSystemInfo& cidInfo={},std::string* error=nullptr) noexcept;
    static bool BuildFromIdentityCids(const FontMetrics& metrics,const cspan<uint16_t>& cids,const FontSubsetOptions& options,FontSubsetPackage& output,const CIDSystemInfo& cidInfo={},std::string* error=nullptr) noexcept;
    // PDF-neutral convenience entry: source GIDs are subset directly. Internally
    // this reuses the Identity-CID path so retained GID mapping semantics remain identical.
    static bool BuildFromGids(const FontMetrics& metrics,const cspan<uint16_t>& gids,const FontSubsetOptions& options,FontSubsetPackage& output,std::string* error=nullptr) noexcept;
    static bool WriteFontProgram(const FontSubsetPackage& package,OutputStream& output) noexcept;
};
bool DecodeUtf8(std::string_view utf8,std::vector<char32_t>& output) noexcept;
}

// Thin consumer-facing wrapper over the reused PoDoFo font tool classes.
// It deliberately contains no PDF objects and never propagates exceptions.
// SPDX-License-Identifier: LGPL-2.0-or-later OR MPL-2.0
#pragma once
#include <podofo/auxiliary/basedefs.h>
#include "FontMetricsFreetype.h"
#include <podofo/intermediate/FontSubsetAdapter.h>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
namespace PoDoFo {
class InputStreamDevice;
class OutputStream;
class PODOFO_API FontToolsFacade final {
public:
    bool LoadFontFile(std::string_view filepath,unsigned faceIndex=0) noexcept;
    bool LoadFontBuffer(const bufferview& data,unsigned faceIndex=0) noexcept;
    bool LoadFontDevice(InputStreamDevice& input,unsigned faceIndex=0) noexcept;
    void Reset() noexcept { m_metrics.reset(); }
    bool IsLoaded() const noexcept { return m_metrics!=nullptr; }
    const FontMetricsFreetype* GetMetrics() const noexcept { return m_metrics.get(); }
    std::string_view GetFontName() const noexcept { return m_metrics?m_metrics->GetFontName():std::string_view{}; }
    std::string_view GetFontFamilyName() const noexcept { return m_metrics?m_metrics->GetFontFamilyName():std::string_view{}; }
    FontFileType GetFontFileType() const noexcept { return m_metrics?m_metrics->GetFontFileType():FontFileType::Unknown; }
    bool SubsetUtf8(std::string_view text,const FontSubsetOptions& options,FontSubsetPackage& output,std::string* error=nullptr) const noexcept;
    bool SubsetUnicode(const cspan<char32_t>& codePoints,const FontSubsetOptions& options,FontSubsetPackage& output,std::string* error=nullptr) const noexcept;
    bool SubsetGids(const cspan<uint16_t>& gids,const FontSubsetOptions& options,FontSubsetPackage& output,std::string* error=nullptr) const noexcept;
    bool SubsetCids(const cspan<CIDGlyphMapping>& mappings,const FontSubsetOptions& options,FontSubsetPackage& output,const CIDSystemInfo& cidInfo={},std::string* error=nullptr) const noexcept;
    static bool WriteFontProgram(const FontSubsetPackage& package,OutputStream& output) noexcept;
    static std::optional<unsigned> GetFontFaceCount(const bufferview& data) noexcept;
    static std::vector<FT::FontFaceInfo> GetAllFontFaces(const bufferview& data) noexcept;
    static std::optional<unsigned> GetFontIndexByName(const bufferview& data,std::string_view targetName) noexcept;
private:
    bool ensureLoaded(std::string* error) const noexcept;
    std::unique_ptr<FontMetricsFreetype> m_metrics;
};
}

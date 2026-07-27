// Adapted from PoDoFo PdfFontMetricsFreetype: font-only subset.
// SPDX-License-Identifier: LGPL-2.0-or-later OR MPL-2.0
#pragma once
#include "FontMetrics.h"
#include <podofo/private/FreetypePrivate.h>
namespace PoDoFo {
class FontMetricsFreetype final : public FontMetrics {
public:
    static std::unique_ptr<FontMetricsFreetype> CreateFromFile(std::string_view filepath,unsigned faceIndex=0) noexcept;
    static std::unique_ptr<FontMetricsFreetype> CreateFromBuffer(const bufferview& view,unsigned faceIndex=0) noexcept;
    FontFileType GetFontFileType() const noexcept override { return m_fileType; }
    const bufferview& GetOrLoadFontFileData() const noexcept override { return m_view; }
    FT_Face GetFaceHandle() const noexcept override { return m_face.get(); }
    std::string_view GetFontName() const noexcept override { return m_fontName; }
    std::string_view GetFontFamilyName() const noexcept override { return m_familyName; }
    unsigned GetGlyphCountFontProgram() const noexcept override;
    unsigned GetUnitsPerEm() const noexcept override;
    bool TryGetGID(char32_t codePoint,unsigned& gid) const noexcept override;
    bool TryGetGlyphWidthFontProgram(unsigned gid,double& width) const noexcept override;
private:
    bool init() noexcept;
    charbuff m_data;
    bufferview m_view;
    FT::FT_FacePtr m_face{nullptr,FT::FreeFace};
    FontFileType m_fileType=FontFileType::Unknown;
    std::string m_fontName;
    std::string m_familyName;
};
}

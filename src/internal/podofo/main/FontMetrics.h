// Font-only interface shaped after PoDoFo PdfFontMetrics.
// SPDX-License-Identifier: LGPL-2.0-or-later OR MPL-2.0
#pragma once
#include "FontDeclarations.h"
#include <string_view>
struct FT_FaceRec_;
typedef struct FT_FaceRec_* FT_Face;
namespace PoDoFo {
class FontMetrics {
public:
    virtual ~FontMetrics()=default;
    virtual FontFileType GetFontFileType() const noexcept=0;
    virtual const bufferview& GetOrLoadFontFileData() const noexcept=0;
    virtual FT_Face GetFaceHandle() const noexcept=0;
    virtual std::string_view GetFontName() const noexcept=0;
    virtual std::string_view GetFontFamilyName() const noexcept=0;
    virtual unsigned GetGlyphCountFontProgram() const noexcept=0;
    virtual unsigned GetUnitsPerEm() const noexcept=0;
    virtual bool TryGetGID(char32_t codePoint,unsigned& gid) const noexcept=0;
    virtual bool TryGetGlyphWidthFontProgram(unsigned gid,double& width) const noexcept=0;
};
}

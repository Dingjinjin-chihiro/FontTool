// Font-only adaptation derived from PoDoFo.
// SPDX-License-Identifier: LGPL-2.0-or-later OR MPL-2.0
#include "FontMetricsFreetype.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_FONT_FORMATS_H
namespace PoDoFo {
std::unique_ptr<FontMetricsFreetype> FontMetricsFreetype::CreateFromFile(std::string_view filepath,unsigned faceIndex) noexcept {
    auto ret=std::unique_ptr<FontMetricsFreetype>(new FontMetricsFreetype());
    ret->m_face=FT::CreateFaceFromFile(filepath,faceIndex,ret->m_data);
    if(!ret->m_face)return nullptr;
    ret->m_view=bufferview(ret->m_data.data(),ret->m_data.size());
    return ret->init()?std::move(ret):nullptr;
}
std::unique_ptr<FontMetricsFreetype> FontMetricsFreetype::CreateFromBuffer(const bufferview& view,unsigned faceIndex) noexcept {
    auto ret=std::unique_ptr<FontMetricsFreetype>(new FontMetricsFreetype());
    ret->m_face=FT::CreateFaceFromBuffer(view,faceIndex,ret->m_data);
    if(!ret->m_face)return nullptr;
    ret->m_view=bufferview(ret->m_data.data(),ret->m_data.size());
    return ret->init()?std::move(ret):nullptr;
}
bool FontMetricsFreetype::init() noexcept {
    if(!m_face||!FT::TryGetFontFileFormat(m_face.get(),m_fileType))return false;
    (void)FT_Select_Charmap(m_face.get(),FT_ENCODING_UNICODE);
    const char* ps=FT_Get_Postscript_Name(m_face.get());
    m_fontName=ps?ps:"";
    m_familyName=m_face->family_name?m_face->family_name:"";
    if(m_fontName.empty())m_fontName=m_familyName;
    // A legal subset font may intentionally omit the optional `name` table.
    // Keep inspection and re-subsetting usable by assigning a deterministic
    // fallback rather than rejecting an otherwise valid font program.
    if(m_fontName.empty())m_fontName="UnnamedFont";
    if(m_familyName.empty())m_familyName=m_fontName;
    return true;
}
unsigned FontMetricsFreetype::GetGlyphCountFontProgram() const noexcept { return m_face?static_cast<unsigned>(m_face->num_glyphs):0; }
unsigned FontMetricsFreetype::GetUnitsPerEm() const noexcept { return m_face?static_cast<unsigned>(m_face->units_per_EM):0; }
bool FontMetricsFreetype::TryGetGID(char32_t codePoint,unsigned& gid) const noexcept { gid=0;if(!m_face||codePoint>0x10FFFF)return false;gid=FT_Get_Char_Index(m_face.get(),static_cast<FT_ULong>(codePoint));return gid!=0; }
bool FontMetricsFreetype::TryGetGlyphWidthFontProgram(unsigned gid,double& width) const noexcept { width=0;if(!m_face||gid>=static_cast<unsigned>(m_face->num_glyphs)||m_face->units_per_EM==0)return false;if(FT_Load_Glyph(m_face.get(),gid,FT_LOAD_NO_SCALE|FT_LOAD_NO_BITMAP)!=0)return false;width=static_cast<double>(m_face->glyph->metrics.horiAdvance)/static_cast<double>(m_face->units_per_EM);return true; }
}

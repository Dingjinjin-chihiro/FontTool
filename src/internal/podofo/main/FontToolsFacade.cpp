// Font-only adaptation derived from PoDoFo.
// SPDX-License-Identifier: LGPL-2.0-or-later OR MPL-2.0
#include "FontToolsFacade.h"
#include <podofo/auxiliary/FontUtils.h>
#include <podofo/auxiliary/InputDevice.h>
namespace PoDoFo {
namespace { bool fail(std::string* error,const char* message) noexcept{if(error)*error=message?message:"font tools failure";return false;} }
bool FontToolsFacade::LoadFontFile(std::string_view filepath,unsigned faceIndex) noexcept{m_metrics=FontMetricsFreetype::CreateFromFile(filepath,faceIndex);return m_metrics!=nullptr;}
bool FontToolsFacade::LoadFontBuffer(const bufferview& data,unsigned faceIndex) noexcept{m_metrics=FontMetricsFreetype::CreateFromBuffer(data,faceIndex);return m_metrics!=nullptr;}
bool FontToolsFacade::LoadFontDevice(InputStreamDevice& input,unsigned faceIndex) noexcept{charbuff data;if(!utls::ReadTo(data,input))return false;return LoadFontBuffer(bufferview(data.data(),data.size()),faceIndex);}
bool FontToolsFacade::ensureLoaded(std::string* error) const noexcept{return m_metrics?true:fail(error,"no font face is loaded");}
bool FontToolsFacade::SubsetUtf8(std::string_view text,const FontSubsetOptions& options,FontSubsetPackage& output,std::string* error) const noexcept{if(!ensureLoaded(error))return false;std::vector<char32_t> cps;if(!DecodeUtf8(text,cps))return fail(error,"input text is not valid UTF-8");return FontSubsetAdapter::BuildFromUnicode(*m_metrics,cspan<char32_t>(cps.data(),cps.size()),options,output,error);}
bool FontToolsFacade::SubsetUnicode(const cspan<char32_t>& cps,const FontSubsetOptions& options,FontSubsetPackage& output,std::string* error) const noexcept{if(!ensureLoaded(error))return false;return FontSubsetAdapter::BuildFromUnicode(*m_metrics,cps,options,output,error);}
bool FontToolsFacade::SubsetGids(const cspan<uint16_t>& gids,const FontSubsetOptions& options,FontSubsetPackage& output,std::string* error) const noexcept{if(!ensureLoaded(error))return false;return FontSubsetAdapter::BuildFromGids(*m_metrics,gids,options,output,error);}
bool FontToolsFacade::SubsetCids(const cspan<CIDGlyphMapping>& mappings,const FontSubsetOptions& options,FontSubsetPackage& output,const CIDSystemInfo& cidInfo,std::string* error) const noexcept{if(!ensureLoaded(error))return false;return FontSubsetAdapter::BuildFromCids(*m_metrics,mappings,options,output,cidInfo,error);}
bool FontToolsFacade::WriteFontProgram(const FontSubsetPackage& package,OutputStream& output) noexcept{return FontSubsetAdapter::WriteFontProgram(package,output);}
std::optional<unsigned> FontToolsFacade::GetFontFaceCount(const bufferview& data) noexcept{return FT::GetFontFaceCount(data);}
std::vector<FT::FontFaceInfo> FontToolsFacade::GetAllFontFaces(const bufferview& data) noexcept{return FT::GetAllFontFaces(data);}
std::optional<unsigned> FontToolsFacade::GetFontIndexByName(const bufferview& data,std::string_view targetName) noexcept{return FT::GetFontIndexByName(data,targetName);}
}

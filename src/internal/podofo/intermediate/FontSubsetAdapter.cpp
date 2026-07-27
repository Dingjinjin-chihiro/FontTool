// Font-only adaptation derived from PoDoFo.
// SPDX-License-Identifier: LGPL-2.0-or-later OR MPL-2.0
#include "FontSubsetAdapter.h"
#include <podofo/private/FontTrueTypeSubset.h>
#include <podofo/private/FontUtilsAFDKO.h>
#include <podofo/auxiliary/OutputStream.h>
#include <podofo/auxiliary/FontUtils.h>
#include <map>
#include <set>
namespace PoDoFo {
namespace {
bool fail(std::string* error,const char* text) noexcept {if(error)*error=text?text:"font subset failed";return false;}
bool dispatch(const FontMetrics& metrics,const std::vector<FontCharGIDInfo>& infos,const FontSubsetOptions& options,FontSubsetPackage& output,const CIDSystemInfo& cidInfo,std::string* error) noexcept {
    output={}; output.Infos=infos;
    if(metrics.GetFontFileType()==FontFileType::TrueType){if(!FontTrueTypeSubset::BuildFont(metrics,cspan<FontCharGIDInfo>(infos.data(),infos.size()),output.Subset.FontProgram,options,&output.Subset,error))return false;}
    else if(metrics.GetFontFileType()==FontFileType::OpenTypeCFF||metrics.GetFontFileType()==FontFileType::Type1CFF||metrics.GetFontFileType()==FontFileType::CIDKeyedCFF||metrics.GetFontFileType()==FontFileType::Type1){if(!afdko::SubsetFontCFF(metrics,cspan<FontCharGIDInfo>(infos.data(),infos.size()),cidInfo,output.Subset.FontProgram,error))return false;output.Subset.FileType=FontFileType::CIDKeyedCFF;output.Subset.RetainsOriginalGids=false;for(const auto& info:infos)output.Subset.CidToSubsetGid[info.Cid]=info.Cid;}
    else return fail(error,"unsupported font outline format");
    output.CanUseIdentityCidToGidMap=output.Subset.FileType==FontFileType::TrueType;for(const auto& pair:output.Subset.CidToSubsetGid)if(pair.first!=pair.second){output.CanUseIdentityCidToGidMap=false;break;}return true;
}
}
bool FontSubsetAdapter::BuildFromUnicode(const FontMetrics& metrics,const cspan<char32_t>& codePoints,const FontSubsetOptions& options,FontSubsetPackage& output,std::string* error) noexcept {if(codePoints.empty())return fail(error,"Unicode subset input is empty");std::vector<FontCharGIDInfo> infos;infos.reserve(codePoints.size());std::map<char32_t,uint16_t> codepointCid;uint16_t nextCid=1;for(char32_t cp:codePoints){if(codepointCid.find(cp)!=codepointCid.end())continue;unsigned gid=0;if(!metrics.TryGetGID(cp,gid))return fail(error,"font does not contain one of the requested Unicode code points");uint16_t cid=0;if(options.RetainGids){if(gid>0xFFFFu)return fail(error,"GID does not fit into CID");cid=static_cast<uint16_t>(gid);}else{if(nextCid==0)return fail(error,"too many Unicode glyphs");cid=nextCid++;}codepointCid.emplace(cp,cid);infos.push_back({cp,cid,{gid,gid}});}return dispatch(metrics,infos,options,output,CIDSystemInfo{},error);}
bool FontSubsetAdapter::BuildFromCids(const FontMetrics& metrics,const cspan<CIDGlyphMapping>& mappings,const FontSubsetOptions& options,FontSubsetPackage& output,const CIDSystemInfo& cidInfo,std::string* error) noexcept {if(mappings.empty())return fail(error,"CID subset input is empty");std::vector<FontCharGIDInfo> infos;infos.reserve(mappings.size());std::map<uint16_t,unsigned> seen;for(const auto& mapping:mappings){if(mapping.Cid==0)return fail(error,"CID 0 is reserved for .notdef");auto old=seen.find(mapping.Cid);if(old!=seen.end()&&old->second!=mapping.SourceGid)return fail(error,"one CID maps to two source GIDs");if(old!=seen.end())continue;seen.emplace(mapping.Cid,mapping.SourceGid);infos.push_back({mapping.Unicode.empty()?0:mapping.Unicode.front(),mapping.Cid,{mapping.SourceGid,mapping.SourceGid}});}return dispatch(metrics,infos,options,output,cidInfo,error);}
bool FontSubsetAdapter::BuildFromIdentityCids(const FontMetrics& metrics,const cspan<uint16_t>& cids,const FontSubsetOptions& options,FontSubsetPackage& output,const CIDSystemInfo& cidInfo,std::string* error) noexcept {if(cids.empty())return fail(error,"Identity CID input is empty");std::vector<CIDGlyphMapping> mappings;mappings.reserve(cids.size());for(uint16_t cid:cids){if(cid==0)continue;mappings.push_back({cid,cid,{}});}if(mappings.empty())return fail(error,"Identity CID input contains only CID 0");return BuildFromCids(metrics,cspan<CIDGlyphMapping>(mappings.data(),mappings.size()),options,output,cidInfo,error);}
bool FontSubsetAdapter::BuildFromGids(const FontMetrics& metrics,const cspan<uint16_t>& gids,const FontSubsetOptions& options,FontSubsetPackage& output,std::string* error) noexcept {return BuildFromIdentityCids(metrics,gids,options,output,CIDSystemInfo{},error);}
bool FontSubsetAdapter::WriteFontProgram(const FontSubsetPackage& package,OutputStream& output) noexcept {return !package.Subset.FontProgram.empty()&&output.Write(package.Subset.FontProgram)&&output.Flush();}
bool DecodeUtf8(std::string_view text,std::vector<char32_t>& output) noexcept {
    std::u32string converted;
    if(!utls::Utf8ToUtf32(text,converted)){output.clear();return false;}
    output.assign(converted.begin(),converted.end());
    return true;
}
}

// Adapted from PoDoFo private/FreetypePrivate.cpp. No C++ exceptions.
// SPDX-License-Identifier: LGPL-2.0-or-later OR MPL-2.0
#include "FreetypePrivate.h"
#include <podofo/auxiliary/BinaryIO.h>
#include <podofo/auxiliary/FontUtils.h>
#include <ft2build.h>
#include FT_CID_H
#include FT_FONT_FORMATS_H
#include FT_TRUETYPE_TABLES_H
#include FT_TRUETYPE_TAGS_H
#include <algorithm>
#include <cctype>
#include <cstring>
#include <limits>
namespace {
constexpr unsigned kDirectoryFixedSize=12;
struct TableInfo { FT_ULong Tag=0; FT_ULong Size=0; };
struct LibraryHolder { FT_Library Library=nullptr; LibraryHolder() noexcept { (void)FT_Init_FreeType(&Library); } ~LibraryHolder(){ if(Library)(void)FT_Done_FreeType(Library); } };
thread_local LibraryHolder g_library;
uint32_t read32(const char* p) noexcept { uint32_t v=0;PoDoFo::utls::ReadUInt32BE(p,v);return v; }
void write16(char* p,uint16_t v) noexcept { PoDoFo::utls::WriteUInt16BE(p,v); }
void write32(char* p,uint32_t v) noexcept { PoDoFo::utls::WriteUInt32BE(p,v); }
size_t align4(size_t n) noexcept { return (n+3u)&~size_t(3u); }
uint32_t checksum(const char* data,size_t size) noexcept { uint32_t sum=0; const size_t padded=align4(size); for(size_t i=0;i<padded;i+=4){ uint32_t word=0; for(unsigned j=0;j<4;++j){word<<=8;if(i+j<size)word|=static_cast<unsigned char>(data[i+j]);} sum+=word;} return sum; }
FT::FT_FacePtr createFace(const PoDoFo::bufferview& view,FT_Long faceIndex) noexcept { FT_Face face=nullptr; if(view.empty()||view.size()>static_cast<size_t>(std::numeric_limits<FT_Long>::max())||!FT::GetLibrary()) return FT::FT_FacePtr(nullptr,FT::FreeFace); if(FT_New_Memory_Face(FT::GetLibrary(),reinterpret_cast<const FT_Byte*>(view.data()),static_cast<FT_Long>(view.size()),faceIndex,&face)!=0)return FT::FT_FacePtr(nullptr,FT::FreeFace); return FT::FT_FacePtr(face,FT::FreeFace); }
bool determineFaceTables(FT_Face face,std::vector<TableInfo>& tables) noexcept { FT_ULong count=0;if(!face||FT_Sfnt_Table_Info(face,0,nullptr,&count)!=0||count==0||count>0xFFFF)return false;tables.resize(static_cast<size_t>(count));for(FT_ULong i=0;i<count;++i)if(FT_Sfnt_Table_Info(face,static_cast<FT_UInt>(i),&tables[i].Tag,&tables[i].Size)!=0)return false;return true; }
PoDoFo::FontFileType determineFormatCFF(FT_Face face) noexcept { FT_ULong size=0;if(FT_Sfnt_Table_Info(face,0,nullptr,&size)==0)return PoDoFo::FontFileType::OpenTypeCFF;FT_Bool isCid=0;if(FT_Get_CID_Is_Internally_CID_Keyed(face,&isCid)==0&&isCid)return PoDoFo::FontFileType::CIDKeyedCFF;return PoDoFo::FontFileType::Type1CFF; }
bool hasUnicodeCharmap(FT_Face face) noexcept { if(!face)return false;for(int i=0;i<face->num_charmaps;++i)if(face->charmaps[i]&&face->charmaps[i]->encoding==FT_ENCODING_UNICODE)return true;return false; }
bool isCidKeyed(FT_Face face) noexcept { FT_Bool value=0;return face&&FT_Get_CID_Is_Internally_CID_Keyed(face,&value)==0&&value!=0; }
}
namespace FT {
void FreeFace(FT_Face face) noexcept { if(face)(void)FT_Done_Face(face); }
FT_Library GetLibrary() noexcept { return g_library.Library; }
bool IsTTCFont(const PoDoFo::bufferview& view) noexcept { return view.size()>=4&&read32(view.data())==FT_MAKE_TAG('t','t','c','f'); }
bool IsTTCFont(FT_Face face) noexcept { if(!face)return false;FT_ULong size=4;uint32_t tag=0;return FT_Load_Sfnt_Table(face,0,0,reinterpret_cast<FT_Byte*>(&tag),&size)==0&&size==4&&read32(reinterpret_cast<const char*>(&tag))==FT_MAKE_TAG('t','t','c','f'); }
FT_FacePtr CreateFaceFromBuffer(const PoDoFo::bufferview& view,unsigned faceIndex,PoDoFo::charbuff& retainedBuffer) noexcept { retainedBuffer.clear(); if(IsTTCFont(view)){auto collectionFace=createFace(view,static_cast<FT_Long>(faceIndex));if(!collectionFace)return FT_FacePtr(nullptr,FreeFace);if(TryExtractDataFromTTC(collectionFace.get(),retainedBuffer))return createFace(PoDoFo::bufferview(retainedBuffer.data(),retainedBuffer.size()),0);retainedBuffer.assign(view.data(),view.size());return createFace(PoDoFo::bufferview(retainedBuffer.data(),retainedBuffer.size()),static_cast<FT_Long>(faceIndex));} retainedBuffer.assign(view.data(),view.size());return createFace(PoDoFo::bufferview(retainedBuffer.data(),retainedBuffer.size()),0); }
FT_FacePtr CreateFaceFromBuffer(const PoDoFo::bufferview& view) noexcept { return createFace(view,0); }
FT_FacePtr CreateFaceFromFile(std::string_view filepath,unsigned faceIndex,PoDoFo::charbuff& retainedBuffer) noexcept { PoDoFo::charbuff source;if(!PoDoFo::utls::ReadTo(source,filepath))return FT_FacePtr(nullptr,FreeFace);return CreateFaceFromBuffer(PoDoFo::bufferview(source.data(),source.size()),faceIndex,retainedBuffer); }
FT_FacePtr ExtractCFFFont(FT_Face face,PoDoFo::charbuff& retainedBuffer) noexcept { retainedBuffer.clear();if(!face)return FT_FacePtr(nullptr,FreeFace);FT_ULong size=0;if(FT_Load_Sfnt_Table(face,TTAG_CFF,0,nullptr,&size)!=0)return FT_FacePtr(nullptr,FreeFace);retainedBuffer.resize(size);if(size&&FT_Load_Sfnt_Table(face,TTAG_CFF,0,reinterpret_cast<FT_Byte*>(retainedBuffer.data()),&size)!=0)return FT_FacePtr(nullptr,FreeFace);retainedBuffer.resize(size);return createFace(PoDoFo::bufferview(retainedBuffer.data(),retainedBuffer.size()),0); }
bool GetDataFromFace(FT_Face face,PoDoFo::charbuff& output) noexcept { output.clear();if(!face)return false;if(IsTTCFont(face))return TryExtractDataFromTTC(face,output);FT_ULong size=0;if(FT_Load_Sfnt_Table(face,0,0,nullptr,&size)!=0)return false;output.resize(size);if(size&&FT_Load_Sfnt_Table(face,0,0,reinterpret_cast<FT_Byte*>(output.data()),&size)!=0){output.clear();return false;}output.resize(size);return true; }
bool TryGetFontFileFormat(FT_Face face,PoDoFo::FontFileType& format) noexcept { format=PoDoFo::FontFileType::Unknown;if(!face)return false;const char* ptr=FT_Get_Font_Format(face);if(!ptr)return false;const std::string_view value(ptr);if(value=="TrueType")format=PoDoFo::FontFileType::TrueType;else if(value=="Type 1")format=PoDoFo::FontFileType::Type1;else if(value=="CID Type 1")format=PoDoFo::FontFileType::CIDKeyedCFF;else if(value=="CFF")format=determineFormatCFF(face);else return false;return true; }
bool IsSupported(FT_Face face) noexcept { PoDoFo::FontFileType format;return TryGetFontFileFormat(face,format); }
std::unordered_map<std::string,unsigned> GetPostMap(FT_Face face) noexcept { std::unordered_map<std::string,unsigned> ret;if(!face||!FT_HAS_GLYPH_NAMES(face))return ret;char name[256]{};for(FT_Long i=0;i<face->num_glyphs;++i){std::memset(name,0,sizeof(name));if(FT_Get_Glyph_Name(face,static_cast<FT_UInt>(i),name,sizeof(name))==0&&name[0])ret.emplace(std::string(name),static_cast<unsigned>(i));}return ret; }
bool TryExtractDataFromTTC(FT_Face face,PoDoFo::charbuff& output) noexcept {
    output.clear(); if(!face||face->face_index<0)return false;
    std::vector<TableInfo> tables; if(!determineFaceTables(face,tables))return false;
    const size_t directorySize=kDirectoryFixedSize+tables.size()*16u; size_t total=directorySize;for(const auto& t:tables){if(t.Size>std::numeric_limits<size_t>::max()-total)return false;total+=align4(static_cast<size_t>(t.Size));}
    output.assign(total,'\0'); uint32_t sfntVersion=0x00010000u; if(const char* format=FT_Get_Font_Format(face);format&&std::string_view(format)=="CFF")sfntVersion=FT_MAKE_TAG('O','T','T','O'); write32(output.data(),sfntVersion);const auto count=static_cast<uint16_t>(tables.size());write16(output.data()+4,count);uint16_t maxPow=1,entry=0;while(static_cast<uint16_t>(maxPow*2u)<=count){maxPow=static_cast<uint16_t>(maxPow*2u);++entry;}write16(output.data()+6,static_cast<uint16_t>(maxPow*16u));write16(output.data()+8,entry);write16(output.data()+10,static_cast<uint16_t>(count*16u-maxPow*16u));
    size_t offset=directorySize;size_t headOffset=0;
    for(size_t i=0;i<tables.size();++i){auto& table=tables[i];FT_ULong size=table.Size;if(offset+size>output.size())return false;if(size&&FT_Load_Sfnt_Table(face,table.Tag,0,reinterpret_cast<FT_Byte*>(output.data()+offset),&size)!=0)return false;if(table.Tag==TTAG_head&&size>=12){write32(output.data()+offset+8,0);headOffset=offset;}char* rec=output.data()+kDirectoryFixedSize+i*16u;write32(rec,static_cast<uint32_t>(table.Tag));write32(rec+4,checksum(output.data()+offset,size));write32(rec+8,static_cast<uint32_t>(offset));write32(rec+12,static_cast<uint32_t>(size));offset+=align4(size);}
    if(headOffset==0)return false;
    write32(output.data()+headOffset+8,0xB1B0AFBAu-checksum(output.data(),output.size()));
    return true;
}
std::optional<unsigned> GetFontFaceCount(const PoDoFo::bufferview& view) noexcept { if(view.empty())return std::nullopt;auto face=createFace(view,-1);if(!face)return std::nullopt;return static_cast<unsigned>(face->num_faces); }
std::optional<FontFaceInfo> GetFontFaceInfo(const PoDoFo::bufferview& view,unsigned faceIndex) noexcept { auto face=createFace(view,static_cast<FT_Long>(faceIndex));if(!face)return std::nullopt;FontFaceInfo info;info.Index=faceIndex;info.FaceCount=static_cast<unsigned>(face->num_faces);info.FamilyName=face->family_name?face->family_name:"";info.StyleName=face->style_name?face->style_name:"";if(const char* value=FT_Get_Postscript_Name(face.get());value)info.PostScriptName=value;(void)TryGetFontFileFormat(face.get(),info.FileType);info.IsScalable=FT_IS_SCALABLE(face.get())!=0;info.HasUnicodeCharmap=hasUnicodeCharmap(face.get());info.IsCidKeyed=isCidKeyed(face.get());info.GlyphCount=face->num_glyphs>0?static_cast<unsigned>(face->num_glyphs):0;info.UnitsPerEm=static_cast<unsigned>(face->units_per_EM);return info; }
std::vector<FontFaceInfo> GetAllFontFaces(const PoDoFo::bufferview& view) noexcept { std::vector<FontFaceInfo> ret;auto count=GetFontFaceCount(view);if(!count)return ret;ret.reserve(*count);for(unsigned i=0;i<*count;++i){auto info=GetFontFaceInfo(view,i);if(info)ret.push_back(std::move(*info));}return ret; }
std::map<std::string,unsigned> GetFontNamesWithIndex(const PoDoFo::bufferview& view) noexcept { std::map<std::string,unsigned> ret;for(const auto& info:GetAllFontFaces(view)){std::string name=!info.PostScriptName.empty()?info.PostScriptName:info.FamilyName;if(name.empty())name="face";if(!ret.emplace(name,info.Index).second)ret.emplace(name+" ("+std::to_string(info.Index)+")",info.Index);}return ret; }
std::optional<unsigned> GetFontIndexByName(const PoDoFo::bufferview& view,std::string_view targetName) noexcept { const auto target=PoDoFo::utls::NormalizeFontName(targetName);if(target.empty())return std::nullopt;for(const auto& info:GetAllFontFaces(view)){const auto family=PoDoFo::utls::NormalizeFontName(info.FamilyName);const auto style=PoDoFo::utls::NormalizeFontName(info.StyleName);const auto ps=PoDoFo::utls::NormalizeFontName(info.PostScriptName);const auto familyStyle=family+style;if(target==family||target==ps||(!familyStyle.empty()&&target==familyStyle))return info.Index;if(!family.empty()&&target.find(family)!=std::string::npos&&(style.empty()||target.find(style)!=std::string::npos))return info.Index;}return std::nullopt; }
std::optional<std::string> GetPostScriptName(FT_Face face) noexcept { if(!face)return std::nullopt;const char* value=FT_Get_Postscript_Name(face);if(!value||!*value)return std::nullopt;return std::string(value); }
std::optional<FT_UShort> GetFsTypeFlags(FT_Face face) noexcept { if(!face)return std::nullopt;return FT_Get_FSType_Flags(face); }
std::optional<unsigned> GetGlyphIndex(FT_Face face,char32_t codePoint) noexcept { if(!face||codePoint>0x10FFFFu||(codePoint>=0xD800u&&codePoint<=0xDFFFu))return std::nullopt;const auto gid=FT_Get_Char_Index(face,static_cast<FT_ULong>(codePoint));if(gid==0)return std::nullopt;return static_cast<unsigned>(gid); }
bool TryLoadSfntTable(FT_Face face,FT_ULong tag,PoDoFo::charbuff& output) noexcept { output.clear();if(!face)return false;FT_ULong size=0;if(FT_Load_Sfnt_Table(face,tag,0,nullptr,&size)!=0)return false;output.resize(size);if(size&&FT_Load_Sfnt_Table(face,tag,0,reinterpret_cast<FT_Byte*>(output.data()),&size)!=0){output.clear();return false;}output.resize(size);return true; }
std::vector<PoDoFo::UnicodeRange> GetUnicodeRanges(FT_Face face) noexcept { std::vector<PoDoFo::UnicodeRange> ret;if(!face||FT_Select_Charmap(face,FT_ENCODING_UNICODE)!=0)return ret;FT_UInt gid=0;FT_ULong current=FT_Get_First_Char(face,&gid);while(gid){PoDoFo::UnicodeRange range{static_cast<char32_t>(current),static_cast<char32_t>(current)};while(true){FT_UInt nextGid=0;const auto next=FT_Get_Next_Char(face,current,&nextGid);if(!nextGid||next!=current+1){current=next;gid=nextGid;break;}range.Last=static_cast<char32_t>(next);current=next;gid=nextGid;}ret.push_back(range);}return ret; }
}

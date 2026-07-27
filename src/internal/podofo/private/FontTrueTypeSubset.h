// Adapted from PoDoFo private/FontTrueTypeSubset.h.
// SPDX-License-Identifier: LGPL-2.0-or-later OR MPL-2.0
#pragma once
#include <podofo/main/FontMetrics.h>
#include <map>
#include <string>
#include <vector>
namespace PoDoFo {
class InputStreamDevice;
class OutputStream;
class FontTrueTypeSubset final {
public:
    static bool BuildFont(const FontMetrics& metrics,const cspan<FontCharGIDInfo>& infos,charbuff& output,const FontSubsetOptions& options={},FontSubsetResult* result=nullptr,std::string* error=nullptr) noexcept;
private:
    struct GlyphCompoundComponentData { unsigned Offset=0; unsigned GlyphIndex=0; };
    struct GlyphData { bool IsCompound=false; unsigned GlyphOffset=0; unsigned GlyphLength=0; unsigned GlyphAdvOffset=0; std::vector<GlyphCompoundComponentData> CompoundComponents; };
    struct LongHorMetrics { uint16_t AdvanceWidth=0; int16_t LeftSideBearing=0; };
    struct TrueTypeTable { uint32_t Tag=0; uint32_t Checksum=0; uint32_t Length=0; uint32_t Offset=0; };
    struct GIDInfo { unsigned Id=0; LongHorMetrics Metrics; };
    FontTrueTypeSubset(InputStreamDevice& device,const FontMetrics& metrics,const FontSubsetOptions& options,std::string* error) noexcept;
    bool buildFont(const cspan<FontCharGIDInfo>& infos,charbuff& output,FontSubsetResult* result) noexcept;
    bool init() noexcept; bool initTables() noexcept; bool getNumberOfGlyphs() noexcept; bool determineLongLocaTable() noexcept;
    bool getTableOffset(unsigned tag,unsigned& offset) const noexcept;
    bool loadGlyphData(unsigned gid) noexcept; bool loadCompound(const GlyphData& data) noexcept;
    bool loadGlyphMetrics(const cspan<FontCharGIDInfo>& infos) noexcept;
    bool getGlyphMetrics(unsigned gid,LongHorMetrics& metrics) noexcept;
    bool writeGlyphTable(OutputStream& output) noexcept; bool writeHmtxTable(OutputStream& output) noexcept; bool writeLocaTable(OutputStream& output) noexcept; bool writeTables(charbuff& output) noexcept;
    bool copyData(OutputStream& output,unsigned offset,unsigned size) noexcept;
    bool fail(const char* message) noexcept;
    InputStreamDevice* m_device=nullptr; const FontMetrics* m_metrics=nullptr; FontSubsetOptions m_options; std::string* m_error=nullptr;
    bool m_isLongLoca=false; uint16_t m_glyphCount=0; uint16_t m_HMetricsCount=0; unsigned m_hmtxTableOffset=0; unsigned m_leftSideBearingsOffset=0;
    std::vector<TrueTypeTable> m_tables; std::map<unsigned,GlyphData> m_glyphDatas; std::vector<GIDInfo> m_subsetGIDs; std::map<unsigned,unsigned> m_gidRemap; charbuff m_tmpBuffer;
};
}

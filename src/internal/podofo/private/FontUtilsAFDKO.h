/**
 * Adapted from PoDoFo private/FontUtilsAFDKO.h
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once
#include <podofo/main/FontMetrics.h>
#include <string>
namespace afdko {
bool ConvertFontType1ToCFF(const PoDoFo::bufferview& src,PoDoFo::charbuff& dst,std::string* error=nullptr) noexcept;
bool ConvertFontToCFF(const PoDoFo::bufferview& src,PoDoFo::charbuff& dst,std::string* error=nullptr) noexcept;
bool SubsetFontCFF(const PoDoFo::FontMetrics& metrics,const PoDoFo::cspan<PoDoFo::FontCharGIDInfo>& subsetInfos,const PoDoFo::CIDSystemInfo& cidInfo,PoDoFo::charbuff& dstCFF,std::string* error=nullptr) noexcept;
}

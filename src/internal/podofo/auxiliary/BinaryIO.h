// Font-only adaptation derived from PoDoFo.
// SPDX-License-Identifier: LGPL-2.0-or-later OR MPL-2.0
// Extracted from PoDoFo PdfDeclarationsPrivate endian helpers.
#pragma once
#include "InputStream.h"
#include "OutputStream.h"
#include <cstdint>
namespace PoDoFo::utls {
bool WriteUInt16BE(OutputStream&,uint16_t) noexcept; bool WriteInt16BE(OutputStream&,int16_t) noexcept; bool WriteUInt32BE(OutputStream&,uint32_t) noexcept;
void WriteUInt16BE(char*,uint16_t) noexcept; void WriteUInt32BE(char*,uint32_t) noexcept;
bool ReadUInt16BE(InputStream&,uint16_t&) noexcept; bool ReadInt16BE(InputStream&,int16_t&) noexcept; bool ReadUInt32BE(InputStream&,uint32_t&) noexcept;
void ReadUInt16BE(const char*,uint16_t&) noexcept; void ReadUInt32BE(const char*,uint32_t&) noexcept;
}

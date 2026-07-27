// Font-only adaptation derived from PoDoFo.
// SPDX-License-Identifier: LGPL-2.0-or-later OR MPL-2.0
#include "BinaryIO.h"
namespace PoDoFo::utls {
void WriteUInt16BE(char* b,uint16_t v) noexcept { b[0]=static_cast<char>(v>>8);b[1]=static_cast<char>(v); }
void WriteUInt32BE(char* b,uint32_t v) noexcept { b[0]=static_cast<char>(v>>24);b[1]=static_cast<char>(v>>16);b[2]=static_cast<char>(v>>8);b[3]=static_cast<char>(v); }
bool WriteUInt16BE(OutputStream& o,uint16_t v) noexcept { char b[2];WriteUInt16BE(b,v);return o.Write(b,2); }
bool WriteInt16BE(OutputStream& o,int16_t v) noexcept { return WriteUInt16BE(o,static_cast<uint16_t>(v)); }
bool WriteUInt32BE(OutputStream& o,uint32_t v) noexcept { char b[4];WriteUInt32BE(b,v);return o.Write(b,4); }
void ReadUInt16BE(const char* b,uint16_t& v) noexcept { v=(static_cast<uint16_t>(static_cast<unsigned char>(b[0]))<<8)|static_cast<unsigned char>(b[1]); }
void ReadUInt32BE(const char* b,uint32_t& v) noexcept { v=(static_cast<uint32_t>(static_cast<unsigned char>(b[0]))<<24)|(static_cast<uint32_t>(static_cast<unsigned char>(b[1]))<<16)|(static_cast<uint32_t>(static_cast<unsigned char>(b[2]))<<8)|static_cast<unsigned char>(b[3]); }
bool ReadUInt16BE(InputStream& i,uint16_t& v) noexcept { char b[2];if(!i.Read(b,2))return false;ReadUInt16BE(b,v);return true; }
bool ReadInt16BE(InputStream& i,int16_t& v) noexcept { uint16_t t=0;if(!ReadUInt16BE(i,t))return false;v=static_cast<int16_t>(t);return true; }
bool ReadUInt32BE(InputStream& i,uint32_t& v) noexcept { char b[4];if(!i.Read(b,4))return false;ReadUInt32BE(b,v);return true; }
}

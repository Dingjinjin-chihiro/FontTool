// Font-only adaptation derived from PoDoFo.
// SPDX-License-Identifier: LGPL-2.0-or-later OR MPL-2.0
#include "InputStream.h"
#include "OutputStream.h"
#include <algorithm>
namespace PoDoFo {
bool InputStream::Read(char* buffer, size_t size) noexcept {
    if ((!buffer && size != 0) || !checkRead()) return false;
    size_t total = 0;
    bool eof = false;
    while (total < size) {
        size_t once = 0;
        if (!readBuffer(buffer + total, size - total, once, eof)) return false;
        total += once;
        if (once == 0 && eof) break;
        if (once == 0 && !eof) return false;
    }
    return total == size;
}
bool InputStream::Read(char* buffer, size_t size, size_t& read, bool& eof) noexcept {
    read = 0; eof = false;
    if ((!buffer && size != 0) || !checkRead()) return false;
    while (read < size && !eof) {
        size_t once = 0;
        if (!readBuffer(buffer + read, size - read, once, eof)) return false;
        read += once;
        if (once == 0 && !eof) return false;
    }
    return true;
}
bool InputStream::Read(char& ch) noexcept { return checkRead() && readChar(ch); }
bool InputStream::CopyTo(OutputStream& stream) noexcept {
    if (!checkRead()) return false;
    char buffer[4096]; bool eof=false;
    while (!eof) { size_t count=0; if (!readBuffer(buffer,sizeof(buffer),count,eof)) return false; if (count && !stream.Write(buffer,count)) return false; if (!count && !eof) return false; }
    return stream.Flush();
}
bool InputStream::CopyTo(OutputStream& stream, size_t size) noexcept {
    if (!checkRead()) return false;
    char buffer[4096]; bool eof=false;
    while (size != 0 && !eof) { size_t count=0; const auto ask=std::min(size,sizeof(buffer)); if (!readBuffer(buffer,ask,count,eof)) return false; if (count && !stream.Write(buffer,count)) return false; if (!count && !eof) return false; size-=count; }
    return size == 0 && stream.Flush();
}
bool InputStream::readChar(char& ch) noexcept { size_t count=0; bool eof=false; ch='\0'; return readBuffer(&ch,1,count,eof) && count==1; }
}

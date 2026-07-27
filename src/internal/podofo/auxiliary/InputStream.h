// Adapted from PoDoFo auxiliary/InputStream.h: exceptions removed.
// SPDX-License-Identifier: LGPL-2.0-or-later OR MPL-2.0
#pragma once
#include "basedefs.h"
#include <cstddef>
namespace PoDoFo {
class OutputStream;
class PODOFO_API InputStream {
public:
    virtual ~InputStream() = default;
    bool Read(char* buffer, size_t size) noexcept;
    bool Read(char* buffer, size_t size, size_t& read, bool& eof) noexcept;
    bool Read(char& ch) noexcept;
    bool CopyTo(OutputStream& stream) noexcept;
    bool CopyTo(OutputStream& stream, size_t size) noexcept;
protected:
    virtual bool readBuffer(char* buffer, size_t size, size_t& read, bool& eof) noexcept = 0;
    virtual bool readChar(char& ch) noexcept;
    virtual bool checkRead() const noexcept { return true; }
};
}

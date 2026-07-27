// Adapted from PoDoFo auxiliary/OutputStream.h: exceptions removed.
// SPDX-License-Identifier: LGPL-2.0-or-later OR MPL-2.0
#pragma once
#include "basedefs.h"
#include <cstddef>
#include <string_view>
namespace PoDoFo {
class PODOFO_API OutputStream {
public:
    virtual ~OutputStream() = default;
    bool Write(char ch) noexcept;
    bool Write(const std::string_view& view) noexcept;
    bool Write(const char* buffer, size_t size) noexcept;
    bool Flush() noexcept;
protected:
    virtual bool writeBuffer(const char* buffer, size_t size) noexcept = 0;
    virtual bool flush() noexcept { return true; }
    virtual bool checkWrite() const noexcept { return true; }
};
}

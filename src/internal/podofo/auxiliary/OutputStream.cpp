// Font-only adaptation derived from PoDoFo.
// SPDX-License-Identifier: LGPL-2.0-or-later OR MPL-2.0
#include "OutputStream.h"
namespace PoDoFo {
bool OutputStream::Write(char ch) noexcept { return checkWrite() && writeBuffer(&ch,1); }
bool OutputStream::Write(const std::string_view& view) noexcept { return view.empty() || (checkWrite() && writeBuffer(view.data(),view.size())); }
bool OutputStream::Write(const char* buffer, size_t size) noexcept { return size==0 || ((!buffer?false:true) && checkWrite() && writeBuffer(buffer,size)); }
bool OutputStream::Flush() noexcept { return flush(); }
}

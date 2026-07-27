// Font-only adaptation derived from PoDoFo.
// SPDX-License-Identifier: LGPL-2.0-or-later OR MPL-2.0
// Small non-PDF utility surface retained for the extracted font module.
// The UTF helpers deliberately reuse the vendored utf8cpp implementation.
#pragma once
#include "basetypes.h"
#include <cstddef>
#include <string>
#include <string_view>
namespace PoDoFo { class InputStreamDevice; }
namespace PoDoFo::utls {
bool ReadTo(charbuff& output, std::string_view filepath, size_t maxReadSize = 0) noexcept;
bool ReadTo(charbuff& output, InputStreamDevice& input, size_t maxReadSize = 0) noexcept;
bool WriteTo(const bufferview& input, std::string_view filepath) noexcept;

// Non-throwing public UTF conversion helpers. Internally they use utf8cpp's
// checked API, catch every exception at the utility boundary, and report
// malformed text to callers with false.
bool IsValidUtf8(std::string_view input) noexcept;
bool Utf8ToUtf16(std::string_view input, std::u16string& output) noexcept;
bool Utf16ToUtf8(std::u16string_view input, std::string& output) noexcept;
bool Utf8ToUtf32(std::string_view input, std::u32string& output) noexcept;
bool Utf32ToUtf8(std::u32string_view input, std::string& output) noexcept;
bool Utf8ToWString(std::string_view input, std::wstring& output) noexcept;
bool WStringToUtf8(std::wstring_view input, std::string& output) noexcept;

// Case-fold and normalize a font name for cache/TTC matching. utf8proc is
// retained from PoDoFo's third-party tree for Unicode-aware matching.
std::string NormalizeFontName(std::string_view input) noexcept;
}

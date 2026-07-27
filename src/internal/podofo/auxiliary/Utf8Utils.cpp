// Font-only adaptation derived from PoDoFo.
// SPDX-License-Identifier: LGPL-2.0-or-later OR MPL-2.0
// utf8cpp checked conversion boundary.
//
// This translation unit deliberately enables C++ exceptions even though the
// rest of the extracted font-tools library is built without them. utf8cpp's
// checked API may throw for malformed input. Every public helper below is
// noexcept and catches all exceptions before returning false to callers.
#include "FontUtils.h"
#include <utf8cpp/utf8.h>
#include <iterator>
#include <string>
#include <type_traits>

namespace PoDoFo::utls {
namespace {

template <typename String>
bool ClearAndFail(String& output) noexcept {
    output.clear();
    return false;
}

} // namespace

bool IsValidUtf8(std::string_view input) noexcept {
    try {
        return utf8::is_valid(input.begin(), input.end());
    } catch (...) {
        return false;
    }
}

bool Utf8ToUtf16(std::string_view input, std::u16string& output) noexcept {
    try {
        std::u16string converted;
        converted.reserve(input.size());
        utf8::utf8to16(input.begin(), input.end(), std::back_inserter(converted));
        output.swap(converted);
        return true;
    } catch (...) {
        return ClearAndFail(output);
    }
}

bool Utf16ToUtf8(std::u16string_view input, std::string& output) noexcept {
    try {
        std::string converted;
        converted.reserve(input.size());
        utf8::utf16to8(input.begin(), input.end(), std::back_inserter(converted));
        output.swap(converted);
        return true;
    } catch (...) {
        return ClearAndFail(output);
    }
}

bool Utf8ToUtf32(std::string_view input, std::u32string& output) noexcept {
    try {
        std::u32string converted;
        converted.reserve(input.size());
        utf8::utf8to32(input.begin(), input.end(), std::back_inserter(converted));
        output.swap(converted);
        return true;
    } catch (...) {
        return ClearAndFail(output);
    }
}

bool Utf32ToUtf8(std::u32string_view input, std::string& output) noexcept {
    try {
        std::string converted;
        converted.reserve(input.size());
        utf8::utf32to8(input.begin(), input.end(), std::back_inserter(converted));
        output.swap(converted);
        return true;
    } catch (...) {
        return ClearAndFail(output);
    }
}

bool Utf8ToWString(std::string_view input, std::wstring& output) noexcept {
    try {
        std::wstring converted;
        if constexpr (sizeof(wchar_t) == sizeof(char16_t)) {
            std::u16string utf16;
            if (!Utf8ToUtf16(input, utf16)) return ClearAndFail(output);
            converted.assign(utf16.begin(), utf16.end());
        } else if constexpr (sizeof(wchar_t) == sizeof(char32_t)) {
            std::u32string utf32;
            if (!Utf8ToUtf32(input, utf32)) return ClearAndFail(output);
            converted.assign(utf32.begin(), utf32.end());
        } else {
            return ClearAndFail(output);
        }
        output.swap(converted);
        return true;
    } catch (...) {
        return ClearAndFail(output);
    }
}

bool WStringToUtf8(std::wstring_view input, std::string& output) noexcept {
    try {
        if constexpr (sizeof(wchar_t) == sizeof(char16_t)) {
            const std::u16string utf16(input.begin(), input.end());
            return Utf16ToUtf8(utf16, output);
        } else if constexpr (sizeof(wchar_t) == sizeof(char32_t)) {
            const std::u32string utf32(input.begin(), input.end());
            return Utf32ToUtf8(utf32, output);
        } else {
            return ClearAndFail(output);
        }
    } catch (...) {
        return ClearAndFail(output);
    }
}

} // namespace PoDoFo::utls

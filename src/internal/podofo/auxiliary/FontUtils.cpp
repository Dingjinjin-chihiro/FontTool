// Font-only adaptation derived from PoDoFo.
// SPDX-License-Identifier: LGPL-2.0-or-later OR MPL-2.0
#include "FontUtils.h"
#include "InputDevice.h"
#include <utf8cpp/utf8/unchecked.h>
#include <utf8proc.h>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <limits>
#include <type_traits>

namespace PoDoFo::utls {
namespace {

std::ifstream OpenInputFile(std::string_view filepath, std::ios_base::openmode mode) noexcept {
#ifdef _WIN32
    std::wstring filepathWide;
    if (!Utf8ToWString(filepath, filepathWide)) return {};
    return std::ifstream(filepathWide.c_str(), mode);
#else
    return std::ifstream(std::string(filepath), mode);
#endif
}

std::ofstream OpenOutputFile(std::string_view filepath, std::ios_base::openmode mode) noexcept {
#ifdef _WIN32
    std::wstring filepathWide;
    if (!Utf8ToWString(filepath, filepathWide)) return {};
    return std::ofstream(filepathWide.c_str(), mode);
#else
    return std::ofstream(std::string(filepath), mode);
#endif
}



} // namespace

bool ReadTo(charbuff& output, std::string_view filepath, size_t maxReadSize) noexcept {
    output.clear();
    if (filepath.empty()) return false;
    auto stream = OpenInputFile(filepath, std::ios::binary);
    if (!stream.is_open()) return false;
    stream.seekg(0, std::ios::end);
    const auto end = stream.tellg();
    if (end < 0) return false;
    size_t size = static_cast<size_t>(end);
    if (maxReadSize != 0) size = std::min(size, maxReadSize);
    output.resize(size);
    stream.seekg(0, std::ios::beg);
    if (size != 0) stream.read(output.data(), static_cast<std::streamsize>(size));
    return stream.good() || (stream.eof() && static_cast<size_t>(stream.gcount()) == size);
}

bool ReadTo(charbuff& output, InputStreamDevice& input, size_t maxReadSize) noexcept {
    output.clear();
    const auto originalPosition = input.GetPosition();
    const auto length = input.GetLength();
    if (!input.Seek(0, SeekDirection::Begin)) return false;
    size_t size = length;
    if (maxReadSize != 0) size = std::min(size, maxReadSize);
    output.resize(size);
    const bool ok = size == 0 || input.Read(output.data(), size);
    const bool restored = input.Seek(static_cast<std::int64_t>(originalPosition), SeekDirection::Begin);
    if (!ok || !restored) {
        output.clear();
        return false;
    }
    return true;
}

bool WriteTo(const bufferview& input, std::string_view filepath) noexcept {
    if (filepath.empty()) return false;
    auto stream = OpenOutputFile(filepath, std::ios::binary | std::ios::trunc);
    if (!stream.is_open()) return false;
    if (!input.empty()) stream.write(input.data(), static_cast<std::streamsize>(input.size()));
    stream.flush();
    return stream.good();
}



namespace
{
    bool IsDiscardedFontNameCodePoint(utf8proc_int32_t codePoint) noexcept
    {
        const auto category =
            utf8proc_category(codePoint);

        switch (category)
        {
        case UTF8PROC_CATEGORY_ZS: // Space separator
        case UTF8PROC_CATEGORY_ZL: // Line separator
        case UTF8PROC_CATEGORY_ZP: // Paragraph separator
        case UTF8PROC_CATEGORY_CC: // Control character
        case UTF8PROC_CATEGORY_CF: // Format character
            return true;

        default:
            break;
        }

        // Ignore common ASCII separators while matching font names.
        // For example, "DejaVu-Sans", "DejaVu Sans" and "DejaVu_Sans"
        // normalize to the same key.
        switch (codePoint)
        {
        case '-':
        case '_':
        case '.':
            return true;

        default:
            return false;
        }
    }
}

std::string NormalizeFontName(
    std::string_view input) noexcept
{
    if (input.empty())
        return {};

    if (input.size() >
        static_cast<std::size_t>(
            (std::numeric_limits<utf8proc_ssize_t>::max)()))
    {
        return {};
    }

    utf8proc_uint8_t* mapped = nullptr;

    constexpr auto options =
        static_cast<utf8proc_option_t>(
            UTF8PROC_STABLE
            | UTF8PROC_COMPAT
            | UTF8PROC_DECOMPOSE
            | UTF8PROC_CASEFOLD
            | UTF8PROC_STRIPMARK
            | UTF8PROC_IGNORE);

    const auto mappedSize =
        utf8proc_map(
            reinterpret_cast<const utf8proc_uint8_t*>(
                input.data()),
            static_cast<utf8proc_ssize_t>(
                input.size()),
            &mapped,
            options);

    if (mappedSize < 0 || mapped == nullptr)
    {
        if (mapped != nullptr)
            std::free(mapped);

        return {};
    }

    std::string normalized;
    normalized.reserve(
        static_cast<std::size_t>(mappedSize));

    const auto* current = mapped;
    const auto* end = mapped + mappedSize;

    while (current < end)
    {
        utf8proc_int32_t codePoint = 0;

        const auto consumed =
            utf8proc_iterate(
                current,
                static_cast<utf8proc_ssize_t>(
                    end - current),
                &codePoint);

        if (consumed <= 0)
        {
            std::free(mapped);
            return {};
        }

        if (!IsDiscardedFontNameCodePoint(codePoint))
        {
            normalized.append(
                reinterpret_cast<const char*>(current),
                static_cast<std::size_t>(consumed));
        }

        current += consumed;
    }

    std::free(mapped);
    return normalized;
}


} // namespace PoDoFo::utls

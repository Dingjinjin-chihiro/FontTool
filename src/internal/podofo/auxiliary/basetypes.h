// Derived from PoDoFo auxiliary/basetypes.h
// SPDX-License-Identifier: LGPL-2.0-or-later OR MPL-2.0
#pragma once
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include "span.h"
namespace PoDoFo {
using bufferview = cspan<char>;
using bufferspan = mspan<char>;
using unicodeview = cspan<char32_t>;
class charbuff final : public std::string {
public:
    using std::string::string;
    charbuff() noexcept = default;
    charbuff(const std::string& value) : std::string(value) {}
    charbuff(std::string&& value) noexcept : std::string(std::move(value)) {}
    explicit charbuff(const bufferview& view) : std::string(view.data(), view.size()) {}
    charbuff& operator=(const bufferview& view) { assign(view.data(), view.size()); return *this; }
};
class datahandle final {
public:
    datahandle() = default;
    explicit datahandle(bufferview view) : m_view(view) {}
    explicit datahandle(charbuff&& buffer) : m_buffer(std::make_shared<charbuff>(std::move(buffer))), m_view(*m_buffer) {}
    explicit datahandle(std::shared_ptr<const charbuff> buffer) : m_buffer(std::move(buffer)), m_view(*m_buffer) {}
    const bufferview& view() const noexcept { return m_view; }
private:
    std::shared_ptr<const charbuff> m_buffer;
    bufferview m_view;
};
}

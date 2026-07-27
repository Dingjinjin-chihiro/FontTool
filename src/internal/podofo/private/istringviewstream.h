// Font-only adaptation derived from PoDoFo.
// SPDX-License-Identifier: LGPL-2.0-or-later OR MPL-2.0
// Reused lightweight PoDoFo private/istringviewstream.h utility.
#pragma once
#include <istream>
#include <streambuf>
#include <string_view>
namespace cmn {
template <typename CharT> class stringviewbuffer final:public std::basic_streambuf<CharT>{public:explicit stringviewbuffer(std::basic_string_view<CharT> v){auto* b=const_cast<CharT*>(v.data());this->setg(b,b,b+v.size());}};
template <typename CharT> class basic_istringviewstream final:public std::basic_istream<CharT>{public:explicit basic_istringviewstream(std::basic_string_view<CharT> v):std::basic_istream<CharT>(nullptr),m_buffer(v){this->rdbuf(&m_buffer);}private:stringviewbuffer<CharT> m_buffer;};
using istringviewstream=basic_istringviewstream<char>; using wistringviewstream=basic_istringviewstream<wchar_t>;
}

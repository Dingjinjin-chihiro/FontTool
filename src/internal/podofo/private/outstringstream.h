// Font-only adaptation derived from PoDoFo.
// SPDX-License-Identifier: LGPL-2.0-or-later OR MPL-2.0
// Reused lightweight PoDoFo private/outstringstream.h utility.
#pragma once
#include <ostream>
#include <streambuf>
#include <string>
#include <string_view>
namespace cmn {
template <typename CharT> class basic_outstringstream final:public std::basic_ostream<CharT>{class buffer final:public std::basic_streambuf<CharT>{public:explicit buffer(size_t reserve=8){m_data.reserve(reserve);syncPointers();}std::basic_string_view<CharT> str() const{return {m_data.data(),size()};}std::basic_string<CharT> take_str(){m_data.resize(size());auto out=std::move(m_data);m_data={};syncPointers();return out;}size_t size() const{return static_cast<size_t>(this->pptr()-this->pbase());}protected:typename std::basic_streambuf<CharT>::int_type overflow(typename std::basic_streambuf<CharT>::int_type ch) override{if(ch==std::char_traits<CharT>::eof())return ch;const auto used=size();m_data.resize(used+std::max<size_t>(used,8));syncPointers();this->pbump(static_cast<int>(used));*this->pptr()=static_cast<CharT>(ch);this->pbump(1);return ch;}private:void syncPointers(){if(m_data.empty())m_data.resize(8);this->setp(m_data.data(),m_data.data()+m_data.size());}std::basic_string<CharT> m_data;};public:explicit basic_outstringstream(size_t reserve=8):std::basic_ostream<CharT>(nullptr),m_buffer(reserve){this->rdbuf(&m_buffer);}std::basic_string_view<CharT> str() const{return m_buffer.str();}std::basic_string<CharT> take_str(){return m_buffer.take_str();}size_t size() const{return m_buffer.size();}private:buffer m_buffer;};
using outstringstream=basic_outstringstream<char>; using woutstringstream=basic_outstringstream<wchar_t>;
}

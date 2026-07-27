// Font-only adaptation derived from PoDoFo.
// SPDX-License-Identifier: LGPL-2.0-or-later OR MPL-2.0
#include "StreamDevice.h"
#include <climits>
#include <limits>
#if defined(_WIN32)
#include <io.h>
#else
#include <unistd.h>
#endif
namespace PoDoFo {
bool StreamDevice::SeekPosition(size_t current,size_t length,std::int64_t offset,SeekDirection direction,size_t& result) noexcept{std::int64_t base=0;if(direction==SeekDirection::Current){if(current>static_cast<size_t>(std::numeric_limits<std::int64_t>::max()))return false;base=static_cast<std::int64_t>(current);}else if(direction==SeekDirection::End){if(length>static_cast<size_t>(std::numeric_limits<std::int64_t>::max()))return false;base=static_cast<std::int64_t>(length);}if((offset>0&&base>std::numeric_limits<std::int64_t>::max()-offset)||(offset<0&&base<std::numeric_limits<std::int64_t>::min()-offset))return false;const auto value=base+offset;if(value<0||static_cast<size_t>(value)>length)return false;result=static_cast<size_t>(value);return true;}
StandardStreamDevice::StandardStreamDevice(DeviceAccess access,std::istream* input,std::ostream* output) noexcept:StreamDevice(access),m_input(input),m_output(output){}
StandardStreamDevice::StandardStreamDevice(std::istream& stream) noexcept:StandardStreamDevice(DeviceAccess::Read,&stream,nullptr){}
StandardStreamDevice::StandardStreamDevice(std::ostream& stream) noexcept:StandardStreamDevice(DeviceAccess::Write,nullptr,&stream){}
StandardStreamDevice::StandardStreamDevice(std::iostream& stream) noexcept:StandardStreamDevice(DeviceAccess::ReadWrite,&stream,&stream){}
size_t StandardStreamDevice::GetLength() const noexcept{if(m_input){auto& s=*m_input;const auto p=s.tellg();if(p<0)return 0;s.clear();s.seekg(0,std::ios::end);const auto e=s.tellg();s.clear();s.seekg(p);return e<0?0:static_cast<size_t>(e);}if(m_output){auto& s=*m_output;const auto p=s.tellp();return p<0?0:static_cast<size_t>(p);}return 0;}
size_t StandardStreamDevice::GetPosition() const noexcept{if(m_input){const auto p=m_input->tellg();return p<0?0:static_cast<size_t>(p);}if(m_output){const auto p=m_output->tellp();return p<0?0:static_cast<size_t>(p);}return 0;}
bool StandardStreamDevice::Eof() const noexcept{return !m_input||m_input->eof();}
bool StandardStreamDevice::writeBuffer(const char* b,size_t n) noexcept{if(!m_output||(!b&&n))return false;m_output->write(b,static_cast<std::streamsize>(n));return m_output->good();}
bool StandardStreamDevice::flush() noexcept{if(!m_output)return false;m_output->flush();return m_output->good();}
bool StandardStreamDevice::readBuffer(char* b,size_t n,size_t& read,bool& eof) noexcept{read=0;eof=true;if(!m_input||(!b&&n))return false;m_input->read(b,static_cast<std::streamsize>(n));read=static_cast<size_t>(m_input->gcount());eof=m_input->eof();return read==n||eof;}
bool StandardStreamDevice::readChar(char& ch) noexcept{if(!m_input||!m_input->get(ch)){ch='\0';return false;}return true;}
bool StandardStreamDevice::peek(char& ch) const noexcept{if(!m_input){ch='\0';return false;}const auto value=m_input->peek();if(value==std::char_traits<char>::eof()){ch='\0';return false;}ch=static_cast<char>(value);return true;}
bool StandardStreamDevice::seek(std::int64_t offset,SeekDirection d) noexcept{const auto dir=d==SeekDirection::Begin?std::ios::beg:(d==SeekDirection::Current?std::ios::cur:std::ios::end);bool ok=false;if(m_input){m_input->clear();m_input->seekg(static_cast<std::streamoff>(offset),dir);ok=m_input->good();}if(m_output){m_output->clear();m_output->seekp(static_cast<std::streamoff>(offset),dir);ok=m_output->good()||ok;}return ok;}
namespace { const char* OpenMode(DeviceAccess access,FileMode mode) noexcept{if(access==DeviceAccess::Read)return "rb";if(access==DeviceAccess::Write){if(mode==FileMode::Append)return "ab";if(mode==FileMode::CreateNew)return "wbx";return "wb";}if(mode==FileMode::Append)return "a+b";if(mode==FileMode::Create||mode==FileMode::Truncate)return "w+b";if(mode==FileMode::CreateNew)return "w+bx";return "r+b";} }
FileStreamDevice::FileStreamDevice(const std::string_view& p) noexcept:FileStreamDevice(p,FileMode::Open,DeviceAccess::Read){}
FileStreamDevice::FileStreamDevice(const std::string_view& p,DeviceAccess a,bool truncateFile) noexcept:FileStreamDevice(p,truncateFile?FileMode::Create:(a==DeviceAccess::Write?FileMode::Append:FileMode::Open),a){}
FileStreamDevice::FileStreamDevice(const std::string_view& p,FileMode mode,DeviceAccess access) noexcept:StreamDevice(access),m_filepath(p){m_file=std::fopen(m_filepath.c_str(),OpenMode(access,mode));if(!m_file&&mode==FileMode::OpenOrCreate)m_file=std::fopen(m_filepath.c_str(),access==DeviceAccess::Read?"rb":"w+b");}
FileStreamDevice::~FileStreamDevice(){close();}
size_t FileStreamDevice::GetLength() const noexcept{if(!m_file)return 0;const auto pos=std::ftell(m_file);if(pos<0)return 0;if(std::fseek(m_file,0,SEEK_END)!=0)return 0;const auto len=std::ftell(m_file);(void)std::fseek(m_file,pos,SEEK_SET);return len<0?0:static_cast<size_t>(len);}
size_t FileStreamDevice::GetPosition() const noexcept{if(!m_file)return 0;const auto p=std::ftell(m_file);return p<0?0:static_cast<size_t>(p);}
bool FileStreamDevice::Eof() const noexcept{return !m_file||std::feof(m_file)!=0;}
bool FileStreamDevice::writeBuffer(const char* b,size_t n) noexcept{return m_file&&(!n||std::fwrite(b,1,n,m_file)==n);}
bool FileStreamDevice::flush() noexcept{return m_file&&std::fflush(m_file)==0;}
bool FileStreamDevice::readBuffer(char* b,size_t n,size_t& read,bool& eof) noexcept{read=0;eof=true;if(!m_file||(!b&&n))return false;read=std::fread(b,1,n,m_file);eof=std::feof(m_file)!=0;return read==n||eof||std::ferror(m_file)==0;}
bool FileStreamDevice::readChar(char& ch) noexcept{if(!m_file){ch='\0';return false;}const auto v=std::fgetc(m_file);if(v==EOF){ch='\0';return false;}ch=static_cast<char>(v);return true;}
bool FileStreamDevice::peek(char& ch) const noexcept{if(!m_file){ch='\0';return false;}const auto v=std::fgetc(m_file);if(v==EOF){ch='\0';return false;}ch=static_cast<char>(v);return std::ungetc(v,m_file)!=EOF;}
bool FileStreamDevice::seek(std::int64_t o,SeekDirection d) noexcept{if(!m_file||o>LONG_MAX||o<LONG_MIN)return false;const int origin=d==SeekDirection::Begin?SEEK_SET:(d==SeekDirection::Current?SEEK_CUR:SEEK_END);return std::fseek(m_file,static_cast<long>(o),origin)==0;}
bool FileStreamDevice::close() noexcept{if(!m_file)return true;const auto rc=std::fclose(m_file);m_file=nullptr;return rc==0;}
bool FileStreamDevice::truncate() noexcept{if(!m_file)return false;const auto p=std::ftell(m_file);if(p<0)return false;
#if defined(_WIN32)
return _chsize(_fileno(m_file),p)==0;
#else
return ftruncate(fileno(m_file),p)==0;
#endif
}
bool SpanStreamDevice::writeBuffer(const char* b,size_t n) noexcept{if(!m_buffer||(!b&&n)||m_position+n>m_length)return false;if(n)std::memcpy(m_buffer+m_position,b,n);m_position+=n;return true;}
bool SpanStreamDevice::readBuffer(char* b,size_t n,size_t& read,bool& eof) noexcept{read=0;eof=true;if(!m_buffer||(!b&&n)||m_position>m_length)return false;read=std::min(n,m_length-m_position);if(read)std::memcpy(b,m_buffer+m_position,read);m_position+=read;eof=m_position>=m_length;return true;}
bool SpanStreamDevice::readChar(char& ch) noexcept{if(Eof()){ch='\0';return false;}ch=m_buffer[m_position++];return true;}
bool SpanStreamDevice::peek(char& ch) const noexcept{if(Eof()){ch='\0';return false;}ch=m_buffer[m_position];return true;}
bool SpanStreamDevice::seek(std::int64_t o,SeekDirection d) noexcept{size_t next=0;if(!SeekPosition(m_position,m_length,o,d,next))return false;m_position=next;return true;}
}

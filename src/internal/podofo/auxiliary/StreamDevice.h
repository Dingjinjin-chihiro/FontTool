// Adapted from PoDoFo auxiliary/StreamDevice.h: same device family, explicit bool failures.
// SPDX-License-Identifier: LGPL-2.0-or-later OR MPL-2.0
#pragma once
#include "InputDevice.h"
#include "OutputDevice.h"
#include "basetypes.h"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <ios>
#include <istream>
#include <ostream>
#include <string>
#include <vector>
namespace PoDoFo {
class PODOFO_API StreamDevice : public InputStreamDevice, public OutputStreamDevice {
protected:
    explicit StreamDevice(DeviceAccess access) noexcept { SetAccess(access); }
    static bool SeekPosition(size_t current,size_t length,std::int64_t offset,SeekDirection direction,size_t& result) noexcept;
};
class PODOFO_API StandardStreamDevice final : public StreamDevice {
public:
    explicit StandardStreamDevice(std::istream& stream) noexcept;
    explicit StandardStreamDevice(std::ostream& stream) noexcept;
    explicit StandardStreamDevice(std::iostream& stream) noexcept;
    size_t GetLength() const noexcept override;
    size_t GetPosition() const noexcept override;
    bool CanSeek() const noexcept override { return true; }
    bool Eof() const noexcept override;
protected:
    bool writeBuffer(const char*,size_t) noexcept override;
    bool flush() noexcept override;
    bool readBuffer(char*,size_t,size_t&,bool&) noexcept override;
    bool readChar(char&) noexcept override;
    bool peek(char&) const noexcept override;
    bool seek(std::int64_t,SeekDirection) noexcept override;
    bool truncate() noexcept override { return false; }
private:
    StandardStreamDevice(DeviceAccess,std::istream*,std::ostream*) noexcept;
    std::istream* m_input=nullptr;
    std::ostream* m_output=nullptr;
};
enum class FileMode : uint8_t { CreateNew=1,Create,Open,OpenOrCreate,Truncate,Append };
class PODOFO_API FileStreamDevice final : public StreamDevice {
public:
    explicit FileStreamDevice(const std::string_view& filepath) noexcept;
    FileStreamDevice(const std::string_view& filepath,DeviceAccess access,bool truncate=false) noexcept;
    FileStreamDevice(const std::string_view& filepath,FileMode mode,DeviceAccess access=DeviceAccess::ReadWrite) noexcept;
    ~FileStreamDevice() override;
    bool IsOpen() const noexcept { return m_file!=nullptr; }
    const std::string& GetFilepath() const noexcept { return m_filepath; }
    size_t GetLength() const noexcept override;
    size_t GetPosition() const noexcept override;
    bool CanSeek() const noexcept override { return true; }
    bool Eof() const noexcept override;
protected:
    bool writeBuffer(const char*,size_t) noexcept override;
    bool flush() noexcept override;
    bool readBuffer(char*,size_t,size_t&,bool&) noexcept override;
    bool readChar(char&) noexcept override;
    bool peek(char&) const noexcept override;
    bool seek(std::int64_t,SeekDirection) noexcept override;
    bool close() noexcept override;
    bool truncate() noexcept override;
private:
    FILE* m_file=nullptr;
    std::string m_filepath;
};
template <typename TContainer> class ContainerStreamDevice : public StreamDevice {
public:
    explicit ContainerStreamDevice(const TContainer& input) noexcept:StreamDevice(DeviceAccess::Read),m_container(const_cast<TContainer*>(&input)),m_position(0){}
    explicit ContainerStreamDevice(TContainer& output) noexcept:StreamDevice(DeviceAccess::ReadWrite),m_container(&output),m_position(output.size()){}
    ContainerStreamDevice(TContainer& data,DeviceAccess access,bool atEnd=false) noexcept:StreamDevice(access),m_container(&data),m_position(atEnd?data.size():0){}
    size_t GetLength() const noexcept override{return m_container?m_container->size():0;}
    size_t GetPosition() const noexcept override{return m_position;}
    bool CanSeek() const noexcept override{return true;}
    bool Eof() const noexcept override{return !m_container||m_position>=m_container->size();}
protected:
    bool writeBuffer(const char* buffer,size_t size) noexcept override{if(!m_container||(!buffer&&size)||m_position+size<m_position)return false;if(m_position+size>m_container->size())m_container->resize(m_position+size);if(size)std::memcpy(m_container->data()+m_position,buffer,size);m_position+=size;return true;}
    bool flush() noexcept override{return true;}
    bool readBuffer(char* buffer,size_t size,size_t& read,bool& eof) noexcept override{read=0;eof=true;if(!m_container||(!buffer&&size)||m_position>m_container->size())return false;read=std::min(size,m_container->size()-m_position);if(read)std::memcpy(buffer,m_container->data()+m_position,read);m_position+=read;eof=m_position>=m_container->size();return true;}
    bool readChar(char& ch) noexcept override{if(Eof()){ch='\0';return false;}ch=(*m_container)[m_position++];return true;}
    bool peek(char& ch) const noexcept override{if(Eof()){ch='\0';return false;}ch=(*m_container)[m_position];return true;}
    bool seek(std::int64_t offset,SeekDirection direction) noexcept override{if(!m_container)return false;size_t next=0;if(!SeekPosition(m_position,m_container->size(),offset,direction,next))return false;m_position=next;return true;}
    bool truncate() noexcept override{if(!m_container||m_position>m_container->size())return false;m_container->resize(m_position);return true;}
private:TContainer* m_container=nullptr;size_t m_position=0;
};
class PODOFO_API SpanStreamDevice final : public StreamDevice {
public:
    explicit SpanStreamDevice(const bufferview& view) noexcept:StreamDevice(DeviceAccess::Read),m_buffer(const_cast<char*>(view.data())),m_length(view.size()){}
    SpanStreamDevice(char* data,size_t size,DeviceAccess access=DeviceAccess::ReadWrite) noexcept:StreamDevice(access),m_buffer(data),m_length(size){}
    size_t GetLength() const noexcept override{return m_length;}
    size_t GetPosition() const noexcept override{return m_position;}
    bool CanSeek() const noexcept override{return true;}
    bool Eof() const noexcept override{return m_position>=m_length;}
protected:
    bool writeBuffer(const char*,size_t) noexcept override;
    bool flush() noexcept override{return true;}
    bool readBuffer(char*,size_t,size_t&,bool&) noexcept override;
    bool readChar(char&) noexcept override;
    bool peek(char&) const noexcept override;
    bool seek(std::int64_t,SeekDirection) noexcept override;
    bool truncate() noexcept override{if(m_position>m_length)return false;m_length=m_position;return true;}
private:char* m_buffer=nullptr;size_t m_length=0;size_t m_position=0;
};
class PODOFO_API NullStreamDevice final : public StreamDevice {
public:
    NullStreamDevice() noexcept:StreamDevice(DeviceAccess::ReadWrite){}
    size_t GetLength() const noexcept override{return m_length;}
    size_t GetPosition() const noexcept override{return m_position;}
    bool CanSeek() const noexcept override{return true;}
    bool Eof() const noexcept override{return m_position>=m_length;}
protected:
    bool writeBuffer(const char*,size_t size) noexcept override{if(m_position+size<m_position)return false;m_position+=size;if(m_position>m_length)m_length=m_position;return true;}
    bool flush() noexcept override{return true;}
    bool readBuffer(char* buffer,size_t size,size_t& read,bool& eof) noexcept override{read=std::min(size,m_length-m_position);if(buffer&&read)std::memset(buffer,0,read);m_position+=read;eof=Eof();return buffer||size==0;}
    bool readChar(char& ch) noexcept override{if(Eof()){ch='\0';return false;}ch='\0';++m_position;return true;}
    bool peek(char& ch) const noexcept override{ch='\0';return !Eof();}
    bool seek(std::int64_t offset,SeekDirection direction) noexcept override{size_t next=0;if(!SeekPosition(m_position,m_length,offset,direction,next))return false;m_position=next;return true;}
    bool truncate() noexcept override{if(m_position>m_length)return false;m_length=m_position;return true;}
private:size_t m_length=0;size_t m_position=0;
};
using VectorStreamDevice=ContainerStreamDevice<std::vector<char>>;
using StringStreamDevice=ContainerStreamDevice<std::string>;
using BufferStreamDevice=ContainerStreamDevice<charbuff>;
}

// Adapted from PoDoFo auxiliary/StreamDeviceBase.h: bool-return API.
// SPDX-License-Identifier: LGPL-2.0-or-later OR MPL-2.0
#pragma once
#include "EnumFlags.h"
#include "basedefs.h"
#include <cstdint>
#include <cstddef>
namespace PoDoFo {
enum class DeviceAccess : uint8_t { Read=1, Write=2, ReadWrite=3 };
enum class SeekDirection : uint8_t { Begin=0, Current, End };
ENABLE_BITMASK_OPERATORS(DeviceAccess)
class PODOFO_API StreamDeviceBase {
public:
    virtual ~StreamDeviceBase() = default;
    DeviceAccess GetAccess() const noexcept { return m_access; }
    bool SetAccess(DeviceAccess access) noexcept { m_access=access; return true; }
    bool Seek(std::int64_t offset, SeekDirection direction) noexcept;
    bool Close() noexcept { return close(); }
    virtual size_t GetLength() const noexcept = 0;
    virtual size_t GetPosition() const noexcept = 0;
    virtual bool CanSeek() const noexcept { return false; }
protected:
    bool EnsureAccess(DeviceAccess access) const noexcept { return (m_access & access) != DeviceAccess{}; }
    virtual bool seek(std::int64_t, SeekDirection) noexcept { return false; }
    virtual bool close() noexcept { return true; }
private:
    DeviceAccess m_access = DeviceAccess::Read;
};
}

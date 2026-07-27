// Font-only adaptation derived from PoDoFo.
// SPDX-License-Identifier: LGPL-2.0-or-later OR MPL-2.0
// Adapted from PoDoFo auxiliary/OutputDevice.h
#pragma once
#include "OutputStream.h"
#include "StreamDeviceBase.h"
namespace PoDoFo {
class PODOFO_API OutputStreamDevice : public OutputStream, virtual public StreamDeviceBase {
public:
    bool Truncate() noexcept { return EnsureAccess(DeviceAccess::Write) && truncate(); }
protected:
    bool checkWrite() const noexcept override { return EnsureAccess(DeviceAccess::Write); }
    virtual bool truncate() noexcept = 0;
};
}

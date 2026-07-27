// Font-only adaptation derived from PoDoFo.
// SPDX-License-Identifier: LGPL-2.0-or-later OR MPL-2.0
// Adapted from PoDoFo auxiliary/InputDevice.h
#pragma once
#include "InputStream.h"
#include "StreamDeviceBase.h"
namespace PoDoFo {
class PODOFO_API InputStreamDevice : public InputStream, virtual public StreamDeviceBase {
public:
    virtual bool Eof() const noexcept = 0;
    bool Peek(char& ch) const noexcept { return peek(ch); }
protected:
    bool checkRead() const noexcept override { return EnsureAccess(DeviceAccess::Read); }
    virtual bool peek(char& ch) const noexcept = 0;
};
}

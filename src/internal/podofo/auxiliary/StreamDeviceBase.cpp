// Font-only adaptation derived from PoDoFo.
// SPDX-License-Identifier: LGPL-2.0-or-later OR MPL-2.0
#include "StreamDeviceBase.h"
namespace PoDoFo {
bool StreamDeviceBase::Seek(std::int64_t offset, SeekDirection direction) noexcept { return CanSeek() && seek(offset,direction); }
}

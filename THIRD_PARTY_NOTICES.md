# Third-Party Notices

| Component | Use | License | Location |
|---|---|---|---|
| PoDoFo-derived font code | Font loading adapters, TrueType subsetting and utility code | LGPL-2.0-or-later OR MPL-2.0 | `src/internal/podofo` |
| Adobe AFDKO | Type1/CFF parsing and subsetting | Apache-2.0 | `third_party/adobe/afdko` |
| utf8proc | Unicode support | MIT | `third_party/utf8proc` |
| utf8cpp | Checked UTF-8 decoding | Upstream permissive license | `third_party/utf8cpp` |
| tcb::span | C++17 span implementation | BSL-1.0 | `src/internal/podofo/3rdparty/span.hpp` |
| FreeType | Font parser and metrics engine | FreeType License or GPL, depending on distribution choice | External dependency |

Full bundled license texts are under `LICENSES/`. AFDKO also retains its upstream `LICENSE` inside its source directory.

Before publishing a binary release, include `NOTICE`, `THIRD_PARTY_NOTICES.md` and `LICENSES/` with the distribution. If FreeType binaries are redistributed, include the corresponding FreeType license files from that binary distribution.

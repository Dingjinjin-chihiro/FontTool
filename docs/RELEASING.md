# Release Checklist

1. Update `project(VERSION ...)`, `include/fonttool/version.hpp` and `CHANGELOG.md` together.
2. Build static and shared configurations with `FONTTOOL_WARNINGS_AS_ERRORS=ON`.
3. Run tests with at least one TrueType font and one OpenType/CFF or Type1 font.
4. Install to a temporary prefix and build an external `find_package(FontTool CONFIG)` consumer.
5. Check that the source archive excludes build directories and generated font files.
6. Include `LICENSE`, `NOTICE`, `THIRD_PARTY_NOTICES.md` and `LICENSES/` in every source or binary release.
7. Tag releases as `vMAJOR.MINOR.PATCH`.

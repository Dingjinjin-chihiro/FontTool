# Changelog

All notable changes follow Semantic Versioning.

## 1.0.0

- Extracted the font subsystem into an independent C++17 library with no PDF object dependency.
- Added a stable `fonttool` namespace and PImpl-based public API.
- Added file/memory loading, collection face enumeration, glyph lookup and advance queries.
- Added UTF-8, Unicode, GID and CID-based subsetting.
- Added TrueType and CFF/Type1 subset backends with source-to-subset mapping results.
- Added optional CLI, examples, tests, install rules, CMake package config and pkg-config metadata.
- Added static/shared builds and repository-ready licensing and CI files.

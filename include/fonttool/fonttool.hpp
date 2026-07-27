// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <fonttool/export.hpp>
#include <fonttool/version.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace fonttool {

/// Stable error categories returned by the public API.
enum class ErrorCode : std::uint8_t {
    Ok = 0,
    InvalidArgument,
    IoError,
    InvalidFont,
    UnsupportedFont,
    FaceNotFound,
    NotLoaded,
    InvalidUtf8,
    MissingGlyph,
    SubsetFailed,
    WriteFailed,
};

/// Non-throwing operation result. A successful status has code == Ok.
struct Status final {
    ErrorCode code = ErrorCode::Ok;
    std::string message;

    [[nodiscard]] bool ok() const noexcept { return code == ErrorCode::Ok; }
    [[nodiscard]] explicit operator bool() const noexcept { return ok(); }
};

/// Font program format detected by FreeType or emitted by the subset backend.
enum class FontFormat : std::uint8_t {
    Unknown = 0,
    Type1,
    TrueType,
    OpenTypeCff,
    Type1Cff,
    CidKeyedCff,
    Cff2,
};

/// Metadata for one face in a standalone font or font collection.
struct FaceInfo final {
    std::uint32_t index = 0;
    std::uint32_t face_count = 0;
    std::string family_name;
    std::string style_name;
    std::string postscript_name;
    FontFormat format = FontFormat::Unknown;
    bool scalable = false;
    bool has_unicode_charmap = false;
    bool cid_keyed = false;
    std::uint32_t glyph_count = 0;
    std::uint32_t units_per_em = 0;
};

/// Controls how the output font program is generated.
struct SubsetOptions final {
    /// Preserve source glyph IDs in the generated font.
    bool retain_gids = false;

    /// Preserve maxp.numGlyphs. Requires retain_gids.
    bool retain_glyph_count = false;

    /// Preserve the original cmap. Requires retain_gids and
    /// retain_glyph_count because cmap glyph IDs must remain unchanged.
    bool include_cmap = false;

    /// Preserve TrueType hinting tables when available.
    bool include_hinting = true;
};

/// Mapping from a source glyph ID to the generated subset glyph ID.
struct GlyphMapping final {
    std::uint32_t source_gid = 0;
    std::uint32_t subset_gid = 0;
};

/// Mapping from a PDF-style CID to the generated subset glyph ID.
struct CidMapping final {
    std::uint16_t cid = 0;
    std::uint32_t subset_gid = 0;
};

/// ROS metadata used when generating a CID-keyed CFF program.
struct CidSystemInfo final {
    std::string registry = "Adobe";
    std::string ordering = "Identity";
    int supplement = 0;
};

/// Explicit CID, source GID and optional Unicode sequence mapping.
struct CidGlyphMapping final {
    std::uint16_t cid = 0;
    std::uint32_t source_gid = 0;
    std::u32string unicode;
};

/// Generated font bytes plus mappings required by PDF/font consumers.
struct SubsetResult final {
    std::vector<std::uint8_t> font_program;
    FontFormat format = FontFormat::Unknown;
    bool retains_original_gids = false;
    bool retains_original_glyph_count = false;
    bool identity_cid_to_gid = false;
    std::vector<GlyphMapping> glyph_mappings;
    std::vector<CidMapping> cid_mappings;

    void clear() noexcept {
        font_program.clear();
        format = FontFormat::Unknown;
        retains_original_gids = false;
        retains_original_glyph_count = false;
        identity_cid_to_gid = false;
        glyph_mappings.clear();
        cid_mappings.clear();
    }
};

/// Stateful, move-only font inspection and subsetting object.
///
/// A FontTool instance owns the loaded font bytes. The caller may release the
/// original input buffer immediately after load_memory() returns. Distinct
/// instances may be used concurrently; one instance must not be accessed by
/// multiple threads without external synchronization.
class FONTTOOL_API FontTool final {
public:
    FontTool() noexcept;
    ~FontTool();

    FontTool(FontTool&& other) noexcept;
    FontTool& operator=(FontTool&& other) noexcept;

    FontTool(const FontTool&) = delete;
    FontTool& operator=(const FontTool&) = delete;

    /// Load one face from a file. Paths use the platform narrow-character
    /// encoding; UTF-8 paths are supported on platforms where narrow paths are UTF-8.
    [[nodiscard]] Status load_file(std::string_view path,
                                   std::uint32_t face_index = 0) noexcept;

    /// Load one face from memory. FontTool copies the bytes it needs.
    [[nodiscard]] Status load_memory(const std::uint8_t* data,
                                     std::size_t size,
                                     std::uint32_t face_index = 0) noexcept;
    [[nodiscard]] Status load_memory(const std::vector<std::uint8_t>& data,
                                     std::uint32_t face_index = 0) noexcept {
        return load_memory(data.data(), data.size(), face_index);
    }

    void reset() noexcept;
    [[nodiscard]] bool is_loaded() const noexcept;
    [[nodiscard]] FaceInfo info() const noexcept;

    /// Resolve a Unicode scalar value through the selected Unicode charmap.
    [[nodiscard]] Status glyph_id(char32_t code_point,
                                  std::uint32_t& gid) const noexcept;

    /// Return horizontal advance normalized to em units.
    [[nodiscard]] Status glyph_advance(std::uint32_t gid,
                                       double& advance_em) const noexcept;

    /// Build a subset from UTF-8 text. Duplicate characters are deduplicated.
    [[nodiscard]] Status subset_utf8(std::string_view text,
                                     const SubsetOptions& options,
                                     SubsetResult& result) const noexcept;

    /// Build a subset from Unicode scalar values.
    [[nodiscard]] Status subset_codepoints(const std::vector<char32_t>& code_points,
                                           const SubsetOptions& options,
                                           SubsetResult& result) const noexcept;

    /// Build a subset directly from source glyph IDs.
    [[nodiscard]] Status subset_glyphs(const std::vector<std::uint32_t>& gids,
                                       const SubsetOptions& options,
                                       SubsetResult& result) const noexcept;

    /// Build a subset using explicit CIDs. CID 0 is reserved for .notdef.
    [[nodiscard]] Status subset_cids(const std::vector<CidGlyphMapping>& mappings,
                                     const SubsetOptions& options,
                                     SubsetResult& result,
                                     const CidSystemInfo& cid_info = {}) const noexcept;

    /// Enumerate all faces from a file or memory buffer.
    [[nodiscard]] static Status inspect_file(std::string_view path,
                                             std::vector<FaceInfo>& faces) noexcept;
    [[nodiscard]] static Status inspect_memory(const std::uint8_t* data,
                                               std::size_t size,
                                               std::vector<FaceInfo>& faces) noexcept;

    /// Find a collection face by family, style, full or PostScript name.
    [[nodiscard]] static Status find_face_index(std::string_view path,
                                                std::string_view face_name,
                                                std::uint32_t& face_index) noexcept;

    /// Write generated font_program bytes to a file.
    [[nodiscard]] static Status write_file(std::string_view path,
                                           const SubsetResult& result) noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

[[nodiscard]] FONTTOOL_API const char* to_string(ErrorCode value) noexcept;
[[nodiscard]] FONTTOOL_API const char* to_string(FontFormat value) noexcept;
[[nodiscard]] FONTTOOL_API const char* version_string() noexcept;

} // namespace fonttool

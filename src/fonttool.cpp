// SPDX-License-Identifier: MPL-2.0
#include <fonttool/fonttool.hpp>

#include <podofo/main/FontMetrics.h>
#include <podofo/main/FontToolsFacade.h>

#include <algorithm>
#include <fstream>
#include <limits>
#include <new>

namespace fonttool {
namespace {

Status success() noexcept {
    return {};
}

Status failure(ErrorCode code, std::string message) noexcept {
    Status status;
    status.code = code;
    status.message = std::move(message);
    return status;
}

FontFormat convert_format(PoDoFo::FontFileType value) noexcept {
    switch (value) {
    case PoDoFo::FontFileType::Type1:
        return FontFormat::Type1;
    case PoDoFo::FontFileType::TrueType:
        return FontFormat::TrueType;
    case PoDoFo::FontFileType::OpenTypeCFF:
        return FontFormat::OpenTypeCff;
    case PoDoFo::FontFileType::Type1CFF:
        return FontFormat::Type1Cff;
    case PoDoFo::FontFileType::CIDKeyedCFF:
        return FontFormat::CidKeyedCff;
    case PoDoFo::FontFileType::CFF2:
        return FontFormat::Cff2;
    case PoDoFo::FontFileType::Unknown:
    default:
        return FontFormat::Unknown;
    }
}

FaceInfo convert_face_info(const FT::FontFaceInfo& input) noexcept {
    FaceInfo output;
    output.index = input.Index;
    output.face_count = input.FaceCount;
    output.family_name = input.FamilyName;
    output.style_name = input.StyleName;
    output.postscript_name = input.PostScriptName;
    output.format = convert_format(input.FileType);
    output.scalable = input.IsScalable;
    output.has_unicode_charmap = input.HasUnicodeCharmap;
    output.cid_keyed = input.IsCidKeyed;
    output.glyph_count = input.GlyphCount;
    output.units_per_em = input.UnitsPerEm;
    return output;
}

PoDoFo::FontSubsetOptions convert_options(const SubsetOptions& input) noexcept {
    PoDoFo::FontSubsetOptions output;
    output.RetainGids = input.retain_gids;
    output.RetainNumGlyphs = input.retain_glyph_count;
    output.IncludeCmap = input.include_cmap;
    output.IncludeHintingTables = input.include_hinting;
    return output;
}

ErrorCode classify_subset_error(std::string_view message) noexcept {
    if (message.find("no font face") != std::string_view::npos)
        return ErrorCode::NotLoaded;
    if (message.find("UTF-8") != std::string_view::npos)
        return ErrorCode::InvalidUtf8;
    if (message.find("does not contain") != std::string_view::npos ||
        message.find("missing glyph") != std::string_view::npos)
        return ErrorCode::MissingGlyph;
    if (message.find("unsupported") != std::string_view::npos)
        return ErrorCode::UnsupportedFont;
    return ErrorCode::SubsetFailed;
}

Status read_binary_file(std::string_view path, std::vector<std::uint8_t>& data) noexcept {
    data.clear();
    if (path.empty())
        return failure(ErrorCode::InvalidArgument, "font path is empty");

    const std::string path_string(path);
    std::ifstream input(path_string, std::ios::binary | std::ios::ate);
    if (!input)
        return failure(ErrorCode::IoError, "unable to open file: " + path_string);

    const std::streamoff end = input.tellg();
    if (end <= 0)
        return failure(ErrorCode::IoError, "file is empty or unreadable: " + path_string);
    if (static_cast<std::uintmax_t>(end) >
        static_cast<std::uintmax_t>(std::numeric_limits<std::size_t>::max()))
        return failure(ErrorCode::IoError, "file is too large: " + path_string);

    data.resize(static_cast<std::size_t>(end));
    input.seekg(0, std::ios::beg);
    input.read(reinterpret_cast<char*>(data.data()), end);
    if (!input)
        return failure(ErrorCode::IoError, "unable to read file: " + path_string);
    return success();
}

void convert_subset_result(const PoDoFo::FontSubsetPackage& package,
                           SubsetResult& result) noexcept {
    result.clear();
    result.font_program.assign(package.Subset.FontProgram.begin(),
                               package.Subset.FontProgram.end());
    result.format = convert_format(package.Subset.FileType);
    result.retains_original_gids = package.Subset.RetainsOriginalGids;
    result.retains_original_glyph_count = package.Subset.RetainsOriginalGlyphCount;
    result.identity_cid_to_gid = package.CanUseIdentityCidToGidMap;

    result.glyph_mappings.reserve(package.Subset.OriginalToSubsetGid.size());
    for (const auto& mapping : package.Subset.OriginalToSubsetGid) {
        result.glyph_mappings.push_back(
            {static_cast<std::uint32_t>(mapping.first),
             static_cast<std::uint32_t>(mapping.second)});
    }

    result.cid_mappings.reserve(package.Subset.CidToSubsetGid.size());
    for (const auto& mapping : package.Subset.CidToSubsetGid) {
        result.cid_mappings.push_back(
            {mapping.first, static_cast<std::uint32_t>(mapping.second)});
    }
}

Status finish_subset(bool ok,
                     const PoDoFo::FontSubsetPackage& package,
                     std::string error,
                     SubsetResult& result) noexcept {
    if (!ok) {
        result.clear();
        if (error.empty())
            error = "font subsetting failed";
        const ErrorCode code = classify_subset_error(error);
        return failure(code, std::move(error));
    }
    convert_subset_result(package, result);
    if (result.font_program.empty()) {
        result.clear();
        return failure(ErrorCode::SubsetFailed,
                       "subsetting succeeded without producing a font program");
    }
    return success();
}

} // namespace

struct FontTool::Impl final {
    PoDoFo::FontToolsFacade facade;
    std::uint32_t face_index = 0;
};


FontTool::FontTool() noexcept : impl_(new (std::nothrow) Impl()) {}
FontTool::~FontTool() = default;
FontTool::FontTool(FontTool&& other) noexcept = default;
FontTool& FontTool::operator=(FontTool&& other) noexcept = default;

Status FontTool::load_file(std::string_view path, std::uint32_t face_index) noexcept {
    std::vector<std::uint8_t> data;
    Status status = read_binary_file(path, data);
    if (!status)
        return status;
    return load_memory(data.data(), data.size(), face_index);
}

Status FontTool::load_memory(const std::uint8_t* data,
                             std::size_t size,
                             std::uint32_t face_index) noexcept {
    if (!impl_)
        return failure(ErrorCode::InvalidFont, "FontTool state allocation failed");
    impl_->facade.Reset();
    impl_->face_index = 0;

    if (data == nullptr || size == 0)
        return failure(ErrorCode::InvalidArgument, "font memory buffer is empty");

    const auto view = PoDoFo::bufferview(reinterpret_cast<const char*>(data), size);
    const auto count = PoDoFo::FontToolsFacade::GetFontFaceCount(view);
    if (!count)
        return failure(ErrorCode::InvalidFont, "buffer is not a readable font");
    if (face_index >= *count)
        return failure(ErrorCode::FaceNotFound, "font face index is out of range");
    if (!impl_->facade.LoadFontBuffer(view, face_index))
        return failure(ErrorCode::UnsupportedFont,
                       "font face is unsupported or could not be initialized");

    impl_->face_index = face_index;
    return success();
}

void FontTool::reset() noexcept {
    if (impl_) {
        impl_->facade.Reset();
        impl_->face_index = 0;
    }
}

bool FontTool::is_loaded() const noexcept {
    return impl_ && impl_->facade.IsLoaded();
}

FaceInfo FontTool::info() const noexcept {
    FaceInfo output;
    if (!is_loaded())
        return output;

    const auto* metrics = impl_->facade.GetMetrics();
    if (!metrics)
        return output;

    const auto source = metrics->GetOrLoadFontFileData();
    const auto faces = PoDoFo::FontToolsFacade::GetAllFontFaces(source);
    const auto found = std::find_if(faces.begin(), faces.end(), [this](const auto& face) {
        return face.Index == impl_->face_index;
    });
    if (found != faces.end())
        return convert_face_info(*found);

    output.index = impl_->face_index;
    output.face_count = 1;
    output.family_name = std::string(metrics->GetFontFamilyName());
    output.postscript_name = std::string(metrics->GetFontName());
    output.format = convert_format(metrics->GetFontFileType());
    output.glyph_count = metrics->GetGlyphCountFontProgram();
    output.units_per_em = metrics->GetUnitsPerEm();
    return output;
}

Status FontTool::glyph_id(char32_t code_point, std::uint32_t& gid) const noexcept {
    gid = 0;
    if (!is_loaded())
        return failure(ErrorCode::NotLoaded, "no font face is loaded");
    if (code_point > 0x10FFFFu ||
        (code_point >= 0xD800u && code_point <= 0xDFFFu))
        return failure(ErrorCode::InvalidArgument, "invalid Unicode code point");

    unsigned internal_gid = 0;
    if (!impl_->facade.GetMetrics()->TryGetGID(code_point, internal_gid))
        return failure(ErrorCode::MissingGlyph,
                       "font does not contain the requested Unicode code point");
    gid = internal_gid;
    return success();
}

Status FontTool::glyph_advance(std::uint32_t gid, double& advance_em) const noexcept {
    advance_em = 0.0;
    if (!is_loaded())
        return failure(ErrorCode::NotLoaded, "no font face is loaded");

    if (!impl_->facade.GetMetrics()->TryGetGlyphWidthFontProgram(gid, advance_em))
        return failure(ErrorCode::InvalidArgument,
                       "glyph ID is outside the loaded font or has no metrics");
    return success();
}

Status FontTool::subset_utf8(std::string_view text,
                             const SubsetOptions& options,
                             SubsetResult& result) const noexcept {
    result.clear();
    if (!is_loaded())
        return failure(ErrorCode::NotLoaded, "no font face is loaded");
    if (text.empty())
        return failure(ErrorCode::InvalidArgument, "subset text is empty");

    PoDoFo::FontSubsetPackage package;
    std::string error;
    const bool ok = impl_->facade.SubsetUtf8(text, convert_options(options), package, &error);
    return finish_subset(ok, package, std::move(error), result);
}

Status FontTool::subset_codepoints(const std::vector<char32_t>& code_points,
                                   const SubsetOptions& options,
                                   SubsetResult& result) const noexcept {
    result.clear();
    if (!is_loaded())
        return failure(ErrorCode::NotLoaded, "no font face is loaded");
    if (code_points.empty())
        return failure(ErrorCode::InvalidArgument, "Unicode subset input is empty");

    PoDoFo::FontSubsetPackage package;
    std::string error;
    const auto view = PoDoFo::cspan<char32_t>(code_points.data(), code_points.size());
    const bool ok = impl_->facade.SubsetUnicode(view, convert_options(options), package, &error);
    return finish_subset(ok, package, std::move(error), result);
}

Status FontTool::subset_glyphs(const std::vector<std::uint32_t>& gids,
                               const SubsetOptions& options,
                               SubsetResult& result) const noexcept {
    result.clear();
    if (!is_loaded())
        return failure(ErrorCode::NotLoaded, "no font face is loaded");
    if (gids.empty())
        return failure(ErrorCode::InvalidArgument, "glyph subset input is empty");

    std::vector<std::uint16_t> internal_gids;
    internal_gids.reserve(gids.size());
    for (const std::uint32_t gid : gids) {
        if (gid > std::numeric_limits<std::uint16_t>::max())
            return failure(ErrorCode::InvalidArgument, "glyph ID exceeds 65535");
        internal_gids.push_back(static_cast<std::uint16_t>(gid));
    }

    PoDoFo::FontSubsetPackage package;
    std::string error;
    const auto view = PoDoFo::cspan<std::uint16_t>(internal_gids.data(),
                                                   internal_gids.size());
    const bool ok = impl_->facade.SubsetGids(view, convert_options(options), package, &error);
    return finish_subset(ok, package, std::move(error), result);
}

Status FontTool::subset_cids(const std::vector<CidGlyphMapping>& mappings,
                             const SubsetOptions& options,
                             SubsetResult& result,
                             const CidSystemInfo& cid_info) const noexcept {
    result.clear();
    if (!is_loaded())
        return failure(ErrorCode::NotLoaded, "no font face is loaded");
    if (mappings.empty())
        return failure(ErrorCode::InvalidArgument, "CID subset input is empty");

    std::vector<PoDoFo::CIDGlyphMapping> internal_mappings;
    internal_mappings.reserve(mappings.size());
    for (const auto& mapping : mappings) {
        if (mapping.cid == 0)
            return failure(ErrorCode::InvalidArgument, "CID 0 is reserved for .notdef");
        PoDoFo::CIDGlyphMapping item;
        item.Cid = mapping.cid;
        item.SourceGid = mapping.source_gid;
        item.Unicode = mapping.unicode;
        internal_mappings.push_back(std::move(item));
    }

    PoDoFo::CIDSystemInfo internal_cid_info;
    internal_cid_info.Registry = cid_info.registry;
    internal_cid_info.Ordering = cid_info.ordering;
    internal_cid_info.Supplement = cid_info.supplement;

    PoDoFo::FontSubsetPackage package;
    std::string error;
    const auto view = PoDoFo::cspan<PoDoFo::CIDGlyphMapping>(internal_mappings.data(),
                                                             internal_mappings.size());
    const bool ok = impl_->facade.SubsetCids(view,
                                             convert_options(options),
                                             package,
                                             internal_cid_info,
                                             &error);
    return finish_subset(ok, package, std::move(error), result);
}

Status FontTool::inspect_file(std::string_view path,
                              std::vector<FaceInfo>& faces) noexcept {
    std::vector<std::uint8_t> data;
    const Status status = read_binary_file(path, data);
    if (!status) {
        faces.clear();
        return status;
    }
    return inspect_memory(data.data(), data.size(), faces);
}

Status FontTool::inspect_memory(const std::uint8_t* data,
                                std::size_t size,
                                std::vector<FaceInfo>& faces) noexcept {
    faces.clear();
    if (data == nullptr || size == 0)
        return failure(ErrorCode::InvalidArgument, "font memory buffer is empty");

    const auto view = PoDoFo::bufferview(reinterpret_cast<const char*>(data), size);
    const auto internal_faces = PoDoFo::FontToolsFacade::GetAllFontFaces(view);
    if (internal_faces.empty())
        return failure(ErrorCode::InvalidFont, "buffer is not a readable font");

    faces.reserve(internal_faces.size());
    for (const auto& face : internal_faces)
        faces.push_back(convert_face_info(face));
    return success();
}

Status FontTool::find_face_index(std::string_view path,
                                 std::string_view face_name,
                                 std::uint32_t& face_index) noexcept {
    face_index = 0;
    if (face_name.empty())
        return failure(ErrorCode::InvalidArgument, "font face name is empty");

    std::vector<std::uint8_t> data;
    const Status status = read_binary_file(path, data);
    if (!status)
        return status;

    const auto view = PoDoFo::bufferview(reinterpret_cast<const char*>(data.data()),
                                         data.size());
    const auto index = PoDoFo::FontToolsFacade::GetFontIndexByName(view, face_name);
    if (!index)
        return failure(ErrorCode::FaceNotFound, "font face name was not found");
    face_index = *index;
    return success();
}

Status FontTool::write_file(std::string_view path,
                            const SubsetResult& result) noexcept {
    if (path.empty())
        return failure(ErrorCode::InvalidArgument, "output path is empty");
    if (result.font_program.empty())
        return failure(ErrorCode::InvalidArgument, "font program is empty");

    const std::string path_string(path);
    std::ofstream output(path_string, std::ios::binary | std::ios::trunc);
    if (!output)
        return failure(ErrorCode::WriteFailed, "unable to create output file: " + path_string);

    output.write(reinterpret_cast<const char*>(result.font_program.data()),
                 static_cast<std::streamsize>(result.font_program.size()));
    if (!output)
        return failure(ErrorCode::WriteFailed, "unable to write output file: " + path_string);
    return success();
}

const char* to_string(ErrorCode value) noexcept {
    switch (value) {
    case ErrorCode::Ok: return "ok";
    case ErrorCode::InvalidArgument: return "invalid_argument";
    case ErrorCode::IoError: return "io_error";
    case ErrorCode::InvalidFont: return "invalid_font";
    case ErrorCode::UnsupportedFont: return "unsupported_font";
    case ErrorCode::FaceNotFound: return "face_not_found";
    case ErrorCode::NotLoaded: return "not_loaded";
    case ErrorCode::InvalidUtf8: return "invalid_utf8";
    case ErrorCode::MissingGlyph: return "missing_glyph";
    case ErrorCode::SubsetFailed: return "subset_failed";
    case ErrorCode::WriteFailed: return "write_failed";
    default: return "unknown_error";
    }
}

const char* to_string(FontFormat value) noexcept {
    switch (value) {
    case FontFormat::Type1: return "Type 1";
    case FontFormat::TrueType: return "TrueType";
    case FontFormat::OpenTypeCff: return "OpenType/CFF";
    case FontFormat::Type1Cff: return "Type 1 CFF";
    case FontFormat::CidKeyedCff: return "CID-keyed CFF";
    case FontFormat::Cff2: return "CFF2";
    case FontFormat::Unknown:
    default: return "Unknown";
    }
}

const char* version_string() noexcept {
    return FONTTOOL_VERSION_STRING;
}

} // namespace fonttool

// SPDX-License-Identifier: MPL-2.0
#include <fonttool/fonttool.hpp>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace {

bool file_exists(const std::string& path) {
    std::ifstream input(path, std::ios::binary);
    return static_cast<bool>(input);
}

std::string find_test_font(int argc, char** argv) {
    if (argc > 1 && argv[1] != nullptr && argv[1][0] != '\0' && file_exists(argv[1]))
        return argv[1];
    if (const char* env = std::getenv("FONTTOOL_TEST_FONT")) {
        if (file_exists(env))
            return env;
    }

    const std::vector<std::string> candidates = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/fonts-go/Go-Regular.ttf",
        "/usr/share/fonts/truetype/liberation2/LiberationSans-Regular.ttf",
        "C:/Windows/Fonts/arial.ttf",
    };
    for (const auto& candidate : candidates) {
        if (file_exists(candidate))
            return candidate;
    }
    return {};
}


std::string find_cff_test_font() {
    const std::vector<std::string> candidates = {
        "/usr/share/fonts/opentype/cabin/Cabin-Regular.otf",
        "/usr/share/fonts/opentype/freefont/FreeSans.otf",
        "/usr/share/fonts/X11/Type1/NimbusSans-Regular.pfb",
    };
    for (const auto& candidate : candidates) {
        if (file_exists(candidate))
            return candidate;
    }
    return {};
}

bool read_file(const std::string& path, std::vector<std::uint8_t>& output) {
    std::ifstream input(path, std::ios::binary | std::ios::ate);
    if (!input)
        return false;
    const auto size = input.tellg();
    if (size <= 0)
        return false;
    output.resize(static_cast<std::size_t>(size));
    input.seekg(0, std::ios::beg);
    input.read(reinterpret_cast<char*>(output.data()), size);
    return static_cast<bool>(input);
}

int require(bool condition, const char* message) {
    if (condition)
        return 0;
    std::cerr << "FAIL: " << message << '\n';
    return 1;
}

} // namespace

int main(int argc, char** argv) {
    int failures = 0;

    fonttool::FontTool tool;
    failures += require(!tool.is_loaded(), "new FontTool must be unloaded");

    std::uint32_t gid = 123;
    const auto not_loaded = tool.glyph_id(U'A', gid);
    failures += require(!not_loaded && not_loaded.code == fonttool::ErrorCode::NotLoaded,
                        "glyph lookup must report NotLoaded");
    failures += require(gid == 0, "failed glyph lookup must clear output GID");

    const std::uint8_t invalid_data[] = {0x00, 0x01, 0x02, 0x03};
    const auto invalid = tool.load_memory(invalid_data, sizeof(invalid_data));
    failures += require(!invalid, "invalid font data must be rejected");

    if (failures != 0)
        return 1;

    const std::string font_path = find_test_font(argc, argv);
    if (font_path.empty()) {
        std::cout << "SKIP: no test font found\n";
        return 77;
    }

    std::vector<fonttool::FaceInfo> faces;
    auto status = fonttool::FontTool::inspect_file(font_path, faces);
    failures += require(status.ok(), "font inspection must succeed");
    failures += require(!faces.empty(), "font inspection must return at least one face");

    std::vector<std::uint8_t> source_bytes;
    failures += require(read_file(font_path, source_bytes), "test font bytes must be readable");
    std::vector<fonttool::FaceInfo> memory_faces;
    status = fonttool::FontTool::inspect_memory(source_bytes.data(), source_bytes.size(), memory_faces);
    failures += require(status.ok() && memory_faces.size() == faces.size(),
                        "memory inspection must match file inspection");

    status = tool.load_file(font_path);
    failures += require(status.ok(), "font loading must succeed");
    failures += require(tool.is_loaded(), "FontTool must report loaded state");

    const auto info = tool.info();
    failures += require(info.glyph_count > 0, "loaded font must have glyphs");
    failures += require(info.units_per_em > 0, "loaded font must have units per em");

    status = tool.glyph_id(U'A', gid);
    failures += require(status.ok() && gid != 0, "font must map U+0041 to a glyph");

    double advance = 0.0;
    status = tool.glyph_advance(gid, advance);
    failures += require(status.ok() && advance > 0.0, "glyph advance must be positive");

    fonttool::SubsetOptions options;
    options.include_hinting = true;
    fonttool::SubsetResult subset;
    const std::string invalid_utf8("\xF0\x28\x8C\x28", 4);
    status = tool.subset_utf8(invalid_utf8, options, subset);
    failures += require(!status && status.code == fonttool::ErrorCode::InvalidUtf8,
                        "invalid UTF-8 must be rejected");

    fonttool::SubsetOptions invalid_options;
    invalid_options.retain_glyph_count = true;
    status = tool.subset_utf8("Hello", invalid_options, subset);
    failures += require(!status && status.code == fonttool::ErrorCode::SubsetFailed,
                        "retain_glyph_count without retain_gids must fail");

    status = tool.subset_utf8("Hello FontTool!", options, subset);
    failures += require(status.ok(), "UTF-8 subsetting must succeed");
    failures += require(!subset.font_program.empty(), "subset font must contain bytes");
    failures += require(!subset.glyph_mappings.empty(), "subset must expose glyph mappings");

    fonttool::FontTool subset_tool;
    status = subset_tool.load_memory(subset.font_program);
    failures += require(status.ok(), "generated subset must be readable by FreeType");
    failures += require(subset_tool.info().glyph_count > 0,
                        "generated subset must contain glyphs");

    const std::string cff_path = find_cff_test_font();
    if (!cff_path.empty()) {
        fonttool::FontTool cff_tool;
        status = cff_tool.load_file(cff_path);
        failures += require(status.ok(), "CFF/Type1 test font must load");
        if (status) {
            fonttool::SubsetResult cff_subset;
            status = cff_tool.subset_utf8("Hello FontTool!", {}, cff_subset);
            failures += require(status.ok(), "CFF/Type1 subsetting must succeed");
            failures += require(cff_subset.format == fonttool::FontFormat::CidKeyedCff,
                                "CFF/Type1 subset output must be CID-keyed CFF");
            fonttool::FontTool reloaded_cff;
            status = reloaded_cff.load_memory(cff_subset.font_program);
            failures += require(status.ok(), "generated CFF subset must be readable");
        }
    }

    if (failures == 0)
        std::cout << "PASS: FontTool API and subsetting validated with " << font_path << '\n';
    return failures == 0 ? 0 : 1;
}

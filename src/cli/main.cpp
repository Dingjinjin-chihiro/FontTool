// SPDX-License-Identifier: MPL-2.0
#include <fonttool/fonttool.hpp>

#include <charconv>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace {

using fonttool::FontTool;
using fonttool::Status;

void print_usage(std::ostream& out) {
    out <<
        "FontTool " << fonttool::version_string() << "\n\n"
        "Usage:\n"
        "  fonttool info <font> [--face <index>] [--json]\n"
        "  fonttool faces <font> [--json]\n"
        "  fonttool subset <font> -o <output> (--text <utf8> | --text-file <file> | --gids <list>)\n"
        "                  [--face <index>] [--retain-gids] [--retain-glyph-count]\n"
        "                  [--include-cmap] [--no-hinting]\n"
        "  fonttool --version\n\n"
        "Examples:\n"
        "  fonttool info NotoSansCJK-Regular.ttc --face 2\n"
        "  fonttool faces NotoSansCJK-Regular.ttc --json\n"
        "  fonttool subset input.ttf -o subset.ttf --text \"Hello 世界\"\n"
        "  fonttool subset input.ttf -o subset.ttf --gids 1,2,36,0x41 --retain-gids\n";
}

int fail(const Status& status) {
    std::cerr << "fonttool: " << fonttool::to_string(status.code);
    if (!status.message.empty())
        std::cerr << ": " << status.message;
    std::cerr << '\n';
    return 1;
}

bool parse_u32(std::string_view text, std::uint32_t& value) {
    value = 0;
    int base = 10;
    if (text.size() > 2 && text[0] == '0' && (text[1] == 'x' || text[1] == 'X')) {
        text.remove_prefix(2);
        base = 16;
    }
    if (text.empty())
        return false;
    const auto result = std::from_chars(text.data(), text.data() + text.size(), value, base);
    return result.ec == std::errc{} && result.ptr == text.data() + text.size();
}

bool read_text_file(std::string_view path, std::string& output) {
    output.clear();
    std::ifstream input(std::string(path), std::ios::binary | std::ios::ate);
    if (!input)
        return false;
    const std::streamoff size = input.tellg();
    if (size < 0)
        return false;
    output.resize(static_cast<std::size_t>(size));
    input.seekg(0, std::ios::beg);
    input.read(output.data(), size);
    return static_cast<bool>(input) || size == 0;
}

std::string json_escape(std::string_view value) {
    std::string output;
    output.reserve(value.size() + 8);
    for (const unsigned char ch : value) {
        switch (ch) {
        case '"': output += "\\\""; break;
        case '\\': output += "\\\\"; break;
        case '\b': output += "\\b"; break;
        case '\f': output += "\\f"; break;
        case '\n': output += "\\n"; break;
        case '\r': output += "\\r"; break;
        case '\t': output += "\\t"; break;
        default:
            if (ch < 0x20) {
                static constexpr char hex[] = "0123456789abcdef";
                output += "\\u00";
                output += hex[(ch >> 4) & 0x0F];
                output += hex[ch & 0x0F];
            } else {
                output.push_back(static_cast<char>(ch));
            }
            break;
        }
    }
    return output;
}

void print_face_text(const fonttool::FaceInfo& face) {
    std::cout << "Face index:          " << face.index << '\n'
              << "Face count:          " << face.face_count << '\n'
              << "Family:              " << face.family_name << '\n'
              << "Style:               " << face.style_name << '\n'
              << "PostScript name:     " << face.postscript_name << '\n'
              << "Format:              " << fonttool::to_string(face.format) << '\n'
              << "Glyph count:         " << face.glyph_count << '\n'
              << "Units per em:        " << face.units_per_em << '\n'
              << "Scalable:            " << (face.scalable ? "yes" : "no") << '\n'
              << "Unicode charmap:     " << (face.has_unicode_charmap ? "yes" : "no") << '\n'
              << "CID-keyed:           " << (face.cid_keyed ? "yes" : "no") << '\n';
}

void print_face_json(const fonttool::FaceInfo& face) {
    std::cout << "{"
              << "\"index\":" << face.index << ','
              << "\"face_count\":" << face.face_count << ','
              << "\"family_name\":\"" << json_escape(face.family_name) << "\","
              << "\"style_name\":\"" << json_escape(face.style_name) << "\","
              << "\"postscript_name\":\"" << json_escape(face.postscript_name) << "\","
              << "\"format\":\"" << json_escape(fonttool::to_string(face.format)) << "\","
              << "\"glyph_count\":" << face.glyph_count << ','
              << "\"units_per_em\":" << face.units_per_em << ','
              << "\"scalable\":" << (face.scalable ? "true" : "false") << ','
              << "\"has_unicode_charmap\":" << (face.has_unicode_charmap ? "true" : "false") << ','
              << "\"cid_keyed\":" << (face.cid_keyed ? "true" : "false")
              << "}";
}

int command_info(int argc, char** argv) {
    if (argc < 3) {
        print_usage(std::cerr);
        return 2;
    }

    const std::string_view path = argv[2];
    std::uint32_t face_index = 0;
    bool json = false;
    for (int i = 3; i < argc; ++i) {
        const std::string_view arg = argv[i];
        if (arg == "--json") {
            json = true;
        } else if (arg == "--face" && i + 1 < argc) {
            if (!parse_u32(argv[++i], face_index)) {
                std::cerr << "fonttool: invalid face index\n";
                return 2;
            }
        } else {
            std::cerr << "fonttool: unknown option: " << arg << '\n';
            return 2;
        }
    }

    FontTool tool;
    const Status status = tool.load_file(path, face_index);
    if (!status)
        return fail(status);
    if (json) {
        print_face_json(tool.info());
        std::cout << '\n';
    } else {
        print_face_text(tool.info());
    }
    return 0;
}

int command_faces(int argc, char** argv) {
    if (argc < 3) {
        print_usage(std::cerr);
        return 2;
    }
    const std::string_view path = argv[2];
    bool json = false;
    for (int i = 3; i < argc; ++i) {
        if (std::string_view(argv[i]) == "--json")
            json = true;
        else {
            std::cerr << "fonttool: unknown option: " << argv[i] << '\n';
            return 2;
        }
    }

    std::vector<fonttool::FaceInfo> faces;
    const Status status = FontTool::inspect_file(path, faces);
    if (!status)
        return fail(status);

    if (json) {
        std::cout << '[';
        for (std::size_t i = 0; i < faces.size(); ++i) {
            if (i != 0)
                std::cout << ',';
            print_face_json(faces[i]);
        }
        std::cout << "]\n";
    } else {
        for (std::size_t i = 0; i < faces.size(); ++i) {
            if (i != 0)
                std::cout << "\n";
            print_face_text(faces[i]);
        }
    }
    return 0;
}

bool parse_gid_list(std::string_view text, std::vector<std::uint32_t>& gids) {
    gids.clear();
    while (!text.empty()) {
        const std::size_t delimiter = text.find(',');
        std::string_view token = text.substr(0, delimiter);
        while (!token.empty() && token.front() == ' ')
            token.remove_prefix(1);
        while (!token.empty() && token.back() == ' ')
            token.remove_suffix(1);
        std::uint32_t gid = 0;
        if (!parse_u32(token, gid))
            return false;
        gids.push_back(gid);
        if (delimiter == std::string_view::npos)
            break;
        text.remove_prefix(delimiter + 1);
    }
    return !gids.empty();
}

int command_subset(int argc, char** argv) {
    if (argc < 3) {
        print_usage(std::cerr);
        return 2;
    }

    const std::string_view input_path = argv[2];
    std::string output_path;
    std::string text;
    std::vector<std::uint32_t> gids;
    std::uint32_t face_index = 0;
    fonttool::SubsetOptions options;
    enum class InputKind { None, Text, Gids } input_kind = InputKind::None;

    for (int i = 3; i < argc; ++i) {
        const std::string_view arg = argv[i];
        if ((arg == "-o" || arg == "--output") && i + 1 < argc) {
            output_path = argv[++i];
        } else if (arg == "--text" && i + 1 < argc) {
            if (input_kind != InputKind::None) {
                std::cerr << "fonttool: choose exactly one subset input\n";
                return 2;
            }
            text = argv[++i];
            input_kind = InputKind::Text;
        } else if (arg == "--text-file" && i + 1 < argc) {
            if (input_kind != InputKind::None) {
                std::cerr << "fonttool: choose exactly one subset input\n";
                return 2;
            }
            const std::string_view text_path = argv[++i];
            if (!read_text_file(text_path, text)) {
                std::cerr << "fonttool: unable to read text file: " << text_path << '\n';
                return 1;
            }
            input_kind = InputKind::Text;
        } else if (arg == "--gids" && i + 1 < argc) {
            if (input_kind != InputKind::None) {
                std::cerr << "fonttool: choose exactly one subset input\n";
                return 2;
            }
            if (!parse_gid_list(argv[++i], gids)) {
                std::cerr << "fonttool: invalid GID list\n";
                return 2;
            }
            input_kind = InputKind::Gids;
        } else if (arg == "--face" && i + 1 < argc) {
            if (!parse_u32(argv[++i], face_index)) {
                std::cerr << "fonttool: invalid face index\n";
                return 2;
            }
        } else if (arg == "--retain-gids") {
            options.retain_gids = true;
        } else if (arg == "--retain-glyph-count") {
            options.retain_gids = true;
            options.retain_glyph_count = true;
        } else if (arg == "--include-cmap") {
            options.include_cmap = true;
        } else if (arg == "--no-hinting") {
            options.include_hinting = false;
        } else {
            std::cerr << "fonttool: unknown or incomplete option: " << arg << '\n';
            return 2;
        }
    }

    if (output_path.empty() || input_kind == InputKind::None) {
        std::cerr << "fonttool: output path and one subset input are required\n";
        return 2;
    }

    FontTool tool;
    Status status = tool.load_file(input_path, face_index);
    if (!status)
        return fail(status);

    fonttool::SubsetResult result;
    if (input_kind == InputKind::Text)
        status = tool.subset_utf8(text, options, result);
    else
        status = tool.subset_glyphs(gids, options, result);
    if (!status)
        return fail(status);

    status = FontTool::write_file(output_path, result);
    if (!status)
        return fail(status);

    std::cout << "Wrote " << result.font_program.size() << " bytes to " << output_path << '\n'
              << "Format: " << fonttool::to_string(result.format) << '\n'
              << "Retains GIDs: " << (result.retains_original_gids ? "yes" : "no") << '\n'
              << "Identity CIDToGIDMap: " << (result.identity_cid_to_gid ? "yes" : "no") << '\n';
    return 0;
}

} // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        print_usage(std::cerr);
        return 2;
    }

    const std::string_view command = argv[1];
    if (command == "--version" || command == "version") {
        std::cout << fonttool::version_string() << '\n';
        return 0;
    }
    if (command == "--help" || command == "-h" || command == "help") {
        print_usage(std::cout);
        return 0;
    }
    if (command == "info")
        return command_info(argc, argv);
    if (command == "faces")
        return command_faces(argc, argv);
    if (command == "subset")
        return command_subset(argc, argv);

    std::cerr << "fonttool: unknown command: " << command << "\n\n";
    print_usage(std::cerr);
    return 2;
}

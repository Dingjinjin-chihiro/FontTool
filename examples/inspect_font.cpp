// SPDX-License-Identifier: MPL-2.0
#include <fonttool/fonttool.hpp>

#include <iostream>
#include <vector>

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: fonttool_inspect_example <font-or-collection>\n";
        return 2;
    }

    std::vector<fonttool::FaceInfo> faces;
    const auto status = fonttool::FontTool::inspect_file(argv[1], faces);
    if (!status) {
        std::cerr << fonttool::to_string(status.code) << ": " << status.message << '\n';
        return 1;
    }

    for (const auto& face : faces) {
        std::cout << '[' << face.index << "] " << face.family_name << ' '
                  << face.style_name << " | " << fonttool::to_string(face.format)
                  << " | glyphs=" << face.glyph_count << '\n';
    }
    return 0;
}

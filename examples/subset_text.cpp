// SPDX-License-Identifier: MPL-2.0
#include <fonttool/fonttool.hpp>

#include <iostream>

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: fonttool_subset_example <input-font> <output-font>\n";
        return 2;
    }

    fonttool::FontTool tool;
    fonttool::Status status = tool.load_file(argv[1]);
    if (!status) {
        std::cerr << status.message << '\n';
        return 1;
    }

    fonttool::SubsetOptions options;
    options.retain_gids = false;
    options.include_hinting = true;

    fonttool::SubsetResult subset;
    status = tool.subset_utf8("Hello, FontTool!", options, subset);
    if (!status) {
        std::cerr << status.message << '\n';
        return 1;
    }

    status = fonttool::FontTool::write_file(argv[2], subset);
    if (!status) {
        std::cerr << status.message << '\n';
        return 1;
    }

    std::cout << "Created " << subset.font_program.size() << "-byte subset font\n";
    return 0;
}

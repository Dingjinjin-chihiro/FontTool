# SPDX-License-Identifier: MPL-2.0
add_executable(fonttool_subset_example subset_text.cpp)
target_link_libraries(fonttool_subset_example PRIVATE FontTool::FontTool)
target_compile_features(fonttool_subset_example PRIVATE cxx_std_17)

add_executable(fonttool_inspect_example inspect_font.cpp)
target_link_libraries(fonttool_inspect_example PRIVATE FontTool::FontTool)
target_compile_features(fonttool_inspect_example PRIVATE cxx_std_17)

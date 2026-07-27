# SPDX-License-Identifier: MPL-2.0
add_executable(fonttool_tests test_fonttool.cpp)
target_link_libraries(fonttool_tests PRIVATE FontTool::FontTool)
target_compile_features(fonttool_tests PRIVATE cxx_std_17)

add_test(NAME fonttool.api COMMAND fonttool_tests "${FONTTOOL_TEST_FONT}")
set_tests_properties(fonttool.api PROPERTIES SKIP_RETURN_CODE 77)

if (NOT CMAKE_SCRIPT_MODE_FILE)
    message(FATAL_ERROR "This script is only meant to be used as a CMake script.")
endif()

message(STATUS "Embedding binary file: ${source_file} into ${output_header}")
get_filename_component(output_name "${source_file}" NAME_WLE)
string(REGEX REPLACE "\\." "_" output_name "${output_name}")

file(READ "${source_file}" original_content HEX)
file(WRITE "${output_header}" "#pragma once\n#include <array>\n#include <cstdint>\n")
file(APPEND "${output_header}" "constexpr auto ${output_name} = std::to_array<std::uint8_t>({\n")

string(REGEX REPLACE "\([0-9a-f][0-9a-f]\)" ",0x\\1" escaped_content "${original_content}")
string(REGEX REPLACE "\(..........................................................................,\)" "\\1\n" escaped_content "${escaped_content}")
string(REGEX REPLACE "^," "" escaped_content "${escaped_content}")
file(APPEND "${output_header}" "${escaped_content}\n")

file(APPEND "${output_header}" "});\n")
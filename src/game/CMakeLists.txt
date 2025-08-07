add_executable(SoldatGame main.cpp)
target_include_directories(SoldatGame
    PRIVATE
        ${ROOT_DIR}/src
)

target_include_directories(SoldatGame SYSTEM PRIVATE
    ${TRACY_INCLUDE_DIR}
    ${DOCTEST_INCLUDE_DIR}
    ${STB_INCLUDE_DIR}
)

target_link_libraries(SoldatGame
    SoldatClientLib
    SoldatServerLib
    CommonLib
    PlatformLib
    SDL3::SDL3
    spdlog::spdlog
    Freetype::Freetype
    ${PHYSFS_LIBRARY}
    GameNetworkingSockets::GameNetworkingSockets_s
    ApprovalTests::ApprovalTests
)

set_target_properties(SoldatGame PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/game)
doctest_discover_tests(SoldatGame)

set(Shader_SRC
    ../shaders/nuklear_ui.frag
    ../shaders/nuklear_ui.vert
    ../shaders/generic.frag.hlsl
    ../shaders/generic.vert.hlsl
)


set(shadercross ${SDL3_shadercross_DIR}/../../../bin/shadercross)
get_filename_component(shadercross ${shadercross} ABSOLUTE)
set(shadercross_libs ${SDL3_shadercross_DIR}/../../;${SDL3_DIR}/../../)


foreach(Shader ${Shader_SRC})
  get_filename_component(Shader_PATH "${Shader}" ABSOLUTE)
  get_filename_component(FILE_NAME ${Shader} NAME)
  set(SPIRV "${PROJECT_BINARY_DIR}/game/shaders/${FILE_NAME}.spv")
  set(SPIRV_DEBUG "${PROJECT_BINARY_DIR}/game/shaders/${FILE_NAME}.debug.spv")
  set(SPIRV_HPP "${PROJECT_BINARY_DIR}/generated/${FILE_NAME}.hpp")
  set(SPIRV_DEBUG_HPP "${PROJECT_BINARY_DIR}/generated/${FILE_NAME}.debug.hpp")
  add_custom_command(
    OUTPUT ${SPIRV} ${SPIRV_DEBUG} ${SPIRV_HPP} ${SPIRV_DEBUG_HPP}
    COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/game/shaders/"
    COMMAND
        ${CMAKE_COMMAND} -E env "\"LD_LIBRARY_PATH=${shadercross_libs}:$ENV{LD_LIBRARY_PATH}\""
        ${shadercross} ${Shader_PATH} -s HLSL -d SPIRV -o ${SPIRV}
    COMMAND
        ${CMAKE_COMMAND} -E env "\"LD_LIBRARY_PATH=${shadercross_libs}:$ENV{LD_LIBRARY_PATH}\""
        ${shadercross} ${Shader_PATH} -s HLSL -d SPIRV -o ${SPIRV_DEBUG} --debug
    COMMAND
        ${CMAKE_COMMAND}
        -Dsource_file=${SPIRV}
        -Doutput_header=${SPIRV_HPP}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/embed_binary.cmake
    COMMAND
        ${CMAKE_COMMAND}
        -Dsource_file=${SPIRV_DEBUG}
        -Doutput_header=${SPIRV_DEBUG_HPP}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/embed_binary.cmake
    DEPENDS ${Shader})
  list(APPEND SPIRV_BINARY_FILES ${SPIRV} ${SPIRV_DEBUG} ${SPIRV_HPP} ${SPIRV_DEBUG_HPP})
endforeach(Shader)


add_custom_target(
    Shaders 
    DEPENDS ${SPIRV_BINARY_FILES}
)

add_dependencies(SoldatGame Shaders)

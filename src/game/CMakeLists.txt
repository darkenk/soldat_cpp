add_executable(SoldatGame main.cpp)
target_include_directories(SoldatGame PRIVATE
    ${ROOT_DIR}/src
)

target_include_directories(SoldatGame SYSTEM PRIVATE
    ${TRACY_INCLUDE_DIR}
    ${DOCTEST_INCLUDE_DIR}
)

target_link_libraries(SoldatGame
    SoldatClientLib
    SoldatServerLib
    CommonLib
    PlatformLib
    SDL3::SDL3
    spdlog::spdlog
    Freetype::Freetype
    ${PHYSFS_LIB}
    GameNetworkingSockets::GameNetworkingSockets_s
    ${SOLOUD_LIBRARY}
)

set_target_properties(SoldatGame PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/game)
doctest_discover_tests(SoldatGame)

set(Shader_SRC
    ../shaders/nuklear_ui.frag
    ../shaders/nuklear_ui.vert
)

foreach(Shader ${Shader_SRC})
  get_filename_component(Shader_PATH "${Shader}" ABSOLUTE)
  get_filename_component(FILE_NAME ${Shader} NAME)
  set(SPIRV "${PROJECT_BINARY_DIR}/game/shaders/${FILE_NAME}.spv")
  set(SPIRV_DEBUG "${PROJECT_BINARY_DIR}/game/shaders/${FILE_NAME}.debug.spv")
  add_custom_command(
    OUTPUT ${SPIRV}
    COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/game/shaders/"
    COMMAND shadercross ${Shader_PATH} -s HLSL -d SPIRV -o ${SPIRV}
    COMMAND shadercross ${Shader_PATH} -s HLSL -d SPIRV -o ${SPIRV_DEBUG} --debug
    DEPENDS ${Shader})
  list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(Shader)

add_custom_target(
    Shaders 
    DEPENDS ${SPIRV_BINARY_FILES}
)

add_dependencies(SoldatGame Shaders)

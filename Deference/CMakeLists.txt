# CMakeList.txt : CMake project for Deference, include source and define
# project specific logic here.
#

set(DXTEX_DIR ../tools/DirectXTex)

# Add source to this project's executable.
file(GLOB_RECURSE SRC CONFIGURE_DEPENDS "src/*.h" "src/*.cpp")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Debug
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Graphics
    ${CMAKE_CURRENT_SOURCE_DIR}/src/DXR
    ${DXTEX_DIR}/DirectXTex
)

# Add source to this project's executable.
add_executable (Deference WIN32 ${SRC})
add_custom_target(copy_resources ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/Deference/res
        ${PROJECT_BINARY_DIR}/Deference
        COMMENT "Copying resources into binary directory")

add_dependencies(Deference copy_resources)
target_precompile_headers(Deference PRIVATE src/pch.h)
target_link_libraries(Deference "d3d12.lib" "d3dcompiler.lib" "dxgi.lib" "dxguid.lib" "dxcompiler.lib" DirectXTex)  

set_property(TARGET Deference PROPERTY WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/Deference)
get_target_property(test_dir Deference WORKING_DIRECTORY)
message("My test's working directory: ${test_dir}")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Deference PROPERTY CXX_STANDARD 20)
  set_property(
    TARGET Deference
    PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()

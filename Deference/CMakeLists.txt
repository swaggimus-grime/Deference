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
    ${DXTEX_DIR}/DirectXTex
)

# Add source to this project's executable.
add_executable (Deference WIN32 ${SRC})
target_precompile_headers(Deference PRIVATE src/pch.h)
target_link_libraries(Deference "d3d12.lib" "d3dcompiler.lib" "dxgi.lib" DirectXTex)  
set_target_properties(
    Deference PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Deference PROPERTY CXX_STANDARD 20)
endif()

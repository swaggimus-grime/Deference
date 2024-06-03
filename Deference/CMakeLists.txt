# CMakeList.txt : CMake project for Deference, include source and define
# project specific logic here.
#

# Add source to this project's executable.
file(GLOB_RECURSE SRC CONFIGURE_DEPENDS 
    "src/*.cpp" 
    "${TINYGLTF_DIR}/tiny_gltf.cc"
    "${IMGUI_DIR}/imgui.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_dx12.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_win32.cpp"
    "${IMGUI_DIR}/imgui_draw.cpp"
    "${IMGUI_DIR}/imgui_demo.cpp"
    "${IMGUI_DIR}/imgui_tables.cpp"
    "${IMGUI_DIR}/imgui_widgets.cpp"
    "${DXTEX_DIR}/DirectXTex/*.cpp"
    "${DXTK12_DIR}/Src/DDSTextureLoader.cpp"
    "${DXTK12_DIR}/Src/ResourceUploadBatch.cpp"
)
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "src/*.h")

include_directories(
    ${DXTEX_DIR}
    ${DXTEX_DIR}/DirectXTex/Shaders/Compiled
    ${DXTK12_DIR}/Inc
    ${TINYGLTF_DIR}
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Debug
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Graphics
)

# Add source to this project's executable.
add_executable (Deference WIN32 ${SRC})
#add_custom_target(copy_resources ALL
#        COMMAND ${CMAKE_COMMAND} -E copy_directory
#        ${PROJECT_SOURCE_DIR}/Deference/res
#        ${PROJECT_BINARY_DIR}/Deference
#        COMMENT "Copying resources into binary directory")

#add_dependencies(Deference copy_resources)

if (CMAKE_VERSION VERSION_GREATER 3.16)
  set(CMAKE_PCH_INSTANTIATE_TEMPLATES ON)
  target_precompile_headers(Deference PUBLIC ${HEADERS})
endif()

set_property(TARGET Deference PROPERTY CXX_STANDARD 23)
target_link_libraries(Deference PRIVATE "d3d12.lib" "d3dcompiler.lib" "dxgi.lib" "dxguid.lib" "dxcompiler.lib" DirectXTex DirectXTK12)  

set_target_properties(Deference PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/Deference")
if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Deference PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/Deference")
  get_target_property(wk_dir Deference VS_DEBUGGER_WORKING_DIRECTORY)
  message("Working directory: ${wk_dir}")
endif()
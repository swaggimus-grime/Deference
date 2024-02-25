# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.18)
set(CMAKE_CXX_STANDARD 23)

#set(CMAKE_BUILD_TYPE "Debug")
set(WIN32 true)

project ("Deference")
# Include sub-prs.
find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if(NOT GIT_SUBMODULE)
        message(FATAL_ERROR "Failed to turn on GIT_SUBMODULE")
    endif()
endif()

function(set_git_submodule dir branch release)
    if("${branch}" STREQUAL "")
        message("${dir} on default branch")
    else()
        execute_process(COMMAND ${GIT_EXECUTABLE} checkout ${branch}
                        WORKING_DIRECTORY ${dir})
    endif()

    if("${release}" STREQUAL "")
        message("${dir} on default release")
    else()
        execute_process(COMMAND ${GIT_EXECUTABLE} checkout ${release}
                        WORKING_DIRECTORY ${dir})
    endif()
               
endfunction(set_git_submodule)

execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive ${PROJECT_SOURCE_DIR}
                WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                RESULT_VARIABLE GIT_SUBMOD_RESULT)
if(NOT GIT_SUBMOD_RESULT EQUAL "0")
    message(FATAL_ERROR "git submodule update --init --recursive ${PROJECT_SOURCE_DIR} failed with ${GIT_SUBMOD_RESULT}")
endif()

set(DXTEX_DIR ${PROJECT_SOURCE_DIR}/tools/DirectXTex)
set(DXTK12_DIR ${PROJECT_SOURCE_DIR}/tools/DirectXTK12)
set(TINYGLTF_DIR ${PROJECT_SOURCE_DIR}/tools/tinygltf)
set(IMGUI_DIR ${PROJECT_SOURCE_DIR}/tools/imgui)

set_git_submodule(${DXTEX_DIR} "" "")
set_git_submodule(${DXTK12_DIR} "" "7d7e5b7")
set_git_submodule(${TINYGLTF_DIR} "" "")
set_git_submodule(${IMGUI_DIR} "docking" "")

add_subdirectory(${DXTEX_DIR})
add_subdirectory(${DXTK12_DIR})
add_subdirectory(${TINYGLTF_DIR})
add_subdirectory("Deference")
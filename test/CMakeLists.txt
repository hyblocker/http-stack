cmake_minimum_required (VERSION 3.8)

project(HttpStackTests)
message("HttpStack - Tests")

function(GroupSourcesByFolder target)
    set(SOURCE_GROUP_DELIMITER "/")
    set(last_dir "")
    set(files "")

    get_target_property(sources ${target} SOURCES)

    foreach(file ${sources})
        file(RELATIVE_PATH relative_file "${PROJECT_SOURCE_DIR}" ${file})
        get_filename_component(dir "${relative_file}" PATH)
        if(NOT "${dir}" STREQUAL "${last_dir}")
            if(files)
                source_group("${last_dir}" FILES ${files})
            endif()
            set(files "")
        endif()
        set(files ${files} ${file})
        set(last_dir "${dir}")
    endforeach()

    if(files)
        source_group("${last_dir}" FILES ${files})
    endif()
endfunction()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add source files
file(GLOB_RECURSE SOURCES_API ${CMAKE_SOURCE_DIR}/src "*.c" "*.h" "*.hpp" "*.cpp")
file(GLOB_RECURSE SOURCES_HEADERS ${CMAKE_SOURCE_DIR}/include "*.h" "*.hpp")

foreach(SOURCE IN ITEMS ${SOURCES_API})
    get_filename_component(SOURCE_PATH "${SOURCE}" PATH)
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" GROUP_PATH "${SOURCE_PATH}")
    string(REPLACE "/" "\\" GROUP_PATH "${GROUP_PATH}")
    source_group("${GROUP_PATH}" FILES "${SOURCE}")
endforeach()

add_executable(HttpStackTests ${SOURCES_API} ${SOURCES_HEADERS})
GroupSourcesByFolder(HttpStackTests)

target_include_directories(HttpStackTests
    PUBLIC ${CMAKE_SOURCE_DIR}
    PUBLIC ${CMAKE_SOURCE_DIR}/test
    PUBLIC ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(HttpStackTests
    PRIVATE Ws2_32
    PRIVATE HttpStack)

target_compile_definitions(HttpStackTests
    PRIVATE NOMINMAX
    PRIVATE UNICODE
)

# Object files are too big with spacecal sooooo
if (MSVC)
    target_compile_options(HttpStackTests PRIVATE /bigobj)
else ()
    target_compile_options(HttpStackTests PRIVATE -Wa,-mbig-obj)
endif ()

# Set as startup project
set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT HttpStackTests)
set_property(TARGET HttpStackTests PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/artifacts/$<CONFIG>")
cmake_minimum_required(VERSION 3.25)
project(raygui_forms C)

set(CMAKE_C_STANDARD 11)

# Dependencies
include(FetchContent)

set(RAYLIB_VERSION 4.5.0)
find_package(raylib ${RAYLIB_VERSION} QUIET) # QUIET or REQUIRED
if (NOT raylib_FOUND) # If there's none, fetch and build raylib
    FetchContent_Declare(
            raylib
            DOWNLOAD_EXTRACT_TIMESTAMP OFF
            URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
    )
    FetchContent_GetProperties(raylib)
    if (NOT raylib_POPULATED) # Have we downloaded raylib yet?
        set(FETCHCONTENT_QUIET NO)
        FetchContent_Populate(raylib)
        set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
        add_subdirectory(${raylib_SOURCE_DIR} ${raylib_BINARY_DIR})
    endif()
endif()

FetchContent_Declare(
        raygui
        GIT_REPOSITORY    https://github.com/raysan5/raygui.git
        GIT_TAG           3.6
        CONFIGURE_COMMAND ""
        BUILD_COMMAND     ""
)
FetchContent_GetProperties(raygui)
if(NOT raygui_POPULATED)
    FetchContent_Populate(raygui)
endif()
add_library(raygui INTERFACE)
target_include_directories(raygui INTERFACE ${raygui_SOURCE_DIR}/src)

FetchContent_Declare(
        tiny-json
        GIT_REPOSITORY    https://github.com/rafagafe/tiny-json.git
        GIT_TAG           025cdde5f2574290d8b2a8524bc257db2829bb7c
        CONFIGURE_COMMAND ""
        BUILD_COMMAND     ""
)
FetchContent_GetProperties(tiny-json)
if(NOT tiny-json_POPULATED)
    FetchContent_Populate(tiny-json)
endif()
add_library(tiny-json INTERFACE)
target_include_directories(tiny-json INTERFACE ${tiny-json_SOURCE_DIR})

add_executable(${PROJECT_NAME}
        ${tiny-json_SOURCE_DIR}/tiny-json.c
        source/main.c
        )

target_include_directories(${PROJECT_NAME} PRIVATE include)

target_link_libraries(${PROJECT_NAME} raylib raygui tiny-json)

# Web Configurations
if (${PLATFORM} STREQUAL "Web")
    # Tell Emscripten to build an example.html file.
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html")
endif()

# Checks if OSX and links appropriate frameworks (Only required on MacOS)
if (APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework IOKit")
    target_link_libraries(${PROJECT_NAME} "-framework Cocoa")
    target_link_libraries(${PROJECT_NAME} "-framework OpenGL")
endif()
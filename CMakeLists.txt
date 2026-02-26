cmake_minimum_required(VERSION 3.25)

project(LunateEpsilon
    VERSION 2.0.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Qt6 — system-installed, no vcpkg, no FetchContent
find_package(Qt6 REQUIRED COMPONENTS Widgets Concurrent)

set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/Converter.cpp
    src/ThemeManager.cpp
)

set(HEADERS
    src/MainWindow.h
    src/Converter.h
    src/ThemeManager.h
    src/Logger.h
)

set(RESOURCES
    resources/resources.qrc
)

add_executable(LunateEpsilon WIN32
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
)

target_include_directories(LunateEpsilon PRIVATE src)

target_link_libraries(LunateEpsilon PRIVATE
    Qt6::Widgets
    Qt6::Concurrent
    dwmapi          # DWM shadow preservation
)

# MSVC-specific flags
if(MSVC)
    target_compile_options(LunateEpsilon PRIVATE
        /W4
        /WX-            # Warnings not treated as errors during development
        /permissive-    # Strict conformance
        /Zc:__cplusplus # Correct __cplusplus macro value
        /utf-8          # UTF-8 source and execution charset
    )
    # Suppress MSVC warnings about Qt internals
    target_compile_definitions(LunateEpsilon PRIVATE
        _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
        NOMINMAX
        WIN32_LEAN_AND_MEAN
    )
endif()

# Debug vs Release
target_compile_definitions(LunateEpsilon PRIVATE
    $<$<CONFIG:Debug>:LE_DEBUG>
    $<$<CONFIG:Release>:QT_NO_DEBUG_OUTPUT>
)
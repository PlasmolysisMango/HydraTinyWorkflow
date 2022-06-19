cmake_minimum_required(VERSION 3.5)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_MODULE_PATH "E:/github/USD_Debug")

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(tiny)

# QT

find_package(Qt5 COMPONENTS Widgets REQUIRED)

# usd

add_definitions(-DNOMINMAX)

find_package(pxr)

add_executable(tiny
    main.cpp
    widget.cpp
    widget.h
)

# link
target_link_libraries(tiny Qt5::Widgets)

target_link_libraries(tiny usd usdImagingGL)

include_directories("C:/Users/Jack/AppData/Local/Programs/Python/Python39/include")
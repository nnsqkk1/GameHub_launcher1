cmake_minimum_required(VERSION 3.20)
project(GameHubLauncher VERSION 0.7 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(UNICODE _UNICODE)

if(MSVC)
    add_compile_options(/utf-8)
endif()

add_executable(GameHubLauncher WIN32 main.cpp)

target_link_libraries(GameHubLauncher PRIVATE
    comctl32 user32 gdi32 shell32 ole32 oleaut32 uxtheme urlmon comdlg32)
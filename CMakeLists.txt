cmake_minimum_required(VERSION 3.12)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(rt VERSION 1.0)

# find source files:
file(GLOB_RECURSE rt_src CONFIGURE_DEPENDS "src/*.cpp")
add_executable(${PROJECT_NAME} ${rt_src})

# find include diretories and libraries
find_package(SDL2 REQUIRED)
find_library(LIBSDL2     SDL2)
find_library(LIBSDL2MAIN SDL2main)
target_link_libraries(${PROJECT_NAME} ${LIBSDL2} ${LIBSDL2MAIN})
include_directories("include/" ${SDL2_INCLUDE_DIRS})

# set working directory:
if(WIN32)
    set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DPI_AWARE "PerMonitor") # set dpi awareness (only works on windows)
endif()
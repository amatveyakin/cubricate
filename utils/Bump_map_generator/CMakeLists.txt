project(Bump_map_generator)
cmake_minimum_required(VERSION 2.6)
find_package(Qt4 REQUIRED)

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})

set(Bump_map_generator_SRCS main.cpp)
qt4_automoc(${Bump_map_generator_SRCS})
add_executable(Bump_map_generator ${Bump_map_generator_SRCS})
target_link_libraries(Bump_map_generator ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})

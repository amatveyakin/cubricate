project (Cubricate_server)
cmake_minimum_required (VERSION 2.6)
find_package (Qt4 REQUIRED)

include_directories (${QT_INCLUDES})

set (Cubricate_server_SRCS ../src/server/main.cpp)
add_executable (cubricate_server ${Cubricate_server_SRCS})
target_link_libraries (cubricate_server ${QT_QTCORE_LIBRARY})

set(CMAKE_CXX_FLAGS  "-std=gnu++0x -Wall -Wextra")

project (Cubricate_server)
cmake_minimum_required (VERSION 2.6)
find_package (Qt4 REQUIRED)

set (src_dir ../src)

include_directories (${QT_INCLUDES} ${src_dir})
add_definitions (-DSERVER_APP)

set (Cubricate_server_SRCS ${src_dir}/server/main.cpp ${src_dir}/server/server_world.cpp
                           ${src_dir}/common/world_chunk.cpp ${src_dir}/common/world_map.cpp ${src_dir}/common/player.cpp)
add_executable (cubricate_server ${Cubricate_server_SRCS})
target_link_libraries (cubricate_server ${QT_QTTEST_LIBRARY} ${QT_QTNETWORK_LIBRARY})

set (CMAKE_CXX_FLAGS  "-std=gnu++0x -Wall -Wextra")

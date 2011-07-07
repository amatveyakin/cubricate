project (Cubricate_client)
cmake_minimum_required (VERSION 2.6)
find_package (Qt4 REQUIRED)

set (src_dir ../src)
set (resource_dir ../resources)

include_directories (${QT_INCLUDES} ${src_dir})
add_definitions (-DCLIENT_APP)


set (Cubricate_client_SRCS ${src_dir}/client/main.cpp ${src_dir}/client/client_world.cpp
     ${src_dir}/client/visible_cube_set.cpp ${src_dir}/client/glwidget.cpp
     ${src_dir}/common/world_chunk.cpp ${src_dir}/common/world_map.cpp ${src_dir}/common/player.cpp
     ${src_dir}/common/physics/water_engine.cpp
     ${src_dir}/client/octree.cpp ${src_dir}/client/simple_world_map.cpp ${src_dir}/client/GLTools/math3d.cpp)

qt4_automoc (${Cubricate_client_SRCS})

add_custom_target (shaders)
add_executable (cubricate_client ${Cubricate_client_SRCS})
add_dependencies (cubricate_client shaders)
target_link_libraries (cubricate_client ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTOPENGL_LIBRARY})

add_custom_command (TARGET shaders PRE_BUILD
                    COMMAND ../build_shaders.sh)

set (CMAKE_CXX_FLAGS  "-std=gnu++0x -Wall -Wextra")

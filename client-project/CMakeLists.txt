project (Cubricate_client)
cmake_minimum_required (VERSION 2.6)
find_package (Qt4 REQUIRED)

set (project_root ${CMAKE_SOURCE_DIR}/..)
set (src_dir ${project_root}/src)
set (resource_dir ${project_root}/resources)

add_subdirectory (${project_root}/utils/Macro_processor ${project_root}/utils/Macro_processor/build)

include_directories (${QT_INCLUDES} ${src_dir} ${src_dir}/client/GLTools/include)
add_definitions (-DCLIENT_APP)


set (Cubricate_client_SRCS ${src_dir}/client/main.cpp ${src_dir}/client/client_world.cpp
     ${src_dir}/client/visible_cube_set.cpp ${src_dir}/client/glwidget.cpp
     ${src_dir}/common/world_chunk.cpp ${src_dir}/common/world_map.cpp ${src_dir}/common/simple_world_map.cpp
     ${src_dir}/common/simple_light_map.cpp ${src_dir}/common/player.cpp ${src_dir}/common/physics/water_engine.cpp
     ${src_dir}/client/octree.cpp ${src_dir}/client/GLTools/math3d.cpp ${src_dir}/client/GLTools/GLShaderLoader.cpp)

qt4_automoc (${Cubricate_client_SRCS})

add_executable (cubricate_client ${Cubricate_client_SRCS})
target_link_libraries (cubricate_client sfml-window ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTOPENGL_LIBRARY})

add_custom_target (shaders ALL ../build_shaders.sh)
add_dependencies (shaders macro_processor)

set (CMAKE_CXX_FLAGS  "-std=gnu++0x -Wall -Wextra")
set (CMAKE_CXX_FLAGS_DEBUG "-g -DENABLE_DEBUG_PRINTING")
set (CMAKE_CXX_FLAGS_RELEASE "-O3 -funroll-loops -DNDEBUG")
set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELEASE} ${CMAKE_CXX_FLAGS_DEBUG}")

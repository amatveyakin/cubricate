project (Cubricate_client)
cmake_minimum_required (VERSION 2.6)
find_package (Qt4 REQUIRED)

set (src_dir ../src)
set (gltools_src_dir ${src_dir}/client/GLTools/src)

include_directories (${QT_INCLUDES} ${src_dir} ${src_dir}/client/GLTools/include/)
add_definitions (-DCLIENT_APP)

set (Cubricate_client_SRCS ${src_dir}/client/main.cpp ${src_dir}/client/glwidget.cpp
     ${gltools_src_dir}/GLBatch.cpp  ${gltools_src_dir}/glew.cpp  ${gltools_src_dir}/GLShaderManager.cpp
     ${gltools_src_dir}/GLTools.cpp  ${gltools_src_dir}/GLTriangleBatch.cpp  ${gltools_src_dir}/math3d.cpp)
qt4_automoc (${Cubricate_client_SRCS})
add_executable (cubricate_client ${Cubricate_client_SRCS})
target_link_libraries (cubricate_client ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} QtOpenGL)

set (CMAKE_CXX_FLAGS  "-std=gnu++0x -Wall -Wextra")

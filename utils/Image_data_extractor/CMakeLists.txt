project( image_data_extractor )
cmake_minimum_required( VERSION 2.6 )
find_package( Qt4 REQUIRED )

include_directories( ${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} )

add_executable( image_data_extractor main.cpp )
target_link_libraries( image_data_extractor ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} )

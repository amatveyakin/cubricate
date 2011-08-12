project (Macro_processor)
cmake_minimum_required (VERSION 2.6)
find_package (Qt4 REQUIRED)

include_directories (${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})

set (macro_processor_SRCS main.cpp)

qt4_automoc (${macro_processor_SRCS})

add_executable (macro_processor ${macro_processor_SRCS})
target_link_libraries (macro_processor ${QT_QTCORE_LIBRARY})

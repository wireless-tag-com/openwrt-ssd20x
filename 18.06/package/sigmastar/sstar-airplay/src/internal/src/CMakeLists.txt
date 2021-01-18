cmake_minimum_required(VERSION 2.8.0)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../ffmpeg/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../sstar/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../internal/include)

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../ffmpeg/lib)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../sstar/lib)

set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../lib)

aux_source_directory(./ LIB_SRCS)

string(REGEX REPLACE ".*/(.*)" "\\1" CURRENT_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}) 
add_library(internal SHARED ${LIB_SRCS})

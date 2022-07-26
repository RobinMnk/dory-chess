cmake_minimum_required(VERSION 3.21)
project(ChessEngine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fomit-frame-pointer -foptimize-sibling-calls")
#
#if(NOT CMAKE_BUILD_TYPE)
#    set(CMAKE_BUILD_TYPE Release)
#endif()
#
#set(CMAKE_CXX_FLAGS "-Wall -Wextra")
#set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS "-O3")

add_executable(ChessEngine main.cpp board.h chess.h utils.cpp utils.h checklogichandler.h piecesteps.h movegen.h movecollectors.h fenreader.h)
target_compile_options(ChessEngine PUBLIC -Wa,-mbig-obj)


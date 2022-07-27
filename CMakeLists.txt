cmake_minimum_required(VERSION 3.21)
project(ChessEngine)

set(CMAKE_CXX_STANDARD 20)

#set(CMAKE_CXX_FLAGS "-tune=native")
#set(CMAKE_CXX_FLAGS "-mbmi2")
#set(CMAKE_CXX_FLAGS "-fomit-frame-pointer -foptimize-sibling-calls")
#set(CMAKE_CXX_FLAGS "-O3")

add_executable(ChessEngine main.cpp board.h chess.h utils.cpp utils.h checklogichandler.h piecesteps.h movegen.h movecollectors.h fenreader.h)
target_compile_options(ChessEngine PUBLIC -march=native)
target_compile_options(ChessEngine PUBLIC -fomit-frame-pointer -foptimize-sibling-calls)
target_compile_options(ChessEngine PUBLIC -Wa,-mbig-obj)
target_compile_options(ChessEngine PUBLIC -O3)


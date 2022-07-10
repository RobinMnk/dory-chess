cmake_minimum_required(VERSION 3.21)
project(ChessEngine)

set(CMAKE_CXX_STANDARD 20)

add_executable(ChessEngine main.cpp board.h chess.h utils.cpp utils.h checklogichandler.h piecesteps.h movegen.h)
target_compile_options(ChessEngine PUBLIC -Wa,-mbig-obj)


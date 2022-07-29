cmake_minimum_required(VERSION 3.21)
project(ChessEngine)

set(CMAKE_CXX_STANDARD 20)

add_executable(ChessEngine src/main.cpp src/board.h src/chess.h src/utils.cpp src/utils.h src/checklogichandler.h src/piecesteps.h src/movegen.h src/movecollectors.h src/fenreader.h)
target_compile_options(ChessEngine PUBLIC -Wall -Wextra)
target_compile_options(ChessEngine PUBLIC -march=native)
target_compile_options(ChessEngine PUBLIC -fomit-frame-pointer -foptimize-sibling-calls)
target_compile_options(ChessEngine PUBLIC -O3)


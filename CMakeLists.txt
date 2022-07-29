cmake_minimum_required(VERSION 3.21)
project(ChessEngine)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(ChessEngine src/main.cpp src/board.h src/chess.h src/utils.cpp src/utils.h src/checklogichandler.h src/piecesteps.h src/movegen.h src/movecollectors.h src/fenreader.h)
target_compile_options(ChessEngine PUBLIC -Wall -Wextra)
target_compile_options(ChessEngine PUBLIC -march=native)
target_compile_options(ChessEngine PUBLIC -fomit-frame-pointer -foptimize-sibling-calls)
target_compile_options(ChessEngine PUBLIC -O3)

enable_testing()

add_executable(tester testing/test.cpp)
target_compile_options(tester PUBLIC -Wall -Wextra)
target_compile_options(tester PUBLIC -march=native)
target_compile_options(tester PUBLIC -fomit-frame-pointer -foptimize-sibling-calls)
target_compile_options(tester PUBLIC -O3)
target_link_libraries(tester GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(tester)
cmake_minimum_required(VERSION 3.21)
project(Dory)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(Dory src/main.cpp)
target_compile_options(Dory PUBLIC -Wall -Wextra)
target_compile_options(Dory PUBLIC -march=native)
target_compile_options(Dory PUBLIC -fomit-frame-pointer -foptimize-sibling-calls)
target_compile_options(Dory PUBLIC -O3)


add_executable(UCI src/uci.cpp)
target_compile_options(UCI PUBLIC -Wall -Wextra)
target_compile_options(UCI PUBLIC -march=native)
target_compile_options(UCI PUBLIC -fomit-frame-pointer -foptimize-sibling-calls)
target_compile_options(UCI PUBLIC -O3)

enable_testing()

add_executable(perft testing/moveGenerationTest.cpp)
target_compile_options(perft PUBLIC -Wall -Wextra)
target_compile_options(perft PUBLIC -march=native)
target_compile_options(perft PUBLIC -fomit-frame-pointer -foptimize-sibling-calls)
target_compile_options(perft PUBLIC -O3)
target_link_libraries(perft GTest::gtest_main)


add_executable(engineTest testing/engineTest.cpp)
target_compile_options(engineTest PUBLIC -Wall -Wextra)
target_compile_options(engineTest PUBLIC -march=native)
target_compile_options(engineTest PUBLIC -fomit-frame-pointer -foptimize-sibling-calls)
target_compile_options(engineTest PUBLIC -O3)
target_link_libraries(engineTest GTest::gtest_main)

include(GoogleTest)
if(TEST_SUITE STREQUAL "perft" OR TEST_SUITE STREQUAL "all")
    gtest_discover_tests(perft)
endif()
if(TEST_SUITE STREQUAL "engineTest" OR TEST_SUITE STREQUAL "all")
    gtest_discover_tests(engineTest)
endif()
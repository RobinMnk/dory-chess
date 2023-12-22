//
// Created by robin on 22.12.2023.
//

#include <gtest/gtest.h>
#include <fstream>
#include "../src/fenreader.h"
#include "../src/engine/engine.h"

using uLong = unsigned long long;

const std::array<std::string, 2> puzzleFiles{
    "../resources/puzzles2000.txt",
    "../resources/puzzlesSacrifices.txt"
};

std::vector<std::pair<std::string, std::string>> loadTestCases(int index) {
    /// Setup Engine
    PieceSteps::load();
    Zobrist::init();

    std::vector<std::pair<std::string, std::string>> testCases;
    std::string fen, solution;
    std::ifstream file (puzzleFiles.at(index));
    if (file.is_open()) {
        while(std::getline(file, fen)) {
            std::getline(file, solution);
            testCases.emplace_back(fen, solution);
        }
        file.close();
    }
    return testCases;
}

typedef std::pair<std::string, std::string> TestParam;

class EngineTest : public testing::TestWithParam<TestParam> {};

TEST_P(EngineTest, Check) {
    auto [fen, solution] = GetParam();
    auto [board, state] = Utils::loadFEN(fen);

    auto [_, line] = EngineMC::iterativeDeepening(board, state, 6);
    std::string output = Utils::moveNameShortNotation(line.back());

    ASSERT_EQ(output, solution);
}

INSTANTIATE_TEST_SUITE_P(
        Puzzles2000,
        EngineTest,
        testing::ValuesIn(loadTestCases(0))
);

INSTANTIATE_TEST_SUITE_P(
        PuzzlesSacrifices,
        EngineTest,
        testing::ValuesIn(loadTestCases(1))
);